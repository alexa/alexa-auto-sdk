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

#ifndef AASB_UTILS_STRING_UTILS_H_
#define AASB_UTILS_STRING_UTILS_H_

#include <AACE/Engine/Utils/String/StringUtils.h>

namespace aasb {
namespace utils {
namespace string {

// alias the engine implementation
const auto equal = aace::engine::utils::string::equal;
const auto toLower = aace::engine::utils::string::toLower;
const auto toUpper = aace::engine::utils::string::toUpper;
const auto toStream = aace::engine::utils::string::toStream;

}  // namespace string
}  // namespace utils
}  // namespace aasb

#endif  // AASB_UTILS_STRING_UTILS_H_
