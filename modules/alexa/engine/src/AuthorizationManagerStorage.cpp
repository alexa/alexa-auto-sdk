/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <memory>
#include <sstream>

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/Alexa/AuthorizationManagerStorage.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace alexaClientSDK::avsCommon::sdkInterfaces::storage;

/// String to identify log entries originating from this file.
static const std::string TAG{"AuthorizationManagerStorage"};

/// Component name for Misc DB
static const std::string COMPONENT_NAME = "authorizationManager";

/// Misc DB table for active authorization
static const std::string ACTIVE_AUTHORIZATION_TABLE = "activeAdapter";

/// Key for authorization service in Misc DB table.
static const std::string AUTHORIZATION_SERVICE_KEY = "service";

/// Key for authorization service state in Misc DB table.
static const std::string AUTHORIZATION_SERVICE_STATE_KEY = "state";

bool AuthorizationManagerStorage::saveAdapterState(AdapterState adapterState) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNot(
            m_miscStorage->put(
                COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, AUTHORIZATION_SERVICE_KEY, adapterState.first),
            "updatingTableForServiceFailed");

        ThrowIfNot(
            m_miscStorage->put(
                COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, AUTHORIZATION_SERVICE_STATE_KEY, adapterState.second),
            "updatingTableForStateFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG).d("reason", ex.what()).d("table", ACTIVE_AUTHORIZATION_TABLE).d("component", COMPONENT_NAME));
        return false;
    }
}

bool AuthorizationManagerStorage::loadAdapterState(AdapterState& adapterState) {
    AACE_DEBUG(LX(TAG));
    try {
        std::string service;
        ThrowIfNot(
            m_miscStorage->get(COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, AUTHORIZATION_SERVICE_KEY, &service),
            "getServiceFailed");

        std::string state;
        ThrowIfNot(
            m_miscStorage->get(COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, AUTHORIZATION_SERVICE_STATE_KEY, &state),
            "getStateFailed");

        adapterState = {service, state};
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG).d("reason", ex.what()).d("table", ACTIVE_AUTHORIZATION_TABLE).d("component", COMPONENT_NAME));
        adapterState = {"", ""};
        return false;
    }
}

bool AuthorizationManagerStorage::clearAdapterStateTable() {
    AACE_DEBUG(LX(TAG));
    try {
        bool verificationStateTableExists = false;
        if (m_miscStorage->tableExists(COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, &verificationStateTableExists)) {
            if (verificationStateTableExists) {
                ThrowIfNot(
                    m_miscStorage->clearTable(COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE), "clearingTableFailed");
            }
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG).d("reason", ex.what()).d("table", ACTIVE_AUTHORIZATION_TABLE).d("component", COMPONENT_NAME));
        return false;
    }
}

std::shared_ptr<AuthorizationManagerStorage> AuthorizationManagerStorage::create(
    std::shared_ptr<MiscStorageInterface> storage) {
    try {
        ThrowIfNull(storage, "invalidStorageReference");
        auto authorizationManagerStorage =
            std::shared_ptr<AuthorizationManagerStorage>(new AuthorizationManagerStorage(storage));
        ThrowIfNot(authorizationManagerStorage->initialize(), "initializeFailed");
        return authorizationManagerStorage;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG).d("reason", ex.what()).d("table", ACTIVE_AUTHORIZATION_TABLE).d("component", COMPONENT_NAME));
        return nullptr;
    }
}

AuthorizationManagerStorage::AuthorizationManagerStorage(std::shared_ptr<MiscStorageInterface> storage) :
        m_miscStorage{storage} {
}

bool AuthorizationManagerStorage::initialize() {
    try {
        if (!m_miscStorage->isOpened() && !m_miscStorage->open()) {
            AACE_DEBUG(LX(TAG).m("Couldn't open misc database. Creating."));
            ThrowIfNot(m_miscStorage->createDatabase(), "Could not create misc database");
        }

        bool tableExists = false;
        ThrowIfNot(
            m_miscStorage->tableExists(COMPONENT_NAME, ACTIVE_AUTHORIZATION_TABLE, &tableExists),
            "Could not check state table information in misc database");
        if (!tableExists) {
            AACE_DEBUG(LX(TAG).m("Creating new table in misc database"));
            ThrowIfNot(
                m_miscStorage->createTable(
                    COMPONENT_NAME,
                    ACTIVE_AUTHORIZATION_TABLE,
                    MiscStorageInterface::KeyType::STRING_KEY,
                    MiscStorageInterface::ValueType::STRING_VALUE),
                "Cannot create table");
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG).d("reason", ex.what()).d("table", ACTIVE_AUTHORIZATION_TABLE).d("component", COMPONENT_NAME));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
