/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_BLUETOOTH_BYTE_ARRAY_H
#define AACE_BLUETOOTH_BYTE_ARRAY_H

#include <vector>
#include <memory>

namespace aace {
namespace bluetooth {

using ByteArray = std::vector<uint8_t>;
using ByteArrayPtr = std::unique_ptr<std::vector<uint8_t>>;

ByteArrayPtr createByteArray(size_t size = 0, bool resize = true);

}  // namespace bluetooth
}  // namespace aace

#endif
