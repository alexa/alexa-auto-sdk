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

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace core {

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.EngineService");

EngineService::EngineService(const ServiceDescription& description) :
        m_description(description), m_initialized(false), m_running(false) {
}

EngineService::~EngineService() {
    if (m_running) {
        AACE_WARN(LX(TAG, "~EngineService")
                      .m("EngineService was not stopped before being deleted")
                      .d("service", getDescription().getType()));
    } else if (m_initialized) {
        AACE_WARN(LX(TAG, "~EngineService")
                      .m("EngineService was not shutdown before being deleted")
                      .d("service", getDescription().getType()));
    }

    AACE_DEBUG(LX(TAG, "~EngineService").m(getDescription().getType()));
}

bool EngineService::handleInitializeEngineEvent(std::shared_ptr<aace::engine::core::EngineContext> context) {
    try {
        ThrowIf(m_initialized, "serviceAlreadyInitialized");

        // set the service engine context
        m_context = context;

        // delegate the intialize request to the service handler
        ThrowIfNot(initialize(), "initializeServiceFailed");

        // set the service initialized flag to true
        m_initialized = true;

        return true;
    } catch (std::exception& ex) {
        m_context.reset();
        AACE_ERROR(LX(TAG, "handleInitializeEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleConfigureEngineEvent(std::shared_ptr<std::istream> configuration) {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");
        ThrowIfNot(configuration != nullptr ? configure(configuration) : configure(), "configureServiceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleConfigureEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleShutdownEngineEvent() {
    try {
        if (m_initialized == false) {
            AACE_WARN(LX(TAG, "handleShutdownEngineEvent")
                          .m("Attempting to shutdown service that is not initialized - doing nothing."));
            return true;
        }

        // if the service is running then stop it first
        if (isRunning()) {
            ThrowIfNot(stop(), "stopServiceFailed");
        }

        // call shutdownService method
        ThrowIfNot(shutdown(), "shutdownServiceFailed");

        // set the service initialized flag to false
        m_initialized = false;

        m_context.reset();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleShutdownEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handlePreRegisterEngineEvent() {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");
        ThrowIfNot(preRegister(), "preRegisterFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handlePreRegisterEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handlePostRegisterEngineEvent() {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");
        ThrowIfNot(postRegister(), "postRegisterFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handlePostRegisterEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleSetupEngineEvent() {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");
        ThrowIfNot(setup(), "setupServiceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleSetupEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleStartEngineEvent() {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");
        ThrowIf(m_running, "serviceAlreadyRunning");
        ThrowIfNot(start(), "startServiceFailed");

        // set the service running flag to true
        m_running = true;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleStartEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleStopEngineEvent() {
    try {
        ThrowIfNot(m_initialized, "serviceNotInitialized");

        if (m_running == false) {
            AACE_WARN(
                LX(TAG, "handleStopEngineEvent").m("Attempting to stop service that is not running - doing nothing."));
            return true;
        }

        ThrowIfNot(stop(), "stopServiceFailed");

        // set the service running and initialized flags to false
        m_running = false;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleStopEngineEvent").d("reason", ex.what()));
        return false;
    }
}

bool EngineService::handleRegisterPlatformInterfaceEngineEvent(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    return registerPlatformInterface(platformInterface);
}

bool EngineService::isRunning() {
    return m_running;
}

bool EngineService::initialize() {
    return true;
}

bool EngineService::configure() {
    return true;
}

bool EngineService::configure(std::shared_ptr<std::istream> configuration) {
    return true;
}

bool EngineService::preRegister() {
    return true;
}

bool EngineService::postRegister() {
    return true;
}

bool EngineService::setup() {
    return true;
}

bool EngineService::start() {
    return true;
}

bool EngineService::stop() {
    return true;
}

bool EngineService::shutdown() {
    return true;
}

bool EngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    return false;
}

std::shared_ptr<aace::engine::core::EngineContext> EngineService::getContext() {
    return m_context;
}

const ServiceDescription& EngineService::getDescription() {
    return m_description;
}

}  // namespace core
}  // namespace engine
}  // namespace aace
