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

#include "AACE/Engine/MobileBridge/DataStream.h"

#include <cstddef>
#include <memory>
#include <stdexcept>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace mobileBridge {

DataStream::~DataStream() = default;

uint32_t DataInputStream::readInt() {
    uint8_t buf[4];
    readFully(buf, 4);
    return static_cast<uint32_t>(buf[0]) << 24 | static_cast<uint32_t>(buf[1]) << 16 |
           static_cast<uint32_t>(buf[2]) << 8 | static_cast<uint32_t>(buf[3]);
}

void DataInputStream::readFully(uint8_t* buf, size_t len) {
    if (len == 0) {
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        buf[i] = readByte();
    }
}

DataInputStreamUnique::DataInputStreamUnique(std::unique_ptr<uint8_t[]> buf, size_t len) :
        m_buf(std::move(buf)), m_len(len), m_off(0) {
}

void DataInputStreamUnique::close() {
    m_buf.reset();
    m_len = 0;
    m_off = 0;
}

size_t DataInputStreamUnique::read(uint8_t* buf, size_t len) {
    if (len == 0 || m_off >= m_len) {
        return 0;
    }
    auto bytes = std::min(m_len - m_off, len);
    readFully(buf, bytes);
    return bytes;
}

void DataInputStreamUnique::readFully(uint8_t* buf, size_t len) {
    if (len == 0) {
        return;
    }
    if (m_off + len > m_len) {
        throw std::runtime_error("out of bound");
    }
    if (buf == nullptr) {
        throw std::runtime_error("null data");
    }
    for (size_t i = 0; i < len; ++i, ++m_off) {
        buf[i] = m_buf[m_off];
    }
}

uint32_t DataInputStreamUnique::readByte() {
    if (m_off >= m_len) {
        throw std::runtime_error("out of bound");
    }
    return m_buf[m_off++];
}

// DataOutputStream

void DataOutputStream::writeBytes(const uint8_t* buf, size_t len) {
    if (len == 0) {
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        writeByte(buf[i]);
    }
}

void DataOutputStream::writeInt(uint32_t v) {
    writeByte((v >> 24) & 0xFF);
    writeByte((v >> 16) & 0xFF);
    writeByte((v >> 8) & 0xFF);
    writeByte((v >> 0) & 0xFF);
}

DataOutputStreamUnique::DataOutputStreamUnique(std::unique_ptr<uint8_t[]> buf, size_t len) :
        m_buf(std::move(buf)), m_len(len), m_off(0) {
}

DataOutputStreamUnique::DataOutputStreamUnique(size_t bytes) : m_buf(new uint8_t[bytes]), m_len(bytes), m_off(0) {
}

void DataOutputStreamUnique::close() {
    m_buf.reset();
    m_len = 0;
    m_off = 0;
}

void DataOutputStreamUnique::writeBytes(const uint8_t* buf, size_t len) {
    if (len == 0) {
        return;
    }
    if (m_off + len > m_len) {
        throw std::runtime_error("out of bound");
    }
    if (buf == nullptr) {
        throw std::runtime_error("null data");
    }
    for (size_t i = 0; i < len; ++i, ++m_off) {
        m_buf[m_off] = buf[i];
    }
}

void DataOutputStreamUnique::writeByte(uint32_t v) {
    if (m_off >= m_len) {
        throw std::runtime_error("out of bound");
    }
    m_buf[m_off++] = v;
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
