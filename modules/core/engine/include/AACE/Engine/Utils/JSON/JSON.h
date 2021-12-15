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

#ifndef AACE_ENGINE_UTILS_JSON_H_
#define AACE_ENGINE_UTILS_JSON_H_

#include <string>
#include <istream>
#include <memory>
#include <nlohmann/json.hpp>

// rapidjson (deprecated)
#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace utils {
namespace json {

using Value = nlohmann::json;
using Type = nlohmann::json::value_t;

// nlohmann

Value toJson(const std::string& str);
Value toJson(std::shared_ptr<std::istream> stream);
Value toJson(std::istream& stream);
std::string toString(const Value& root, bool prettyPrint = true);
std::shared_ptr<std::stringstream> toStream(const Value& root, bool prettyPrint = true);
bool merge(Value& into, const Value& from);

bool has(const Value& root, const std::string& path, Type type = Type::null);
bool isType(const Value& value, Type type);

Value get(const Value& root, const std::string& path, Type type = Type::null);
bool set(Value& root, const std::string& path, Value value);
bool push(Value& arr, Value value);

std::string get(const Value& root, const std::string& path, const std::string& defaultValue);
bool set(Value& root, const std::string& path, const std::string& value);

std::string get(const Value& root, const std::string& path, const char* defaultValue);
bool set(Value& root, const std::string& path, const char* value);

bool get(const Value& root, const std::string& path, bool defaultValue);
bool set(Value& root, const std::string& path, bool value);

uint64_t get(const Value& root, const std::string& path, uint64_t defaultValue);
bool set(Value& root, const std::string& path, uint64_t value);

int64_t get(const Value& root, const std::string& path, int64_t defaultValue);
bool set(Value& root, const std::string& path, int64_t value);

double get(const Value& root, const std::string& path, double defaultValue);
bool set(Value& root, const std::string& path, double value);

// rapidjson (deprecated)
bool merge(
    const rapidjson::Document::Object& into,
    const rapidjson::Document::Object& from,
    rapidjson::Document::AllocatorType& allocator,
    const std::string& path = "");

std::shared_ptr<rapidjson::Document> parse(
    std::shared_ptr<std::istream> stream,
    rapidjson::Type type = rapidjson::kObjectType);
std::shared_ptr<rapidjson::Document> parse(const std::string& value, rapidjson::Type type = rapidjson::kObjectType);

std::string toString(const rapidjson::Document& document, bool prettyPrint = false);

std::shared_ptr<std::stringstream> toStream(const rapidjson::Document& document, bool prettyPrint = false);

}  // namespace json
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_JSON_H_