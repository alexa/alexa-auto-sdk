/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <AACE/Core/CoreProperties.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/MessageBroker/Message.h>
#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/Metrics/AASBMetricsDispatcher.h>
#include <AACE/Engine/Metrics/DataPoint.h>
#include <AACE/Engine/Metrics/MetricContext.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/MetricsEngineService.h>
#include <AACE/Engine/Metrics/MetricsEmissionInterface.h>
#include <AACE/Engine/Metrics/MetricsEngineService.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>
#include <AACE/Engine/Vehicle/VehicleConfigServiceInterface.h>

#include <nlohmann/json.hpp>
#include <openssl/sha.h>

using json = nlohmann::json;
using namespace aace::engine::vehicle;

namespace aace {
namespace engine {
namespace metrics {

// Config keys in aace.metrics config
static const std::string KEY_DEVICE_ID_TAG("metricDeviceIdTag");
static const std::string KEY_STORAGE_PATH("metricStoragePath");
static const std::string KEY_BUFFER_CONFIG("bufferConfig");
static const std::string KEY_ASSISTANT_ID("assistantId");
static const std::string KEY_MAX_PRE_ENABLEMENT("maxMetricsBufferedPreEnablement");
static const std::string KEY_MIN_AASB("minMetricsInAasbMessage");
static const std::string KEY_PUBLISH_PERIOD("aasbPublishPeriodSeconds");
static const std::string KEY_UPLOADER_CONFIG("uploaderConfig");
static const std::string KEY_METRIC_CONFIGS("metricConfigs");
static const std::string KEY_PROGRAM("program");
static const std::string KEY_SOURCE("source");
static const std::string KEY_UPLOAD_RULES("uploadRules");
static const std::string KEY_GROUP_ID("groupId");
static const std::string KEY_SCHEMA_ID("schemaId");
static const std::string KEY_BUILD_TYPE("buildType");

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricsEngineService");

// register the service
REGISTER_SERVICE(MetricsEngineService);

MetricsEngineService::MetricsEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool MetricsEngineService::configure(std::shared_ptr<std::istream> configuration) {
    auto messageBrokerService =
        getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
            "aace.messageBroker");
    if (messageBrokerService == nullptr) {
        AACE_ERROR(LX(TAG, "MessageBrokerServiceInterface is null"));
        return false;
    }
    auto messageBroker = messageBrokerService->getMessageBroker();

