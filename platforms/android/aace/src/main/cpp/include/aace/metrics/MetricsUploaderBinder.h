/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_METRICS_METRICS_UPLOADER_BINDER_H
#define AACE_METRICS_METRICS_UPLOADER_BINDER_H

#include "AACE/Metrics/MetricsUploader.h"
#include "aace/core/PlatformInterfaceBinder.h"

class MetricsUploaderBinder : public PlatformInterfaceBinder, public aace::metrics::MetricsUploader {
public:
    MetricsUploaderBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool record( const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints, const std::unordered_map<std::string, std::string>& metadata ) override;

private:
    jobject convert( aace::metrics::MetricsUploader::DatapointType type );

private:
    jmethodID m_javaMethod_record = nullptr;

    // Datapoint
    ClassRef m_javaClass_Datapoint;
    jfieldID m_javaField_Datapoint_name;
    jfieldID m_javaField_Datapoint_value;
    jfieldID m_javaField_Datapoint_count;
    jfieldID m_javaField_Datapoint_type;

    // DatapointType
    ObjectRef m_enum_DatapointType_TIMER;
    ObjectRef m_enum_DatapointType_STRING;
    ObjectRef m_enum_DatapointType_COUNTER;
};

#endif //AACE_METRICS_METRICS_UPLOADER_BINDER_H
