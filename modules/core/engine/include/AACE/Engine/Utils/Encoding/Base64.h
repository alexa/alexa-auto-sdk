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

#ifndef AACE_ENGINE_UTILS_ENCODING_BASE64_H
#define AACE_ENGINE_UTILS_ENCODING_BASE64_H

#include <memory>
#include <istream>
#include <ostream>

namespace aace {
namespace engine {
namespace utils {
namespace encoding {

class Base64 {
private:
    Base64() = default;

public:
    static bool encode(std::istream& src, std::ostream& dest);
    static bool decode(std::istream& src, std::ostream& dest);
};

}  // namespace encoding
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_ENCODING_BASE64_H