    json config;
    try {
        config = json::parse(*configuration);
        AACE_DEBUG(LX(TAG).sensitive("config", config.dump()));
        ThrowIfNot(config.contains(KEY_STORAGE_PATH), "Missing " + KEY_STORAGE_PATH);
        m_storagePath = config.at(KEY_STORAGE_PATH);
        ThrowIf(m_storagePath.empty(), "Value cannot be empty. Key=" + KEY_STORAGE_PATH);

        ThrowIfNot(config.contains(KEY_DEVICE_ID_TAG), "Missing " + KEY_DEVICE_ID_TAG);
        const std::string deviceIdTag = config.at(KEY_DEVICE_ID_TAG);
        ThrowIf(deviceIdTag.empty(), "Value cannot be empty. Key=" + KEY_DEVICE_ID_TAG);

        AgentIdType alexaId = aace::engine::utils::agent::AGENT_ID_ALEXA;
        std::lock_guard<std::mutex> lock(m_processorsMutex);

        if (config.contains(KEY_BUFFER_CONFIG)) {
            json bufferConfig = config.at(KEY_BUFFER_CONFIG);
            auto numBufferConfigs = bufferConfig.size();
            for (const auto& itr : bufferConfig.items()) {
                const json& item = itr.value();
                // Assume config is for Alexa if agent not specified and there
                // is only one buffer config
                AgentIdType id = aace::engine::utils::agent::AGENT_ID_ALEXA;
                if (!item.contains(KEY_ASSISTANT_ID)) {
                    if (numBufferConfigs > 1) {
                        AACE_ERROR(LX(TAG, "Buffer config is missing assistantId"));
                        return false;
                    }
                    AACE_DEBUG(LX(TAG, "Buffer config is missing assistantId; assuming Alexa assistant"));
                } else {
                    id = item.at(KEY_ASSISTANT_ID);
                }

                AACE_DEBUG(LX(TAG, "Configuring buffer for assistantId " + std::to_string(id)));
                int maxPreEnablement = DEFAULT_PRE_DISPATCH_BUFFER_SIZE;
                int minMetricsInMessage = DEFAULT_AASB_MIN_METRICS_FOR_PUBLISH;
                int publishPeriod = DEFAULT_AASB_METRICS_PUBLISH_SECONDS;
                if (item.contains(KEY_MAX_PRE_ENABLEMENT)) {
                    maxPreEnablement = item.at(KEY_MAX_PRE_ENABLEMENT);
                }
                if (item.contains(KEY_MIN_AASB)) {
                    minMetricsInMessage = item.at(KEY_MIN_AASB);
                }
                if (item.contains(KEY_PUBLISH_PERIOD)) {
                    publishPeriod = item.at(KEY_PUBLISH_PERIOD);
                }
                bool hasDispatchConditions = id == aace::engine::utils::agent::AGENT_ID_ALEXA;
                auto dispatcher = AASBMetricsDispatcher::create(
                    messageBroker, id, hasDispatchConditions, maxPreEnablement, publishPeriod, minMetricsInMessage);
                if (dispatcher == nullptr) {
                    AACE_ERROR(LX(TAG, "Failed to create MessageDispatcherInterface for agent").d("agentId", id));
                    return false;
                }
                auto processor = std::unique_ptr<MetricProcessor>(new MetricProcessor{nullptr, std::move(dispatcher)});
                m_metricProcessors.emplace(id, std::move(processor));
            }
        }
        // Create a dispatcher with default config for Alexa if config wasn't specified
        if (m_metricProcessors.find(alexaId) == m_metricProcessors.end()) {
            AACE_DEBUG(LX(TAG, "Creating default metrics dispatcher for Alexa"));
            auto alexaDispatcher = AASBMetricsDispatcher::create(messageBroker, alexaId, true);
            if (alexaDispatcher == nullptr) {
                AACE_ERROR(LX(TAG, "Failed to create default metrics dispatcher for Alexa"));
                return false;
            }
            auto alexaProcessor =
                std::unique_ptr<MetricProcessor>(new MetricProcessor{nullptr, std::move(alexaDispatcher)});
            m_metricProcessors.emplace(alexaId, std::move(alexaProcessor));
        }

        if (config.contains(KEY_UPLOADER_CONFIG)) {
            const json uploaderConfigJson = config.at(KEY_UPLOADER_CONFIG);
            ThrowIfNot(uploaderConfigJson.contains(KEY_METRIC_CONFIGS), "Missing metricConfig key in uploaderConfig");
            const json metricConfigs = uploaderConfigJson.at(KEY_METRIC_CONFIGS);
            ThrowIfNot(metricConfigs.is_array(), "Value must be an array: " + KEY_METRIC_CONFIGS);
            ThrowIf(metricConfigs.size() == 0, "Array must be nonempty if present: " + KEY_METRIC_CONFIGS);
            auto uploaderConfigs = MetricsUploadConfiguration::createConfigurations(metricConfigs);
            for (auto uploaderConfig : uploaderConfigs) {
                AgentIdType agentId = uploaderConfig.first;
                auto config = std::make_shared<MetricsUploadConfiguration>(uploaderConfig.second);
                AACE_INFO(LX(TAG, "Created MetricsUploadConfiguration for agent")
                              .d("agentId", agentId)
                              .d("numberOfRules", config->size()));
                if (m_metricProcessors.find(agentId) != m_metricProcessors.end()) {
                    // The agent will already be present in m_metricProcessors if it had a dispatcher created with
                    // bufferConfig. Its default null MetricsUploadConfiguration will be replaced according to
                    // the uploaderConfig
                    AACE_DEBUG(
                        LX(TAG, "Replacing existing MetricsUploadConfiguration for agent").d("agentId", agentId));
                    m_metricProcessors.at(agentId)->uploadConfig = config;
                } else {
                    bool hasDispatchConditions = agentId == aace::engine::utils::agent::AGENT_ID_ALEXA;
                    auto dispatcher = AASBMetricsDispatcher::create(
                        messageBroker,
                        agentId,
                        hasDispatchConditions,
                        DEFAULT_PRE_DISPATCH_BUFFER_SIZE,
                        DEFAULT_AASB_METRICS_PUBLISH_SECONDS,
                        DEFAULT_AASB_MIN_METRICS_FOR_PUBLISH);
                    if (dispatcher == nullptr) {
                        AACE_ERROR(
                            LX(TAG, "Failed to create MessageDispatcherInterface for agent").d("agentId", agentId));
                        return false;
                    }
                    auto processor =
                        std::unique_ptr<MetricProcessor>(new MetricProcessor{std::move(config), std::move(dispatcher)});
                    m_metricProcessors.emplace(agentId, std::move(processor));
                }
            }
        }

        std::string buildType = DIMENSION_VALUE_BUILD_TYPE_RELEASE;
        if (config.contains(KEY_BUILD_TYPE)) {
            buildType = config.at(KEY_BUILD_TYPE);
        }

        // Set up dimensions that are common to each metric
        ThrowIfNot(
            populateCommonDimensions(deviceIdTag, buildType), "Failed to populate values for common metric dimensions");

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).m("Failed to parse configuration").d("reason", ex.what()));
        return false;
    }

