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

#ifndef AACE_ENGINE_STORAGE_INTERFACE_LOCAL_STORAGE_INTERFACE_H
#define AACE_ENGINE_STORAGE_INTERFACE_LOCAL_STORAGE_INTERFACE_H

#include <vector>
#include <memory>
#include <string>

namespace aace {
namespace engine {
namespace storage {

class LocalStorageInterface {
public:
    using KeyValuePair = std::pair<std::string, std::string>;

    virtual ~LocalStorageInterface();

public:
    virtual bool put(const std::string& table, const std::string& key, const std::string& value) = 0;
    virtual std::string get(const std::string& table, const std::string& key) = 0;
    virtual std::string get(const std::string& table, const std::string& key, const std::string& defaultValue) = 0;
    virtual bool removeKey(const std::string& table, const std::string& key) = 0;
    virtual bool removeTable(const std::string& table) = 0;
    virtual bool containsKey(const std::string& table, const std::string& key) = 0;
    virtual bool containsTable(const std::string& table) = 0;
    virtual std::vector<std::string> keys(const std::string& table) = 0;
    virtual std::vector<KeyValuePair> list(const std::string& table) = 0;
    virtual bool begin() = 0;
    virtual bool commit() = 0;
    virtual bool cancel() = 0;
};

}  // namespace storage
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_STORAGE_INTERFACE_LOCAL_STORAGE_INTERFACE_H
