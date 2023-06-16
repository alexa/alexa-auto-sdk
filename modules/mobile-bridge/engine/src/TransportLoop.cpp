/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "AACE/Engine/MobileBridge/TransportLoop.h"

#include <pthread.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <thread>
#include <vector>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/DataStream.h"
#include "AACE/Engine/MobileBridge/DataStreamPipe.h"
#include "AACE/Engine/MobileBridge/Util.h"
#include "AVSCommon/Utils/RetryTimer.h"

namespace aace {
namespace engine {
namespace mobileBridge {

#define LXT LX(TAG).d("transport", m_transport->id)

using Connection = aace::mobileBridge::MobileBridge::Connection;

struct TransportLoop::Impl {
    static constexpr const char* TAG = "TransportLoop::Impl";

    Impl(
        std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
        std::shared_ptr<aace::mobileBridge::Transport> transport,
        const std::vector<int>& retryTable,
        std::shared_ptr<Listener> listener) :
            m_state(State::INITIALIZED),
            m_mobileBridge(std::move(mobileBridge)),
            m_transport(std::move(transport)),
            m_listener(std::move(listener)),
            m_quit{false} {
        m_executor.submit([this, retryTable]() {
            setThreadName("TransportLoop");
            transportLoop(retryTable);
        });
    }

    State m_state;

    std::shared_ptr<aace::mobileBridge::MobileBridge> m_mobileBridge;
    std::shared_ptr<aace::mobileBridge::Transport> m_transport;
    std::weak_ptr<Listener> m_listener;

    aace::engine::utils::threading::Executor m_executor;
    std::function<void()> m_abortConnectionLoop;
    std::atomic<bool> m_quit;
    std::condition_variable m_cvQuit;
    std::mutex m_mutex;

    void transportLoop(const std::vector<int>& retryTable) {
        AACE_INFO(LXT);

        alexaClientSDK::avsCommon::utils::RetryTimer retryTimer(retryTable);

        auto transportId = m_transport->id;
        int retryCount = 0;
        do {
            AACE_INFO(LXT.m("Connecting " + transportId));
            setState(State::CONNECTING);
            auto connection = m_mobileBridge->connect(transportId);
            if (connection != nullptr) {
                setState(State::CONNECTED);
                connectionLoop(connection);
                m_mobileBridge->disconnect(transportId);

                retryCount = 0;
            } else {
                AACE_ERROR(LXT.m("Failed to connect"));
            }

            setState(State::DISCONNECTED);
            if (!m_quit) {
                auto timeToRetry = retryTimer.calculateTimeToRetry(retryCount++);
                AACE_DEBUG(LXT.m("Reconnect").d("timeToRetry", timeToRetry.count()));

                std::unique_lock<std::mutex> lk(m_mutex);
                m_cvQuit.wait_for(lk, std::chrono::milliseconds{timeToRetry}, [this]() { return m_quit.load(); });
            }
        } while (!m_quit);
    }

    class DataOutputStreamToTransport : public DataOutputStream {
    private:
        std::shared_ptr<Connection> m_conn;

    public:
        DataOutputStreamToTransport(std::shared_ptr<Connection> conn) : m_conn(std::move(conn)) {
        }

        void writeBytes(const uint8_t* buf, size_t len) override {
            m_conn->write(buf, 0, len);
        }

        void writeByte(uint32_t v) override {
            m_conn->write(reinterpret_cast<uint8_t*>(&v), 0, 1);
        }

        void close() override {
        }
    };

    void connectionLoop(std::shared_ptr<Connection> connection) {
        AACE_DEBUG(LX(TAG, "connectionLoop").m("Entering"));

        constexpr int PIPE_SIZE = 1024;

        // Prepare output stream to transport connection
        auto outputToTransport = std::make_shared<DataOutputStreamToTransport>(connection);
        auto incomingPipe = std::make_shared<DataStreamPipe>(PIPE_SIZE);

        // Setup a function to close incoming pipe and transport connection for
        // aborting incoming data puller and connection loop.
        setupAbortConnectionLoop([incomingPipe, connection] {
            incomingPipe->close();
            connection->close();
        });

        if (auto listener = m_listener.lock()) {
            listener->onOutputStreamReady(m_transport->id, outputToTransport);
        }

        auto incomingPipeOutput = incomingPipe->getOutput();
        auto incomingDataPuller = std::thread([this, connection, incomingPipeOutput] {
            setThreadName("DataPuller:In");
            uint8_t buf[PIPE_SIZE];
            while (!m_quit) {
                try {
                    auto len = connection->read(buf, 0, sizeof(buf));
                    if (len > 0) {
                        AACE_DEBUG(LX(TAG, "DataPuller:In").d("len", len));
                        incomingPipeOutput->writeBytes(buf, len);
                    } else {
                        AACE_DEBUG(LX(TAG, "DataPuller:In").m("EOS"));
                        break;
                    }
                } catch (std::exception& ex) {
                    AACE_ERROR(LXT.m("Stop pulling incoming data").m(ex.what()));
                    break;
                }
            }
            // Close the pipe output so that we can quit reading the pipe input as well.
            incomingPipeOutput->close();
        });

        while (!m_quit) {
            try {
                if (auto listener = m_listener.lock()) {
                    incomingPipe->waitForAvailableBytes(1);

                    auto ret = listener->onIncomingData(m_transport->id, incomingPipe->getInput());
                    if (ret == Handling::ABORT) {
                        AACE_INFO(LXT.m("Abort transport"));
                        break;
                    }
                    if (ret == Handling::HANDSHAKED) {
                        setState(State::HANDSHAKED);
                    }
                } else {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cvQuit.wait(lock, [this]() { return m_quit.load(); });
                }
            } catch (std::exception& ex) {
                AACE_ERROR(LXT.m("Abort transport").m(ex.what()));
                break;
            }
        }

        abortConnectionLoop();
        incomingDataPuller.join();
        AACE_DEBUG(LX(TAG, "connectionLoop").m("Exiting"));
    }

    void setupAbortConnectionLoop(std::function<void()> abortConnectionLoopFunc) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_abortConnectionLoop = abortConnectionLoopFunc;
    }

    void abortConnectionLoop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_abortConnectionLoop) {
            AACE_INFO(LXT);
            auto abortConnectionLoopFunc = m_abortConnectionLoop;
            m_abortConnectionLoop = nullptr;
            lock.unlock();

            abortConnectionLoopFunc();
        }
    }

    void setState(State state) {
        if (m_state == state) {
            return;
        }
        m_state = state;
        if (auto listener = m_listener.lock()) {
            listener->onTransportState(m_transport->id, state);
        }
    }

    void quit() {
        if (!m_quit) {
            AACE_INFO(LXT);
            abortConnectionLoop();
            m_quit = true;
            m_cvQuit.notify_all();
            m_executor.waitForSubmittedTasks();
        }
    }
};

// String to identify log entries originating from this file.
static const char* TAG = "TransportLoop";

TransportLoop::TransportLoop(
    std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
    std::shared_ptr<aace::mobileBridge::Transport> transport,
    const std::vector<int>& retryTable,
    std::shared_ptr<Listener> listener) {
    AACE_INFO(LX(TAG).d("transport", transport->id));
    m_impl = std::make_unique<Impl>(mobileBridge, transport, retryTable, listener);
}

TransportLoop::~TransportLoop() {
    quit();
}

void TransportLoop::quit() {
    m_impl->quit();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