    if (!registerServiceInterface<MetricRecorderServiceInterface>(shared_from_this())) {
        AACE_ERROR(LX(TAG, "Failed to register MetricsEngineService as MetricRecorderServiceInterface"));
        return false;
    }
    if (!registerServiceInterface<MetricsConfigServiceInterface>(shared_from_this())) {
        AACE_ERROR(LX(TAG, "Failed to register MetricsEngineService as MetricsConfigServiceInterface"));
        return false;
    }
    return true;
}

bool MetricsEngineService::configure() {
    AACE_ERROR(LX(TAG, "aace.metrics configuration is required"));
    return false;
}

bool MetricsEngineService::preRegister() {
    AACE_DEBUG(LX(TAG));
    auto emissionController =
        getContext()->getServiceInterface<aace::engine::metrics::MetricsEmissionInterface>("aace.alexa");

    std::lock_guard<std::mutex> lock(m_processorsMutex);
    for (const auto& processor : m_metricProcessors) {
        auto dispatcher = processor.second->dispatcher;
        if (dispatcher->hasPreDispatchRules()) {
            if (emissionController == nullptr) {
                AACE_WARN(
                    LX(TAG,
                       "Cannot register dispatcher as emission listener with null MetricsEmissionInterface. Enabling "
                       "emission directly"));
                dispatcher->onMetricEmissionStateChanged(true);
            } else {
                emissionController->addListener(dispatcher);
            }
        }
    }

    auto messageBrokerService =
        getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
            "aace.messageBroker");
    if (messageBrokerService == nullptr) {
        AACE_ERROR(LX(TAG, "MessageBrokerServiceInterface is null"));
        return false;
    }
    auto messageBroker = messageBrokerService->getMessageBroker();
    std::weak_ptr<MetricsEngineService> wp = shared_from_this();
    messageBroker->subscribe(
        "Metrics",
        "Submit",
        [wp](const aace::engine::messageBroker::Message& message) {
            auto sp = wp.lock();
            if (sp == nullptr) {
                AACE_ERROR(LX(TAG, "MetricsSubmitPublishHandler").m("Invalid weak pointer. Dropping message"));
                return;
            }
            sp->processInboundSubmitMessage(message);
        },
        aace::engine::messageBroker::Message::Direction::INCOMING);
    return true;
}

bool MetricsEngineService::stop() {
    AACE_DEBUG(LX(TAG));
    std::lock_guard<std::mutex> lock(m_processorsMutex);
    for (const auto& processor : m_metricProcessors) {
        processor.second->dispatcher->prepareForShutdown();
    }
    return true;
}

