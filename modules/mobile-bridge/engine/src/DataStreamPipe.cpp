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

#include "AACE/Engine/MobileBridge/DataStreamPipe.h"

#include <condition_variable>
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace mobileBridge {

template <typename T>
class CircularQueue {
    std::deque<T> content;
    size_t capacity;
    bool nonblocking;

    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

    CircularQueue(const CircularQueue&) = delete;
    CircularQueue(CircularQueue&&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;
    CircularQueue& operator=(CircularQueue&&) = delete;

public:
    CircularQueue(size_t capacity) : capacity(capacity), nonblocking(false) {
    }

    void push(T&& item) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            not_full.wait(lock, [this]() { return (content.size() < capacity) || nonblocking; });
            if (content.size() < capacity) {
                content.push_back(std::move(item));
            } else if (nonblocking) {
                throw std::runtime_error("Would wait forever");
            } else {
                throw std::runtime_error("Not reached");
            }
        }
        not_empty.notify_one();
    }

    void push(const T* items, size_t len) {
        if (len == 0) {
            return;
        }
        size_t pushed = 0;
        for (; pushed < len;) {
            {
                std::unique_lock<std::mutex> lock(mutex);
                auto batch = std::max(std::min(len - pushed, capacity - content.size()), (size_t)1);
                not_full.wait(lock, [this, batch]() { return (content.size() + batch <= capacity) || nonblocking; });
                if (content.size() + batch <= capacity) {
                    auto* batch_it = items + pushed;
                    content.insert(content.end(), batch_it, batch_it + batch);
                    pushed += batch;
                } else if (nonblocking) {
                    throw std::runtime_error("Would wait forever");
                } else {
                    throw std::runtime_error("Not reached");
                }
            }
            not_empty.notify_one();
        }
    }

    void pop(T& item) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            not_empty.wait(lock, [this]() { return !content.empty() || nonblocking; });

            if (!content.empty()) {
                item = std::move(content.front());
                content.pop_front();
            } else if (nonblocking) {
                throw std::runtime_error("Would wait forever");
            } else {
                throw std::runtime_error("Not reached");
            }
        }
        not_full.notify_one();
    }

    void pop(T* items, size_t len) {
        if (len == 0) {
            return;
        }
        size_t popped = 0;
        for (; popped < len;) {
            {
                std::unique_lock<std::mutex> lock(mutex);
                auto batch = std::max(std::min(len - popped, content.size()), (size_t)1);
                not_empty.wait(lock, [this, batch]() { return (content.size() >= batch) || nonblocking; });
                if (content.size() >= batch) {
                    auto* batch_it = items + popped;
                    for (size_t i = 0; i < batch; ++i) {
                        *batch_it++ = std::move(content.front());
                        content.pop_front();
                    }
                    popped += batch;
                } else if (nonblocking) {
                    throw std::runtime_error("Would wait forever");
                } else {
                    throw std::runtime_error("Not reached");
                }
            }
            not_full.notify_one();
        }
    }

    size_t waitForAvailableBytes(size_t minAvailable) {
        std::unique_lock<std::mutex> lock(mutex);
        not_empty.wait(lock, [this, minAvailable]() { return (content.size() >= minAvailable) || nonblocking; });
        if (content.size() >= minAvailable) {
            return content.size();
        }
        if (nonblocking) {
            throw std::runtime_error("Would wait forever");
        }
        throw std::runtime_error("Not reached");
    }

    size_t size() {
        std::unique_lock<std::mutex> lock(mutex);
        return content.size();
    }

    void setNonblcking() {
        std::unique_lock<std::mutex> lock(mutex);
        nonblocking = true;
        not_full.notify_all();
        not_empty.notify_all();
    }
};

struct DataStreamPipe::Impl {
    using CircularBuffer = CircularQueue<uint8_t>;

    struct BlockingInputStream : public DataInputStream {
        std::shared_ptr<CircularBuffer> m_buf;

        BlockingInputStream(std::shared_ptr<CircularBuffer> buf) : m_buf(std::move(buf)) {
        }

        size_t read(uint8_t* buf, size_t len) override {
            size_t minAvailable = std::min((size_t)1, len);
            auto available = m_buf->waitForAvailableBytes(minAvailable);
            size_t toRead = std::min(available, len);
            readFully(buf, toRead);
            return toRead;
        }

        void readFully(uint8_t* buf, size_t len) override {
            m_buf->pop(buf, len);
        }

        uint32_t readByte() override {
            uint8_t b;
            m_buf->pop(b);
            return b;
        }

        void close() override {
            m_buf->setNonblcking();
        }
    };

    struct BlockingOutputStream : public DataOutputStream {
        std::shared_ptr<CircularBuffer> m_buf;

        BlockingOutputStream(std::shared_ptr<CircularBuffer> buf) : m_buf(std::move(buf)) {
        }

        void writeBytes(const uint8_t* buf, size_t len) override {
            m_buf->push(buf, len);
        }

        void writeByte(uint32_t b) override {
            m_buf->push(b);
        }

        void close() override {
            m_buf->setNonblcking();
        }
    };

    std::shared_ptr<CircularBuffer> m_buf;
    std::shared_ptr<BlockingInputStream> m_input;
    std::shared_ptr<BlockingOutputStream> m_output;

    Impl(size_t bufferSize) :
            m_buf{std::make_shared<CircularBuffer>(bufferSize)},
            m_input{std::make_shared<BlockingInputStream>(m_buf)},
            m_output{std::make_shared<BlockingOutputStream>(m_buf)} {
    }

    /**
     * There is an advanced use case: you can keep reading and writing to a closed pipe
     * as long as you still keep shared reference to the input or output stream until the pipe
     * is full or empty. In other words, non-blocking operations are allowed no matter the pipe
     * is closed or not. Blocking operations on a closed pipe will throw exception.
     */
    void close() {
        if (m_buf) {
            m_buf->setNonblcking();
            m_buf.reset();
        }
        m_input.reset();
        m_output.reset();
    }

    size_t size() const {
        return m_buf ? m_buf->size() : 0;
    }

    size_t waitForAvailableBytes(size_t minAvailable) const {
        if (!m_buf) {
            throw std::runtime_error("Pipe was closed");
        }
        return m_buf->waitForAvailableBytes(minAvailable);
    }
};

DataStreamPipe::DataStreamPipe(size_t bufferSize) {
    m_impl = std::make_unique<Impl>(bufferSize);
}

DataStreamPipe::~DataStreamPipe() {
}

void DataStreamPipe::close() {
    m_impl->close();
}

std::shared_ptr<DataInputStream> DataStreamPipe::getInput() {
    auto input = m_impl->m_input;
    if (!input) {
        throw std::runtime_error("Input was closed");
    }
    return input;
}

std::shared_ptr<DataOutputStream> DataStreamPipe::getOutput() {
    auto output = m_impl->m_output;
    if (!output) {
        throw std::runtime_error("Output was closed");
    }
    return output;
}

size_t DataStreamPipe::size() {
    return m_impl->size();
}

size_t DataStreamPipe::waitForAvailableBytes(size_t minAvailable) {
    return m_impl->waitForAvailableBytes(minAvailable);
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
