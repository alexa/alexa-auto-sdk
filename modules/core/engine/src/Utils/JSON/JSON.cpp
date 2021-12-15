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

#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <sstream>

#define JSON_POINTER(p) nlohmann::json::json_pointer(p.empty() == false && p[0] != '/' ? "/" + p : p)

// rapidjson (deprecated)
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace aace {
namespace engine {
namespace utils {
namespace json {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.utils.json.JSON");

// nlohmann

Value toJson(const std::string& str) {
    try {
        return nlohmann::json::parse(str);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return Value();
    }
}

Value toJson(std::shared_ptr<std::istream> stream) {
    try {
        return nlohmann::json::parse(*stream);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return Value();
    }
}

Value toJson(std::istream& stream) {
    try {
        return nlohmann::json::parse(stream);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return Value();
    }
}

std::string toString(const Value& root, bool prettyPrint) {
    return prettyPrint ? root.dump(3) : root.dump();
}

std::shared_ptr<std::stringstream> toStream(const Value& root, bool prettyPrint) {
    return std::make_shared<std::stringstream>(toString(root, prettyPrint));
}

bool merge(Value& into, const Value& from) {
    try {
        for (auto& next : from.items()) {
            auto key = next.key();

            if (into.find(key) != into.end()) {
                ThrowIfNot(next.value().is_object() && into[key].is_object(), "configurationAlreadySpecified:" + key);
                ThrowIfNot(merge(into[key], next.value()), "mergeChildNodeFailed");
            } else {
                into[key] = next.value();
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool has(const Value& root, const std::string& path, Type type) {
    try {
        return get(root, path, type) != nullptr;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool isType(const Value& value, Type type) {
    return value.type() == type;
}

Value get(const Value& root, const std::string& path, Type type) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            ThrowIf(type != Type::null && value.type() != type, "invalidType");
            return value;
        } else {
            return nullptr;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return Value();
    }
}

bool set(Value& root, const std::string& path, Value value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool push(Value& arr, Value value) {
    try {
        ThrowIfNot(isType(arr, Type::array), "invalidArrayType");
        arr.push_back(value);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string get(const Value& root, const std::string& path, const std::string& defaultValue) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            if (value.is_string() == false) {
                AACE_WARN(LX(TAG).d("reason", "invalidValueType"));
                return defaultValue;
            }
            return value;
        } else {
            return defaultValue;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return defaultValue;
    }
}

bool set(Value& root, const std::string& path, const std::string& value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string get(const Value& root, const std::string& path, const char* defaultValue) {
    return get(root, path, std::string(defaultValue != nullptr ? defaultValue : ""));
}

bool set(Value& root, const std::string& path, const char* value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool get(const Value& root, const std::string& path, bool defaultValue) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            if (value.is_boolean() == false) {
                AACE_WARN(LX(TAG).d("reason", "invalidValueType"));
                return defaultValue;
            }
            return value;
        } else {
            return defaultValue;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return defaultValue;
    }
}

bool set(Value& root, const std::string& path, bool value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

uint64_t get(const Value& root, const std::string& path, uint64_t defaultValue) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            if (value.is_number_unsigned() == false) {
                AACE_WARN(LX(TAG).d("reason", "invalidValueType"));
                return defaultValue;
            }
            return value;
        } else {
            return defaultValue;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return defaultValue;
    }
}

bool set(Value& root, const std::string& path, uint64_t value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t get(const Value& root, const std::string& path, int64_t defaultValue) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            if (value.is_number_integer() == false) {
                AACE_WARN(LX(TAG).d("reason", "invalidValueType"));
                return defaultValue;
            }
            return value;
        } else {
            return defaultValue;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return defaultValue;
    }
}

bool set(Value& root, const std::string& path, int64_t value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

double get(const Value& root, const std::string& path, double defaultValue) {
    try {
        auto ptr = JSON_POINTER(path);
        if (root.contains(ptr)) {
            auto value = root.at(ptr);
            if (value.is_number_float() == false) {
                AACE_WARN(LX(TAG).d("reason", "invalidValueType"));
                return defaultValue;
            }
            return value;
        } else {
            return defaultValue;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return defaultValue;
    }
}

bool set(Value& root, const std::string& path, double value) {
    try {
        root[JSON_POINTER(path)] = value;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

// rapidjson (deprecated)

bool merge(
    const rapidjson::Document::Object& into,
    const rapidjson::Document::Object& from,
    rapidjson::Document::AllocatorType& allocator,
    const std::string& path) {
    try {
        for (auto next = from.begin(); next != from.end(); next++) {
            auto intoNode = into.FindMember(next->name);

            if (intoNode != into.end()) {
                // both nodes must be objects to recursively copy
                ThrowIfNot(
                    next->value.IsObject() && intoNode->value.IsObject(),
                    std::string("configurationAlreadySpecified:") + next->name.GetString());

                // TODO: we should probably compare attributes to make sure they are the same

                // recursively merge the two objects
                ThrowIfNot(
                    merge(
                        intoNode->value.GetObject(),
                        next->value.GetObject(),
                        allocator,
                        path + "+" + next->name.GetString()),
                    "mergeChildNodeFailed");
            } else {
                // copy the new value into the target document since it doesn't already exist
                into.AddMember(
                    rapidjson::Value().CopyFrom(next->name, allocator),
                    rapidjson::Value().CopyFrom(next->value, allocator),
                    allocator);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "merge").d("reason", ex.what()).d("path", path));
        return false;
    }
}

std::shared_ptr<rapidjson::Document> parse(std::shared_ptr<std::istream> stream, rapidjson::Type type) {
    try {
        rapidjson::IStreamWrapper isw(*stream);

        auto document = std::make_shared<rapidjson::Document>();

        document->ParseStream(isw);

        ThrowIf(document->HasParseError(), GetParseError_En(document->GetParseError()));
        ThrowIfNot(document->GetType() == type, "invalidDocumentType");

        return document;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "parse").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<rapidjson::Document> parse(const std::string& value, rapidjson::Type type) {
    try {
        auto document = std::make_shared<rapidjson::Document>();

        document->Parse(value.c_str());

        ThrowIf(document->HasParseError(), GetParseError_En(document->GetParseError()));
        ThrowIfNot(document->GetType() == type, "invalidDocumentType");

        return document;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "parse").d("reason", ex.what()));
        return nullptr;
    }
}

std::string toString(const rapidjson::Document& document, bool prettyPrint) {
    rapidjson::StringBuffer buffer;
    if (prettyPrint) {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
    } else {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
    }

    return buffer.GetString();
}

std::shared_ptr<std::stringstream> toStream(const rapidjson::Document& document, bool prettyPrint) {
    return std::make_shared<std::stringstream>(toString(document, prettyPrint));
}

}  // namespace json
}  // namespace utils
}  // namespace engine
}  // namespace aace