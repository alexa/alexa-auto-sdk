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

#ifndef AASB_UTILS_UUID_H_
#define AASB_UTILS_UUID_H_

#include <AACE/Engine/Utils/UUID/UUID.h>

namespace aasb {
namespace utils {
namespace uuid {

// alias the engine implementation
const auto generateUUID = aace::engine::utils::uuid::generateUUID;
const auto compare = aace::engine::utils::uuid::compare;

}  // namespace uuid
}  // namespace utils
}  // namespace aasb

#endif  // AASB_UTILS_UUID_H_
