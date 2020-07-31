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

#ifndef AACE_ENGINE_UTILS_STRING_STRING_UTILS_H_
#define AACE_ENGINE_UTILS_STRING_STRING_UTILS_H_

#include <string>
#include <sstream>
#include <memory>

namespace aace {
namespace engine {
namespace utils {
namespace string {

static const char* TRUE = "true";
static const char* FALSE = "false";

bool equal(const std::string& str1, const std::string& str2, bool caseSensitive = true);
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
std::shared_ptr<std::stringstream> toStream(const std::string& str);

template <typename T>
std::string from(T value) {
    std::stringstream stream;
    stream << value;
    return stream.str();
}

}  // namespace string
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_STRING_STRING_UTILS_H_
