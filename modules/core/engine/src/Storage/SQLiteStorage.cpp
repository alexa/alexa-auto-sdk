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

#include "AACE/Engine/Storage/SQLiteStorage.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace storage {

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.SQLiteStorage");

SQLiteStorage::SQLiteStorage(const std::string& path) : m_path(path) {
}

SQLiteStorage::~SQLiteStorage() {
    // cancel a transaction if it is in progress
    if (m_transactionInProgress) {
        cancel();
    }

    // close the database
    if (m_db != nullptr) {
        if (sqlite3_close(m_db) != SQLITE_OK) {
            AACE_ERROR(LX(TAG, "~SQLiteStorage").d("reason", "closeDatabaseFailed"));
        }
    }
}

std::shared_ptr<SQLiteStorage> SQLiteStorage::create(const std::string& path) {
    try {
        auto storage = std::shared_ptr<SQLiteStorage>(new SQLiteStorage(path));

        ThrowIfNot(storage->initialize(), "initializeFailed");

        return storage;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool SQLiteStorage::initialize() {
    try {
        std::ifstream is(m_path);

        if (is.good()) {
            ThrowIf(
                sqlite3_open_v2(m_path.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr) !=
                    SQLITE_OK,
                "openDatabaseFailed");
        } else {
            ThrowIf(
                sqlite3_open_v2(
                    m_path.c_str(),
                    &m_db,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                    nullptr) != SQLITE_OK,
                "createDatabaseFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));

        if (m_db != nullptr) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }

        return false;
    }
}

std::string SQLiteStorage::createStatement(const char* stmt, ...) {
    try {
        va_list args;

        va_start(args, stmt);
        auto size = std::vsnprintf(nullptr, 0, stmt, args);

        std::string output(size + 1, 0);

        va_start(args, stmt);
        std::vsprintf(&output[0], stmt, args);

        va_end(args);

        return output;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createStatement").d("reason", ex.what()));
        return "";
    }
}

bool SQLiteStorage::checkTable(const std::string& table, bool create) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");

        std::string sql =
            createStatement("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%s';", table.c_str());

        bool exists = false;

        query(
            sql,
            [](void* data, int argc, char** argv, char** azColName) {
                *((bool*)data) = argc == 1 && std::atoi(*argv) == 1;
                return SQLITE_OK;
            },
            &exists);

        if (exists == false && create) {
            sql = createStatement(
                "CREATE TABLE '%s' (key STRING PRIMARY KEY NOT NULL,value STRING NOT NULL);", table.c_str());
            ThrowIfNot(query(sql) && checkTable(table), "createTableFailed");
            return true;
        }

        return exists;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "checkTable").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::checkKey(const std::string& table, const std::string& key) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ReturnIfNot(checkTable(table), false);

        bool exists = false;

        auto sql = createStatement("SELECT value FROM '%s' WHERE key='%s';", table.c_str(), key.c_str());
        query(
            sql,
            [](void* data, int argc, char** argv, char** azColName) {
                *((bool*)data) = argc == 1;
                return SQLITE_OK;
            },
            &exists);

        return exists;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "checkTable").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::query(const std::string& sql, int (*cb)(void*, int, char**, char**), void* data) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");

        char* errmsg = nullptr;
        bool success = sqlite3_exec(m_db, sql.c_str(), cb, data, &errmsg) == SQLITE_OK;

        if (errmsg != nullptr) {
            AACE_ERROR(LX(TAG, "query").d("reason", errmsg).sensitive("q", sql));
            sqlite3_free(errmsg);
        }

        return success;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "query").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::put(const std::string& table, const std::string& key, const std::string& value) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(checkTable(table, true), "invalidTable");

        std::string sql;

        if (containsKey(table, key)) {
            sql = createStatement(
                "UPDATE '%s' SET value='%s' WHERE key='%s';", table.c_str(), value.c_str(), key.c_str());
        } else {
            sql = createStatement(
                "INSERT INTO '%s' (key,value) VALUES ('%s','%s');", table.c_str(), key.c_str(), value.c_str());
        }

        ThrowIfNot(query(sql), "executeSqlStatementFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "put").d("reason", ex.what()));
        return false;
    }
}

