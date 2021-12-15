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

#include <AACE/Bluetooth/ByteArray.h>

namespace aace {
namespace bluetooth {

ByteArrayPtr createByteArray(size_t size, bool resize) {
    auto byteArray = ByteArrayPtr(new ByteArray());

    if (size > 0) {
        if (resize) {
            byteArray->resize(size);
        } else {
            byteArray->reserve(size);
        }
    }

    return byteArray;
}

}  // namespace bluetooth
}  // namespace aace