bool MetricsEngineService::shutdown() {
    AACE_DEBUG(LX(TAG));
    m_executor.waitForSubmittedTasks();
    std::lock_guard<std::mutex> lock(m_processorsMutex);
    for (const auto& processor : m_metricProcessors) {
        processor.second->dispatcher->shutdown();
    }
    m_metricProcessors.clear();
    return true;
}

/**
 * Check any preconditions and submit the metric for dispatch.
 * Calling thread must hold @c m_processorsMutex.
 * 
 * @return false if the dispatch fails due to the filter precondition
 */
bool dispatchIfAllowedLocked(const MetricEvent& metricEvent, const MetricProcessor& processor) {
    if (processor.uploadConfig == nullptr) {
        processor.dispatcher->submitMetric(metricEvent);
        return true;
    }
    if (!processor.uploadConfig->isAllowed(metricEvent)) {
        return false;
    }
    processor.dispatcher->submitMetric(metricEvent);
    return true;
}

void MetricsEngineService::recordMetric(const MetricEvent& metricEvent) {
    m_executor.submit([this, metricEvent] {
        std::lock_guard<std::mutex> lock(m_processorsMutex);
        auto context = metricEvent.getMetricContext();
        auto agentId = context.getAgentId();
        if (m_metricProcessors.empty()) {
            AACE_WARN(LX(TAG, "recordMetricExec")
                          .m("Metric service is not initialized; dropping metric")
                          .d("agentId", agentId));
            return;
        }

        if (agentId == aace::engine::utils::agent::AGENT_ID_ALL) {
            for (const auto& processor : m_metricProcessors) {
                dispatchIfAllowedLocked(metricEvent, *(processor.second));
            }
            return;
        }

        if (agentId == aace::engine::utils::agent::AGENT_ID_NONE) {
            // Metric is not associated with an agent or the agent is not available.
            // Submit the metric to the processor for every agent if allowed by the
            // agent's filter
            for (const auto& processor : m_metricProcessors) {
                dispatchIfAllowedLocked(metricEvent, *(processor.second));
            }
            return;
        }
        // Agent is known and tagged. Submit the metric to the processor of the
        // right agent
        auto processor = m_metricProcessors.find(agentId);
        if (processor == m_metricProcessors.end()) {
            AACE_WARN(LX(TAG).m("No metric processor for agent ID; dropping metric").d("agentId", agentId));
            return;
        }
        bool passedFilter = dispatchIfAllowedLocked(metricEvent, *(processor->second));
        if (!passedFilter) {
            AACE_WARN(LX(TAG, "recordMetricExec")
                          .m("Explicitly tagged metric not allowed by metrics filter")
                          .d("agentId", agentId)
                          .d("program", metricEvent.getProgramName())
                          .d("source", metricEvent.getSourceName()));
        }
    });
}

