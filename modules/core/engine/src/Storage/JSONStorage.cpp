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

#include <fstream>

#include "AACE/Engine/Storage/JSONStorage.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace storage {

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.JSONStorage");

JSONStorage::JSONStorage(const std::string& path) : m_path(path) {
}

JSONStorage::~JSONStorage() {
}

std::shared_ptr<JSONStorage> JSONStorage::create(const std::string& path) {
    try {
        auto storage = std::shared_ptr<JSONStorage>(new JSONStorage(path));

        ThrowIfNot(storage->initialize(), "initializeFailed");

        return storage;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool JSONStorage::initialize() {
    try {
        std::ifstream is(m_path);

        if (is.good()) {
            rapidjson::IStreamWrapper isw(is);

            m_document.ParseStream(isw);

            ThrowIf(m_document.HasParseError(), GetParseError_En(m_document.GetParseError()));
            ThrowIfNot(m_document.IsObject(), "invalidDataFormat");
        } else {
            std::ofstream os(m_path);

            // if the new data file is not open, then we weren't able to create it
            ThrowIfNot(os.is_open(), "createDataFileFailed");

            // close the data file
            os.close();

            // set the root document object
            m_document.SetObject();

            // write the data file so that it is a valid empty json document
            ThrowIfNot(write(), "writeStorageFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::write() {
    try {
        std::ofstream os(m_path);
        rapidjson::OStreamWrapper osw(os);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

        ThrowIfNot(m_document.Accept(writer), "documentInvalid");

        m_dirty = false;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "write").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::put(const std::string& table, const std::string& key, const std::string& value) {
    try {
        auto root = m_document.GetObject();

        // add new table node to the storage document if it doesn't alread exist
        if (root.HasMember(table.c_str()) == false) {
            root.AddMember(
                rapidjson::Value().SetString(table.c_str(), table.length(), m_document.GetAllocator()),
                rapidjson::Value(rapidjson::kObjectType),
                m_document.GetAllocator());
        }

        // get a reference to the table node
        auto tableNode = root[table.c_str()].GetObject();

        // Add the new attribute value in the table node
        if (tableNode.HasMember(key.c_str())) {
            tableNode[key.c_str()].SetString(value.c_str(), value.length(), m_document.GetAllocator());
        } else {
            tableNode.AddMember(
                rapidjson::Value().SetString(key.c_str(), key.length(), m_document.GetAllocator()),
                rapidjson::Value().SetString(value.c_str(), value.length(), m_document.GetAllocator()),
                m_document.GetAllocator());
        }

        // write the storage file if we are not currently processing a transaction
        if (m_transactionInProgress == false) {
            ThrowIfNot(write(), "writeStorageFailed");
        } else {
            m_dirty = true;
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "put").d("reason", ex.what()));
        return false;
    }
}

std::string JSONStorage::get(const std::string& table, const std::string& key) {
    try {
        ThrowIf(m_transactionInProgress, "attemptingToAccessStorageWhileInTransaction");
        ThrowIfNot(containsKey(table, key), "invalidKey");

        auto root = m_document.GetObject();
        auto tableNode = root[table.c_str()].GetObject();

        return tableNode[key.c_str()].GetString();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "get").d("reason", ex.what()));
        return std::string();
    }
}

std::string JSONStorage::get(const std::string& table, const std::string& key, const std::string& defaultValue) {
    try {
        ThrowIf(m_transactionInProgress, "attemptingToAccessStorageWhileInTransaction");

        auto root = m_document.GetObject();

        if (root.HasMember(table.c_str())) {
            auto tableNode = root[table.c_str()].GetObject();

            if (tableNode.HasMember(key.c_str())) {
                return tableNode[key.c_str()].GetString();
            }
        }

        return defaultValue;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "get").d("reason", ex.what()));
        return defaultValue;
    }
}

bool JSONStorage::removeKey(const std::string& table, const std::string& key) {
    try {
        auto root = m_document.GetObject();

        ReturnIfNot(containsKey(table, key), true);
        ThrowIfNot(root[table.c_str()].RemoveMember(key.c_str()), "removeValueFailed");

        // write the storage file if we are not currently processing a transaction
        if (m_transactionInProgress == false) {
            ThrowIfNot(write(), "writeStorageFailed");
        } else {
            m_dirty = true;
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "remove").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::removeTable(const std::string& table) {
    try {
        auto root = m_document.GetObject();

        ReturnIfNot(root.HasMember(table.c_str()), true);
        ThrowIfNot(root.RemoveMember(table.c_str()), "removeTableFailed");

        // write the storage file if we are not currently processing a transaction
        if (m_transactionInProgress == false) {
            ThrowIfNot(write(), "writeStorageFailed");
        } else {
            m_dirty = true;
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "remove").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::containsKey(const std::string& table, const std::string& key) {
    try {
        ThrowIf(m_transactionInProgress, "attemptingToAccessStorageWhileInTransaction");

        auto root = m_document.GetObject();

        ReturnIfNot(root.HasMember(table.c_str()) && root[table.c_str()].IsObject(), false);

        auto tableNode = root[table.c_str()].GetObject();

        return tableNode.HasMember(key.c_str()) && tableNode[key.c_str()].IsString();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "contains").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::containsTable(const std::string& table) {
    try {
        ThrowIf(m_transactionInProgress, "attemptingToAccessStorageWhileInTransaction");

        auto root = m_document.GetObject();

        return root.HasMember(table.c_str()) && root[table.c_str()].IsObject();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "contains").d("reason", ex.what()));
        return false;
    }
}

std::vector<std::string> JSONStorage::keys(const std::string& table) {
    try {
        auto root = m_document.GetObject();
        std::vector<std::string> keys;

        if (containsTable(table)) {
            auto tableNode = root[table.c_str()].GetObject();

            for (rapidjson::Value::ConstMemberIterator it = tableNode.MemberBegin(); it != tableNode.MemberEnd();
                 ++it) {
                if (it->value.IsString()) {
                    keys.push_back(it->name.GetString());
                }
            }
        }

        return keys;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "keys").d("reason", ex.what()));
        return std::vector<std::string>();
    }
}

std::vector<JSONStorage::KeyValuePair> JSONStorage::list(const std::string& table) {
    try {
        auto root = m_document.GetObject();
        std::vector<LocalStorageInterface::KeyValuePair> keyValuePairList;

        if (containsTable(table)) {
            auto tableNode = root[table.c_str()].GetObject();

            for (rapidjson::Value::ConstMemberIterator it = tableNode.MemberBegin(); it != tableNode.MemberEnd();
                 ++it) {
                if (it->value.IsString()) {
                    keyValuePairList.push_back(KeyValuePair(it->name.GetString(), it->value.GetString()));
                }
            }
        }

        return keyValuePairList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "list").d("reason", ex.what()));
        return std::vector<LocalStorageInterface::KeyValuePair>();
    }
}

