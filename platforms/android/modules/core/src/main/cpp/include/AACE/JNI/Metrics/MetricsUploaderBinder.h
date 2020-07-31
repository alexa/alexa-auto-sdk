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

#ifndef AACE_JNI_METRICS_METRICS_UPLOADER_BINDER_H
#define AACE_JNI_METRICS_METRICS_UPLOADER_BINDER_H

#include <AACE/Metrics/MetricsUploader.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace metrics {

//
// MetricsUploaderHandler
//

class MetricsUploaderHandler : public aace::metrics::MetricsUploader {
public:
    MetricsUploaderHandler(jobject obj);

    // aace::metrics::MetricsUploader
    bool record(const std::vector<Datapoint>& datapoints, const std::unordered_map<std::string, std::string>& metadata)
        override;

private:
    JObject m_obj;
};

//
// MetricsUploaderBinder
//

class MetricsUploaderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    MetricsUploaderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_metricsUploaderHandler;
    }

private:
    std::shared_ptr<MetricsUploaderHandler> m_metricsUploaderHandler;
};

//
// JDatapointType
//

class JDatapointTypeConfig : public EnumConfiguration<MetricsUploaderHandler::DatapointType> {
public:
    using T = MetricsUploaderHandler::DatapointType;

    const char* getClassName() override {
        return "com/amazon/metricuploadservice/MetricsUploader$DatapointType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::TIMER, "TIMER"}, {T::STRING, "STRING"}, {T::COUNTER, "COUNTER"}};
    }
};

using JDatapointType = JEnum<MetricsUploaderHandler::DatapointType, JDatapointTypeConfig>;

}  // namespace metrics
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_METRICS_METRICS_UPLOADER_BINDER_H