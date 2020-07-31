/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Utils/Encoding/Base64.h"
#include "AACE/Engine/Core/EngineMacros.h"

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.utils.encoding.Base64");

namespace aace {
namespace engine {
namespace utils {
namespace encoding {

static const std::string BASE64_ENCODING_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool Base64::encode(std::istream& src, std::ostream& dest) {
    try {
        uint8_t in[3];
        uint8_t out[4];
        size_t bytesRead = 0;

        while (src.good()) {
            src.read((char*)in, 3);
            bytesRead = src.gcount();

            if (bytesRead == 0) break;

            if (bytesRead == 3) {
                out[0] = (in[0] & 0xfc) >> 2;
                out[1] = ((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4);
                out[2] = ((in[1] & 0x0f) << 2) + ((in[2] & 0xc0) >> 6);
                out[3] = in[2] & 0x3f;

                for (uint8_t n : out) {
                    dest << BASE64_ENCODING_TABLE[n];
                }
            }
        }

        if (bytesRead) {
            for (int j = bytesRead; j < 3; j++) in[j] = '\0';

            out[0] = (in[0] & 0xfc) >> 2;
            out[1] = ((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4);
            out[2] = ((in[1] & 0x0f) << 2) + ((in[2] & 0xc0) >> 6);

            for (int j = 0; j < bytesRead + 1; j++) {
                dest << BASE64_ENCODING_TABLE[out[j]];
            }

            for (int j = bytesRead; j < 3; j++) {
                dest << '=';
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "encode").d("reason", ex.what()));
        return false;
    }
}

bool Base64::decode(std::istream& src, std::ostream& dest) {
    try {
        uint8_t in[4];
        uint8_t out[3];
        int bytesRead = 0;
        char c;

        while (src.good() && src.get(c) && BASE64_ENCODING_TABLE.find(c) != std::string::npos) {
            in[bytesRead++] = c;

            if (bytesRead == 4) {
                for (int j = 0; j < 4; j++) {
                    in[j] = BASE64_ENCODING_TABLE.find(in[j]);
                }

                out[0] = (in[0] << 2) + ((in[1] & 0x30) >> 4);
                out[1] = ((in[1] & 0xf) << 4) + ((in[2] & 0x3c) >> 2);
                out[2] = ((in[2] & 0x3) << 6) + in[3];

                dest.write((char*)out, 3);

                bytesRead = 0;
            }
        }

        if (bytesRead) {
            for (int j = 0; j < bytesRead; j++) {
                in[j] = BASE64_ENCODING_TABLE.find(in[j]);
            }

            out[0] = (in[0] << 2) + ((in[1] & 0x30) >> 4);
            out[1] = ((in[1] & 0xf) << 4) + ((in[2] & 0x3c) >> 2);

            dest.write((char*)out, bytesRead - 1);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "decode").d("reason", ex.what()));
        return false;
    }
}

}  // namespace encoding
}  // namespace utils
}  // namespace engine
}  // namespace aace
