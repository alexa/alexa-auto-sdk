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

#ifndef AACE_MOBILE_BRIDGE_TRANSPORT_H
#define AACE_MOBILE_BRIDGE_TRANSPORT_H

#include <cstddef>
#include <iostream>
#include <string>

/** @file */

namespace aace {
namespace mobileBridge {

struct Transport {
    enum class TransportType { UNDEFINED, BLUETOOTH, WIFI, EAP, USB, TEST };

    friend std::ostream& operator<<(std::ostream& stream, const TransportType& type) {
        switch (type) {
            case Transport::TransportType::BLUETOOTH:
                stream << "BLUETOOTH";
                break;
            case Transport::TransportType::WIFI:
                stream << "WIFI";
                break;
            case Transport::TransportType::USB:
                stream << "USB";
                break;
            case Transport::TransportType::EAP:
                stream << "EAP";
                break;
            case Transport::TransportType::UNDEFINED:
                stream << "UNDEFINED";
                break;
            case Transport::TransportType::TEST:
                stream << "TEST";
                break;
            default:
                stream.setstate(std::ios_base::failbit);
                break;
        }
        return stream;
    }

    Transport(std::string id, TransportType type);

    std::string id;
    TransportType type;
};

}  // namespace mobileBridge
}  // namespace aace

#endif  // AACE_MOBILE_BRIDGE_TRANSPORT_H
