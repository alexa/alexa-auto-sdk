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

#include <AACE/Engine/Utils/String/StringUtils.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <cctype>

namespace aace {
namespace engine {
namespace utils {
namespace string {

bool equal(const std::string& str1, const std::string& str2, bool caseSensitive) {
    // return immedieately if the string length is different
    ReturnIf(str1.size() != str2.size(), false);

    // call std::strcmp for case sensitive comparison
    if (caseSensitive) {
        return str1 == str2;
    } else {
        // iterate through the chars in the two strings
        for (auto j = 0; j < str1.size(); j++) {
            ReturnIf(std::tolower(str1[j]) != std::tolower(str2[j]), false);
        }
    }

    return true;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) -> unsigned char {
        return static_cast<unsigned char>(std::tolower(c));
    });
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) -> unsigned char {
        return static_cast<unsigned char>(std::toupper(c));
    });
    return result;
}

std::shared_ptr<std::stringstream> toStream(const std::string& str) {
    auto stream = std::make_shared<std::stringstream>();
    *stream << str;
    return stream;
}

}  // namespace string
}  // namespace utils
}  // namespace engine
}  // namespace aace