std::string SQLiteStorage::get(const std::string& table, const std::string& key) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(checkTable(table, false), "invalidTable");

        std::string output;

        auto sql = createStatement("SELECT value FROM '%s' WHERE key='%s';", table.c_str(), key.c_str());
        query(
            sql,
            [](void* data, int argc, char** argv, char** azColName) {
                if (argc == 1) {
                    ((std::string*)data)->append(*argv);
                }
                return SQLITE_OK;
            },
            &output);

        return output;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "get").d("reason", ex.what()));
        return std::string();
    }
}

std::string SQLiteStorage::get(const std::string& table, const std::string& key, const std::string& defaultValue) {
    auto value = get(table, key);
    return value.empty() ? defaultValue : value;
}

bool SQLiteStorage::removeKey(const std::string& table, const std::string& key) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(containsKey(table, key), "invalidKey");

        auto sql = createStatement("DELETE FROM '%s' WHERE key='%s';", table.c_str(), key.c_str());
        ThrowIfNot(query(sql), "removeKeyFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "removeKey").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::removeTable(const std::string& table) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(containsTable(table), "invalidTable");

        auto sql = createStatement("DELETE FROM '%s';", table.c_str());
        ThrowIfNot(query(sql), "deleteFromTableFailed");

        sql = createStatement("DROP TABLE IF EXISTS '%s';", table.c_str());
        ThrowIfNot(query(sql), "dropTableFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "removeTable").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::containsKey(const std::string& table, const std::string& key) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        return checkKey(table, key);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "containsKey").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::containsTable(const std::string& table) {
    return checkTable(table);
}

std::vector<std::string> SQLiteStorage::keys(const std::string& table) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        std::vector<std::string> keys;

        auto sql = createStatement("SELECT key FROM '%s';", table.c_str());
        query(
            sql,
            [](void* data, int argc, char** argv, char** azColName) {
                ((std::vector<std::string>*)data)->push_back(*argv);
                return SQLITE_OK;
            },
            &keys);

        return keys;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "keys").d("reason", ex.what()));
        return std::vector<std::string>();
    }
}

std::vector<SQLiteStorage::KeyValuePair> SQLiteStorage::list(const std::string& table) {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        std::vector<LocalStorageInterface::KeyValuePair> keyValuePairList;

        auto sql = createStatement("SELECT key,value FROM '%s';", table.c_str());
        query(
            sql,
            [](void* data, int argc, char** argv, char** azColName) {
                ((std::vector<LocalStorageInterface::KeyValuePair>*)data)->push_back({argv[0], argv[1]});
                return SQLITE_OK;
            },
            &keyValuePairList);

        return keyValuePairList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "list").d("reason", ex.what()));
        return std::vector<LocalStorageInterface::KeyValuePair>();
    }
}

bool SQLiteStorage::begin() {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(query("BEGIN TRANSACTION;"), "beginTransactionFailed");

        m_transactionInProgress = true;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "begin").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::commit() {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(m_transactionInProgress, "transactionNotInProgresss");
        ThrowIfNot(query("COMMIT TRANSACTION;"), "commitTransactionFailed");

        m_transactionInProgress = false;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "commit").d("reason", ex.what()));
        return false;
    }
}

bool SQLiteStorage::cancel() {
    try {
        ThrowIfNull(m_db, "invalidDatabase");
        ThrowIfNot(m_transactionInProgress, "transactionNotInProgresss");
        ThrowIfNot(query("ROLLBACK TRANSACTION;"), "cancelTransactionFailed");

        m_transactionInProgress = false;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "cancel").d("reason", ex.what()));
        return false;
    }
}

}  // namespace storage
}  // namespace engine
}  // namespace aace
