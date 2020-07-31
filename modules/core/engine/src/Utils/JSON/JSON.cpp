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

#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <sstream>

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
    auto stream = toStream(document, prettyPrint);
    return stream != nullptr ? stream->str().c_str() : "";
}

std::shared_ptr<std::stringstream> toStream(const rapidjson::Document& document, bool prettyPrint) {
    rapidjson::StringBuffer buffer;

    if (prettyPrint) {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
    } else {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
    }

    return std::make_shared<std::stringstream>(buffer.GetString());
}

}  // namespace json
}  // namespace utils
}  // namespace engine
}  // namespace aace
