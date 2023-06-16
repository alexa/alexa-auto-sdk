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

#include "AACE/Engine/MobileBridge/MobileBridgeEngineImpl.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/SessionManager.h"
#include "AACE/Engine/MobileBridge/TcpProxy.h"
#include "AACE/Engine/MobileBridge/TransportLoop.h"
#include "AACE/Engine/MobileBridge/TransportManager.h"
#include "AACE/Engine/MobileBridge/UdpProxy.h"

namespace aace {
namespace engine {
namespace mobileBridge {

struct MobileBridgeEngineImpl::Impl {
    static constexpr const char* TAG = "MobileBridgeEngineImpl::Impl";

    std::shared_ptr<Config> m_config;

    std::vector<int> m_retryTable = {
        10 * 1000,
        20 * 1000,
        30 * 1000,
    };

    using ConnectionId = uint32_t;

    std::shared_ptr<aace::mobileBridge::MobileBridge> m_mobileBridge;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;

    std::unordered_map<std::string, std::shared_ptr<TransportLoop>> m_transportLoops;
    std::shared_ptr<TransportManager> m_transportManager;
    std::shared_ptr<SessionManager> m_sessionManager;

    std::shared_ptr<TcpProxy> m_tcpProxy;
    std::shared_ptr<UdpProxy> m_udpProxy;

    Impl(
        std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::shared_ptr<Config> config) :
            m_mobileBridge(std::move(mobileBridge)), m_deviceInfo(std::move(deviceInfo)) {
        AACE_INFO(LX(TAG));
        m_config = config ? config : std::make_shared<Config>(Config::getDefault());
    }

    aace::engine::utils::threading::Executor m_executor;

    bool start(int tunFd) {
        return m_executor.submit([this, tunFd] { return execStart(tunFd); }).get();
    }

    bool execStart(int tunFd) {
        AACE_INFO(LX(TAG));

        if (m_transportManager) {
            AACE_WARN(LX(TAG).m("Already started"));
            return false;
        }

        // Initialize Session Manager here
        m_sessionManagerListener = std::make_shared<SessionManagerListener>(this);
        m_sessionManager =
            std::make_shared<SessionManager>(m_config->tcpProxyPort, m_config->udpProxyPort, m_sessionManagerListener);
        m_sessionManager->start(tunFd);

        // Start TCP / UDP proxy
        m_tcpProxy = std::make_shared<TcpProxy>(m_config->tcpProxyPort, [this](int connId, auto data) {
            if (m_transportManager) {
                m_transportManager->sendTcpData(connId, data);
            } else {
                AACE_ERROR(LX(TAG).m("TransportManager is not available"));
            }
        });
        m_udpProxy = std::make_shared<UdpProxy>(m_config->udpProxyPort, [this](int datagramId, auto datagram) {
            if (m_transportManager) {
                m_transportManager->sendUdpData(datagramId, datagram);
            } else {
                AACE_ERROR(LX(TAG).m("TransportManager is not available"));
            }
        });

        // Get device info
        auto deviceTypeId = m_deviceInfo ? m_deviceInfo->getDeviceType() : "";

        // Initialize transport manager
        m_transportManagerListener = std::make_shared<TransportManagerListener>(this);
        m_transportManager =
            std::make_shared<TransportManager>(m_tcpProxy, m_udpProxy, deviceTypeId, m_transportManagerListener);

        m_executor.submit([this] {
            // Retrieve all transports and start them
            auto transports = m_mobileBridge->getTransports();
            for (auto& transport : transports) {
                startTransportLoop(transport);
            }
        });

        return true;
    }

    void startTransportLoop(std::shared_ptr<aace::mobileBridge::Transport> transport) {
        AACE_INFO(LX(TAG).d("transport", transport->id));

        m_transportManager->regiserTransport(transport);

        auto loop = std::make_shared<TransportLoop>(m_mobileBridge, transport, m_retryTable, m_transportManager);
        m_transportLoops[transport->id] = loop;
    }

    bool stop() {
        return m_executor.submit([this] { return execStop(); }).get();
    }

    bool execStop() {
        AACE_INFO(LX(TAG));

        // Stop all transports
        for (auto& item : m_transportLoops) {
            item.second->quit();
        }
        m_transportLoops.clear();

        // Release transport manager
        m_transportManager.reset();

        // Stop TCP/UDP proxy
        m_udpProxy.reset();
        m_tcpProxy.reset();

        return true;
    }

    void authorizeDevice(const std::string& deviceToken, bool authorized) {
        m_executor.submit([this, deviceToken, authorized] { return execAuthorizeDevice(deviceToken, authorized); })
            .get();
    }