bool JSONStorage::begin() {
    try {
        std::unique_lock<std::mutex> lock(m_mutex);

        // wait until the transaction is complete
        bool success = m_notifyTransactionComplete.wait_for(
            lock, std::chrono::seconds(5), [this]() { return m_transactionInProgress == false; });

        // fail if we timed out
        ThrowIfNot(success, "beginTransactionFailed");

        // start the new transaction
        m_transactionInProgress = true;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "begin").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::commit() {
    try {
        ThrowIfNot(m_transactionInProgress, "invalidTransaction");

        // if the document is dirty then write it, otherwise just return
        if (m_dirty) {
            ThrowIfNot(write(), "commitTransactionFailed");
        }

        // mark the transaction complete and notify the conditional locks
        m_transactionInProgress = false;
        m_notifyTransactionComplete.notify_all();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "commit").d("reason", ex.what()));
        return false;
    }
}

bool JSONStorage::cancel() {
    try {
        ReturnIfNot(m_transactionInProgress, false);

        // restore the document model by parsing the file again
        std::ifstream is(m_path);
        rapidjson::IStreamWrapper isw(is);
        rapidjson::Document document;

        document.ParseStream(isw);

        ThrowIf(document.HasParseError(), GetParseError_En(document.GetParseError()));
        ThrowIfNot(document.IsObject(), "invalidDataFormat");

        m_document.GetObject() = document.GetObject();

        // mark the transaction complete and notify the conditional locks
        m_dirty = false;
        m_transactionInProgress = false;
        m_notifyTransactionComplete.notify_all();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "cancel").d("reason", ex.what()));
        return false;
    }
}

}  // namespace storage
}  // namespace engine
}  // namespace aace
