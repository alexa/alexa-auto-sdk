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

#ifndef AACE_ENGINE_UTILS_JSON_H_
#define AACE_ENGINE_UTILS_JSON_H_

#include <string>
#include <istream>
#include <memory>

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace utils {
namespace json {

bool merge(
    const rapidjson::Document::Object& into,
    const rapidjson::Document::Object& from,
    rapidjson::Document::AllocatorType& allocator,
    const std::string& path = "");

std::shared_ptr<rapidjson::Document> parse(
    std::shared_ptr<std::istream> stream,
    rapidjson::Type type = rapidjson::kObjectType);
std::shared_ptr<rapidjson::Document> parse(const std::string& value, rapidjson::Type type = rapidjson::kObjectType);

std::string toString(const rapidjson::Document& document, bool prettyPrint = true);

std::shared_ptr<std::stringstream> toStream(const rapidjson::Document& document, bool prettyPrint = true);

}  // namespace json
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_JSON_H_
