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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_DATA_STREAM_H
#define AACE_ENGINE_MOBILE_BRIDGE_DATA_STREAM_H

#include <cstddef>
#include <cstdint>
#include <memory>

namespace aace {
namespace engine {
namespace mobileBridge {

class DataStream {
public:
    virtual ~DataStream();
    virtual void close() = 0;
};

class DataInputStream : public DataStream {
public:
    /**
     * Read whatever available in the stream. It will block until there is available data
     * or the stream is closed.
     *
     * @return the number of bytes read into the buffer
     */
    virtual size_t read(uint8_t* buf, size_t len) = 0;
    /**
     * Read until the specified buffer is filled fully or the stream is closed.
     */
    virtual void readFully(uint8_t* buf, size_t len);
    virtual uint32_t readInt();
    virtual uint32_t readByte() = 0;
};

class DataInputStreamUnique : public DataInputStream {
public:
    DataInputStreamUnique(std::unique_ptr<uint8_t[]> buf, size_t len);

    std::unique_ptr<uint8_t[]> detach() {
        m_len = 0;
        m_off = 0;
        return std::move(m_buf);
    }

    void close() override;
    size_t read(uint8_t* buf, size_t len) override;
    void readFully(uint8_t* buf, size_t len) override;
    uint32_t readByte() override;

private:
    std::unique_ptr<uint8_t[]> m_buf;
    size_t m_len;
    size_t m_off;
};

class DataOutputStream : public DataStream {
public:
    virtual void writeBytes(const uint8_t* buf, size_t len);
    virtual void writeInt(uint32_t v);
    virtual void writeByte(uint32_t v) = 0;
};

class DataOutputStreamUnique : public DataOutputStream {
public:
    DataOutputStreamUnique(std::unique_ptr<uint8_t[]> buf, size_t len);
    DataOutputStreamUnique(size_t bytes);

    std::unique_ptr<uint8_t[]> detach() {
        m_len = 0;
        m_off = 0;
        return std::move(m_buf);
    }

    void close() override;
    void writeBytes(const uint8_t* buf, size_t len) override;
    void writeByte(uint32_t v) override;

private:
    std::unique_ptr<uint8_t[]> m_buf;
    size_t m_len;
    size_t m_off;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_MUXER_H
