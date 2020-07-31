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

#ifndef AACE_ENGINE_STORAGE_JSON_STORAGE_H
#define AACE_ENGINE_STORAGE_JSON_STORAGE_H

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>

#include <rapidjson/document.h>

#include "LocalStorageInterface.h"

namespace aace {
namespace engine {
namespace storage {

class JSONStorage : public LocalStorageInterface {
public:
    static std::shared_ptr<JSONStorage> create(const std::string& path);

    virtual ~JSONStorage();

private:
    JSONStorage(const std::string& path);

    bool initialize();
    bool write();

public:
    bool put(const std::string& table, const std::string& key, const std::string& value) override;
    std::string get(const std::string& table, const std::string& key) override;
    std::string get(const std::string& table, const std::string& key, const std::string& defaultValue) override;
    bool removeKey(const std::string& table, const std::string& key) override;
    bool removeTable(const std::string& table) override;
    bool containsKey(const std::string& table, const std::string& key) override;
    bool containsTable(const std::string& table) override;
    std::vector<std::string> keys(const std::string& table) override;
    std::vector<KeyValuePair> list(const std::string& table) override;
    bool begin() override;
    bool commit() override;
    bool cancel() override;

private:
    std::string m_path;
    rapidjson::Document m_document;
    bool m_transactionInProgress = false;
    bool m_dirty = false;

    std::mutex m_mutex;
    std::condition_variable m_notifyTransactionComplete;
};

}  // namespace storage
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_STORAGE_JSON_STORAGE_H