void MetricsEngineService::processInboundSubmitMessage(const aace::engine::messageBroker::Message& message) {
    m_executor.submit([this, message] {
        try {
            json payloadJson = nlohmann::json::parse(message.payload());
            ThrowIf(!payloadJson.contains("metrics"), "Missing metrics array");
            json metricsArray = payloadJson["metrics"];
            ThrowIf(!metricsArray.is_array(), "Metrics entry is not an array");
            for (const auto& item : metricsArray.items()) {
                auto& metricJson = item.value();
                std::string program = "AlexaAutoSDK";
                if (metricJson.contains("program")) {
                    program = metricJson["program"];
                }
                ThrowIfNot(metricJson.contains("source"), "Missing metric source");
                std::string source = metricJson["source"];
                ThrowIfNot(metricJson.contains("dataPoints"), "Missing dataPoints array");
                json datapointsJson = metricJson["dataPoints"];
                ThrowIfNot(datapointsJson.is_array(), "dataPoints is not an array");
                ThrowIf(datapointsJson.size() == 0, "dataPoints is empty");

                MetricEventBuilder builder;
                builder.withProgramName(program).withSourceName(source);

                for (const auto& datapointJson : datapointsJson) {
                    ThrowIfNot(datapointJson.contains("name"), "Data point missing name");
                    ThrowIfNot(datapointJson.contains("value"), "Data point missing value");
                    ThrowIfNot(datapointJson.contains("type"), "Data point missing type");
                    std::string name = datapointJson["name"];
                    ThrowIf(name.empty(), "Data point has empty name");
                    std::string type = datapointJson["type"];
                    ThrowIf(type.empty(), "Data point has empty type");
                    DataType dataType = dataTypeFromString(type);
                    std::string valueStr;
                    if (dataType == DataType::STRING) {
                        valueStr = datapointJson["value"];
                        ThrowIf(valueStr.empty(), "String data point has empty value");
                    } else if (dataType == DataType::COUNTER) {
                        uint32_t value = datapointJson["value"];
                        valueStr = std::to_string(value);
                        ThrowIf(valueStr.empty(), "Counter data point has empty value");
                    } else if (dataType == DataType::DURATION) {
                        uint64_t value = datapointJson["value"];
                        valueStr = std::to_string(value);
                        ThrowIf(valueStr.empty(), "Duration data point has empty value");
                    }
                    DataPoint dataPoint(name, valueStr, dataType);
                    builder.addDataPoint(dataPoint);
                }
                AgentIdType agentId = aace::engine::utils::agent::AGENT_ID_ALEXA;
                Priority priority = Priority::NORMAL;
                BufferType bufferType = BufferType::NO_BUFFER;
                IdentityType identityType = IdentityType::NORMAL;
                if (metricJson.contains("context")) {
                    json context = metricJson["context"];
                    if (context.contains("agentId")) {
                        agentId = context["agentId"];
                    }
                    if (context.contains("priority")) {
                        priority = priorityFromString(context["priority"]);
                    }
                    if (context.contains("identityType")) {
                        identityType = identityTypeFromString(context["identityType"]);
                    }
                    if (context.contains("bufferType")) {
                        bufferType = bufferTypeFromString(context["bufferType"]);
                    }
                }
                builder.withAgentId(agentId)
                    .withPriority(priority)
                    .withIdentityType(identityType)
                    .withBufferType(bufferType);
                recordMetric(builder.build());
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "MetricsSubmitPublishHandler")
                           .m("Failed to parse AASB message. Dropping its metrics")
                           .d("reason", ex.what()));
        }
    });
}

std::string MetricsEngineService::getMetricStoragePath() {
    return m_storagePath;
}

std::pair<std::string, std::string> MetricsEngineService::getStableUniqueAnonymousId() {
    return std::make_pair(CUSTOM_KEY_UNIQUE_ANON_DEVICE_ID, m_anonUniqueId);
}