    void execAuthorizeDevice(const std::string& deviceToken, bool authorized) {
        AACE_INFO(LX(TAG));
        if (!m_transportManager) {
            AACE_WARN(LX(TAG).m("TransportManager is not ready yet"));
            return;
        }
        m_transportManager->authorizeDevice(deviceToken, authorized);
    }

    void sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
        m_executor.submit([this, deviceToken, infoId, info] { return execSendInfo(deviceToken, infoId, info); }).get();
    }

    void execSendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
        AACE_INFO(LX(TAG));
        if (!m_transportManager) {
            AACE_WARN(LX(TAG).m("TransportManager is not ready yet"));
            return;
        }
        m_transportManager->sendInfo(deviceToken, infoId, info);
    }

    bool shutdown() {
        AACE_INFO(LX(TAG));

        stop();
        m_executor.waitForSubmittedTasks();

        m_mobileBridge->setEngineInterface(nullptr);
        return true;
    }

    // TransportManager::Listener interface

    struct TransportManagerListener : TransportManager::Listener {
        MobileBridgeEngineImpl::Impl* self;

        TransportManagerListener(MobileBridgeEngineImpl::Impl* self) : self(self) {
        }

        void onActiveTransportChange(const std::string& transportId, const std::string& transportState) override {
            self->onActiveTransportChange(transportId, transportState);
        }

        void onDeviceHandshaked(
            const std::string& transportId,
            const std::string& deviceToken,
            const std::string& friendlyName) override {
            self->onDeviceHandshaked(transportId, deviceToken, friendlyName);
        }

        void onInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) override {
            self->onInfo(deviceToken, infoId, info);
        }
    };
    std::shared_ptr<TransportManagerListener> m_transportManagerListener;

    void onActiveTransportChange(const std::string& transportId, const std::string& transportState) {
        AACE_INFO(LX(TAG).d("transportId", transportId).d("transportState", transportState));
        m_executor.submit([this, transportId, transportState] {
            m_mobileBridge->onActiveTransportChange(transportId, transportState);
        });
    }

    void onDeviceHandshaked(
        const std::string& transportId,
        const std::string& deviceToken,
        const std::string& friendlyName) {
        AACE_DEBUG(LX(TAG).d("deviceToken", deviceToken).d("friendlyName", friendlyName));
        m_executor.submit([this, transportId, deviceToken, friendlyName] {
            m_mobileBridge->onDeviceHandshaked(transportId, deviceToken, friendlyName);
        });
    }

    void onInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
        AACE_DEBUG(LX(TAG).d("infoId", infoId).d("info", info));
        m_executor.submit([this, deviceToken, infoId, info] { m_mobileBridge->onInfo(deviceToken, infoId, info); });
    }

    struct SessionManagerListener : SessionManager::Listener {
        MobileBridgeEngineImpl::Impl* self;

        SessionManagerListener(MobileBridgeEngineImpl::Impl* self) : self(self) {
        }

        void onProtectSocket(int sock) override {
            self->onProtectSocket(sock);
        }
    };
    std::shared_ptr<SessionManagerListener> m_sessionManagerListener;

    void onProtectSocket(int sock) {
        m_executor.submit([this, sock] { m_mobileBridge->protectSocket(sock); }).get();
    }
};

// String to identify log entries originating from this file.
static const char* TAG = "MobileBridgeEngineImpl";

std::shared_ptr<MobileBridgeEngineImpl> MobileBridgeEngineImpl::create(
    std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridgePlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<Config> config) {
    try {
        ThrowIfNull(mobileBridgePlatformInterface, "nullMobileBridge");

        auto engineImpl = std::make_shared<MobileBridgeEngineImpl>(mobileBridgePlatformInterface, deviceInfo, config);
        mobileBridgePlatformInterface->setEngineInterface(engineImpl);
        return engineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

MobileBridgeEngineImpl::MobileBridgeEngineImpl(
    std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<Config> config) {
    m_impl = std::make_unique<Impl>(mobileBridge, deviceInfo, config);
}

MobileBridgeEngineImpl::~MobileBridgeEngineImpl() {
    shutdown();
}

void MobileBridgeEngineImpl::setRetryTable(const std::vector<int>& retryTable) {
    m_impl->m_retryTable = retryTable;
}

bool MobileBridgeEngineImpl::onStart(int tunFd) {
    return m_impl->start(tunFd);
}

bool MobileBridgeEngineImpl::onStop() {
    return m_impl->stop();
}

void MobileBridgeEngineImpl::onDeviceAuthorized(const std::string& deviceToken, bool authorized) {
    return m_impl->authorizeDevice(deviceToken, authorized);
}

void MobileBridgeEngineImpl::onInfoSent(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
    return m_impl->sendInfo(deviceToken, infoId, info);
}

bool MobileBridgeEngineImpl::shutdown() {
    return m_impl->shutdown();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
