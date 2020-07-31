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

#include <AACE/JNI/Metrics/MetricsUploaderBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.metrics.MetricsUploaderBinder";

namespace aace {
namespace jni {
namespace metrics {

//
// MetricsUploaderBinder
//

MetricsUploaderBinder::MetricsUploaderBinder(jobject obj) {
    m_metricsUploaderHandler = std::shared_ptr<MetricsUploaderHandler>(new MetricsUploaderHandler(obj));
}

//
// MetricsUploaderHandler
//

MetricsUploaderHandler::MetricsUploaderHandler(jobject obj) :
        m_obj(obj, "com/amazon/metricuploadservice/MetricsUploader") {
}

bool MetricsUploaderHandler::record(
    const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints,
    const std::unordered_map<std::string, std::string>& metadata) {
    try_with_context {
        auto dataPointClass =
            aace::jni::native::JavaClass::find("com/amazon/metricuploadservice/MetricsUploader$Datapoint");
        ThrowIfNull(dataPointClass, "findClassFailed");

        JObjectArray arr(datapoints.size(), dataPointClass);
        ThrowIfNot(arr.isValid(), "invalidObjectArray");

        for (int j = 0; j < datapoints.size(); j++) {
            jobject checkedDataPointTypeObj;
            ThrowIfNot(
                JDatapointType::checkType(datapoints[j].getType(), &checkedDataPointTypeObj), "invalidDataPointType");

            jobject dataPointObj = dataPointClass->newInstance(
                "(Lcom/amazon/metricuploadservice/MetricsUploader$Datapoint;I)V",
                checkedDataPointTypeObj,
                JString(datapoints[j].getName()).get(),
                JString(datapoints[j].getValue()).get(),
                datapoints[j].getCount());
            ThrowIfNull(dataPointObj, "createDataPointFailed");

            ThrowIfNot(arr.setAt(j, dataPointObj), "insertArrayElementFailed");
        }

        JObject map("java/util/HashMap");

        for (auto itr : metadata) {
            ThrowIfNot(
                map.invoke<jobject>(
                    "put",
                    "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
                    nullptr,
                    JString(itr.first).get(),
                    JString(itr.second).get()),
                "invokeMethodFailed");
        }

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "record",
                "([Lcom/amazon/metricuploadservice/MetricsUploader$Datapoint;Ljava/util/HashMap;)Z",
                &result,
                arr.get(),
                map.get()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "record", ex.what());
        return false;
    }
}

}  // namespace metrics
}  // namespace jni
}  // namespace aace

#define METRICS_UPLOADER_BINDER(ref) reinterpret_cast<aace::jni::metrics::MetricsUploaderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL
Java_com_amazon_aace_metricuploadservice_MetricsUploader_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::metrics::MetricsUploaderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_metricuploadservice_MetricsUploader_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto metricsUploaderBinder = METRICS_UPLOADER_BINDER(ref);
        ThrowIfNull(metricsUploaderBinder, "invalidMetricsUploaderBinder");
        delete metricsUploaderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_metrics_MetricsUploader_disposeBinder", ex.what());
    }
}
}