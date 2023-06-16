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

#include "AACE/Engine/MobileBridge/Muxer.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

namespace aace {
namespace engine {
namespace mobileBridge {

// String to identify log entries originating from this file.
static const char* TAG = "Muxer";

Muxer::Muxer() {
}

static const uint8_t AAMB_MAGIC[] = {'A', 'M', 'B', '1'};

std::string Muxer::flagsToString(uint32_t flags) {
    std::vector<const char*> flagStrings;
    if ((flags & Flags::TCP) != 0) {
        flagStrings.emplace_back("TCP");
    }
    if ((flags & Flags::UDP) != 0) {
        flagStrings.emplace_back("UDP");
    }
    if ((flags & Flags::AUTH) != 0) {
        flagStrings.emplace_back("AUTH");
    }
    if ((flags & Flags::INFO) != 0) {
        flagStrings.emplace_back("INFO");
    }
    if ((flags & Flags::PING) != 0) {
        flagStrings.emplace_back("PING");
    }
    if ((flags & Flags::PONG) != 0) {
        flagStrings.emplace_back("PONG");
    }
    if ((flags & Flags::SYN) != 0) {
        flagStrings.emplace_back("SYN");
    }
    if ((flags & Flags::FIN) != 0) {
        flagStrings.emplace_back("FIN");
    }
    if ((flags & Flags::RST) != 0) {
        flagStrings.emplace_back("RST");
    }
    if ((flags & Flags::JSON) != 0) {
        flagStrings.emplace_back("JSON");
    }

    std::ostringstream oss;
    for (auto it = flagStrings.begin(); it != flagStrings.end(); ++it) {
        if (it != flagStrings.begin()) {
            oss << "|";
        }
        oss << *it;
    }
    return oss.str();
}

inline bool isAambMagic(const uint8_t magic[]) {
    return std::memcmp(magic, AAMB_MAGIC, sizeof(AAMB_MAGIC)) == 0;
}

Muxer::Frame Muxer::demux(std::shared_ptr<DataInputStream> stream) {
    if (!stream) {
        throw std::runtime_error("invalid stream");
    }

    try {
        uint8_t magic[4];
        stream->readFully(magic, sizeof(magic));

        if (!isAambMagic(magic)) {
            // Realign to frame boundary
            do {
                magic[0] = magic[1];
                magic[1] = magic[2];
                magic[2] = magic[3];
                magic[3] = stream->readByte();
            } while (!isAambMagic(magic));
        }

        Frame frame;
        frame.id = stream->readInt();
        frame.flags = stream->readInt();
        frame.len = stream->readInt();
        if (frame.len > 0) {
            auto buf = std::unique_ptr<uint8_t[]>(new uint8_t[frame.len]);
            stream->readFully(buf.get(), frame.len);
            frame.payload = std::move(buf);
        }
        return frame;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        throw;
    }
}

void Muxer::muxTo(
    std::shared_ptr<DataOutputStream> stream,
    uint32_t id,
    uint32_t flags,
    const uint8_t* payload,
    uint32_t off,
    uint32_t len) {
    if (!stream) {
        throw std::runtime_error("null stream");
    }
    if (len > 0 && payload == nullptr) {
        throw std::runtime_error("null payload");
    }
    stream->writeBytes(AAMB_MAGIC, sizeof(AAMB_MAGIC));
    stream->writeInt(id);
    stream->writeInt(flags);
    stream->writeInt(len);
    if (len > 0) {
        stream->writeBytes(payload + off, len);
    }
}

std::string& ltrim(std::string& str) {
    auto it =
        std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(str.begin(), it);
    return str;
}

std::string& rtrim(std::string& str) {
    auto it =
        std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(it.base(), str.end());
    return str;
}

Muxer::Headers Muxer::parsePayloadHeaders(const Frame& frame) {
    Muxer::Headers headers;
    std::regex keyValueRegex(R"(([^:]+)\s*:\s*(.+))");
    std::stringstream payload(std::string((char*)frame.payload.get(), frame.len));
    std::string line;

    while (std::getline(payload, line)) {
        std::smatch keyValueMatch;
        if (std::regex_search(line, keyValueMatch, keyValueRegex)) {
            auto key = keyValueMatch[1].str();
            auto value = keyValueMatch[2].str();
            headers[utils::string::toLower(key)] = ltrim(rtrim(value));
        } else {
            AACE_WARN(LX(TAG).m("Failed to parse").d("line", line));
        }
    }

    return headers;
}

std::string Muxer::createHeaders(std::initializer_list<std::pair<std::string, std::string>> headers) {
    std::ostringstream oss;
    for (auto& pair : headers) {
        oss << pair.first << ": " << pair.second << "\r\n";
    }
    return oss.str();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
