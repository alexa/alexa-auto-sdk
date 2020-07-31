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

#include <typeinfo>
#include <algorithm>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/Metrics/MetricsEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace metrics {

// String to identify log entries originating from this file.
static const std::string TAG("aace.metrics.MetricsEngineService");

// register the service
REGISTER_SERVICE(MetricsEngineService);

MetricsEngineService::MetricsEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool MetricsEngineService::shutdown() {
    if (m_metricsUploaderEngineImpl != nullptr) {
        // get the logger service interface
        auto loggerServiceInterface =
            getContext()->getServiceInterface<aace::engine::logger::LoggerServiceInterface>("aace.logger");
        ThrowIfNull(loggerServiceInterface, "invalidLoggerServiceInterface");

        // remove the metrics uploader from the logger
        loggerServiceInterface->removeSink(m_metricsUploaderEngineImpl->getId());
    }

    return true;
}

bool MetricsEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::metrics::MetricsUploader>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool MetricsEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::metrics::MetricsUploader> metricsUploader) {
    try {
        ThrowIfNotNull(m_metricsUploaderEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the logger service interface
        auto loggerServiceInterface =
            getContext()->getServiceInterface<aace::engine::logger::LoggerServiceInterface>("aace.logger");
        ThrowIfNull(loggerServiceInterface, "invalidLoggerServiceInterface");

        // create the metrics uploader engine implementation
        m_metricsUploaderEngineImpl = aace::engine::metrics::MetricsUploaderEngineImpl::create(metricsUploader);
        ThrowIfNull(m_metricsUploaderEngineImpl, "createMetricsUploaderEngineImplFailed");

        // add the uploader service impl to the logger service
        loggerServiceInterface->addSink(m_metricsUploaderEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<MetricsUploader>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