bool MetricsEngineService::populateCommonDimensions(const std::string& deviceIdTag, const std::string& buildType) {
    try {
        auto vehicleService =
            getContext()->getServiceInterface<aace::engine::vehicle::VehicleConfigServiceInterface>("aace.vehicle");
        ThrowIfNull(vehicleService, "Null VehicleConfigServiceInterface");
        const std::string serialNumber =
            vehicleService->getDeviceInfoValue(aace::engine::vehicle::KEY_DEVICE_INFO_SERIAL_NUMBER);
        try {
            m_anonUniqueId = createAnonUniqueDeviceId(serialNumber, deviceIdTag);
            ThrowIf(m_anonUniqueId.empty(), "ID is empty");
        } catch (std::exception& ex) {
            throw("Failed to generate stable unique anonymous device identifier: " + std::string(ex.what()));
        }

        m_predefinedCommonDimensions = g_predefinedDimensionDefaults;
        m_customCommonDimensions = g_customDimensionDefaults;

        ThrowIf(
            buildType != DIMENSION_VALUE_BUILD_TYPE_RELEASE && buildType != DIMENSION_VALUE_BUILD_TYPE_DEBUG &&
                buildType != DIMENSION_VALUE_BUILD_TYPE_QA,
            "Invalid configured value for buildType");
        m_predefinedCommonDimensions[PREDEFINED_KEY_BUILD_TYPE] = buildType;

        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "PropertyManager is null");
        std::string sdkVersion = propertyManager->getProperty(aace::core::property::VERSION);
        if (sdkVersion.empty()) {
            AACE_WARN(LX(TAG).m("Auto SDK version property is empty"));
        } else {
            m_customCommonDimensions[CUSTOM_KEY_AUTO_SDK_VERSION] = sdkVersion;
        }

        std::string make = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_MAKE);
        ThrowIf(make.empty(), "Missing required config key: " + KEY_VEHICLE_INFO_MAKE);

        std::string model = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_MODEL);
        ThrowIf(model.empty(), "Missing required config key: " + KEY_VEHICLE_INFO_MODEL);

        std::string year = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_YEAR);
        ThrowIf(year.empty(), "Missing required config key: " + KEY_VEHICLE_INFO_YEAR);

        m_customCommonDimensions[CUSTOM_KEY_VEHICLE_MAKE] = make;
        m_customCommonDimensions[CUSTOM_KEY_VEHICLE_MODEL] = model;
        m_customCommonDimensions[CUSTOM_KEY_VEHICLE_YEAR] = year;

        std::string country = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_OPERATING_COUNTRY);
        if (country.empty()) {
            country = DIMENSION_VALUE_UNKNOWN;
            AACE_WARN(LX(TAG).m("Vehicle operating country config is empty"));
        }
        m_customCommonDimensions[CUSTOM_KEY_OPERATING_COUNTRY] = country;

        std::string platform = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_PLATFORM);
        if (platform.empty()) {
            platform = DIMENSION_VALUE_UNKNOWN;
            AACE_WARN(LX(TAG).m("Device platform config is empty"));
        }
        m_predefinedCommonDimensions[PREDEFINED_KEY_PLATFORM] = platform;

        std::string hardware = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_HARDWARE_ARCH);
        if (hardware.empty()) {
            hardware = DIMENSION_VALUE_UNKNOWN;
            AACE_WARN(LX(TAG).m("Device hardware config is empty"));
        }
        m_predefinedCommonDimensions[PREDEFINED_KEY_HARDWARE] = hardware;

        std::string deviceModel = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_MODEL);
        if (deviceModel.empty()) {
            deviceModel = DIMENSION_VALUE_UNKNOWN;
            AACE_WARN(LX(TAG).m("Device model config is empty"));
        }
        m_predefinedCommonDimensions[PREDEFINED_KEY_MODEL] = deviceModel;

        std::string appVersion = vehicleService->getAppInfoValue(KEY_APP_INFO_SOFTWARE_VERSION);
        if (appVersion.empty()) {
            appVersion = DIMENSION_VALUE_UNKNOWN;
            AACE_WARN(LX(TAG).m("App version config is empty"));
        }
        m_predefinedCommonDimensions[PREDEFINED_KEY_SOFTWARE_VERSION] = appVersion;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "populateCommonDimensions failed").d("reason", ex.what()));
        return false;
    }
}

DimensionsMap MetricsEngineService::getPredefinedCommonDimensions() {
    return m_predefinedCommonDimensions;
}

DimensionsMap MetricsEngineService::getCustomCommonDimensions() {
    return m_customCommonDimensions;
}

/**
 * Apply SHA256 hash to the specified input
 * 
 * @return The hashed input
 * @throw std::runtime_error in case of failure
 */
static std::string applyHash(const std::string& input) {
    SHA256_CTX context;
    if (!SHA256_Init(&context)) {
        throw std::runtime_error("SHA256_Init failed");
    }
    unsigned char digest[SHA256_DIGEST_LENGTH];
    if (!SHA256_Update(&context, (unsigned char*)input.c_str(), input.length())) {
        throw std::runtime_error("SHA256_Update failed");
    }
    if (!SHA256_Final(digest, &context)) {
        throw std::runtime_error("SHA256_Final failed");
    }
    std::ostringstream digestString;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        digestString << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    return digestString.str();
}

std::string MetricsEngineService::createAnonUniqueDeviceId(const std::string& deviceId, const std::string& salt) {
    auto intermediateHash = applyHash(deviceId);
    return applyHash(intermediateHash + salt);
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
