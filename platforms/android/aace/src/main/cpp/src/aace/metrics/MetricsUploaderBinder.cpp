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

#include "aace/metrics/MetricsUploaderBinder.h"

void MetricsUploaderBinder::initialize( JNIEnv* env )
{
    //Record method
    m_javaMethod_record = env->GetMethodID( getJavaClass(), "record", "([Lcom/amazon/metricuploadservice/MetricsUploader$Datapoint;Ljava/util/HashMap;)Z");

    //Datapoint
    m_javaClass_Datapoint = NativeLib::FindClass( env, "com/amazon/metricuploadservice/MetricsUploader$Datapoint" );
    m_javaField_Datapoint_name = env->GetFieldID( m_javaClass_Datapoint.get(), "name", "Ljava/lang/String;" );
    m_javaField_Datapoint_value = env->GetFieldID( m_javaClass_Datapoint.get(), "value", "Ljava/lang/String;" );
    m_javaField_Datapoint_count = env->GetFieldID( m_javaClass_Datapoint.get(), "count", "I" );
    m_javaField_Datapoint_type = env->GetFieldID( m_javaClass_Datapoint.get(), "type", "Lcom/amazon/metricuploadservice/MetricsUploader$DatapointType;" );

    //DatapointType
    jclass datapointTypeEnumClass = env->FindClass( "com/amazon/metricuploadservice/MetricsUploader$DatapointType" );
    m_enum_DatapointType_TIMER = NativeLib::FindEnum( env, datapointTypeEnumClass, "TIMER", "Lcom/amazon/metricuploadservice/MetricsUploader$DatapointType;" );
    m_enum_DatapointType_STRING = NativeLib::FindEnum( env, datapointTypeEnumClass, "STRING", "Lcom/amazon/metricuploadservice/MetricsUploader$DatapointType;" );
    m_enum_DatapointType_COUNTER = NativeLib::FindEnum( env, datapointTypeEnumClass, "COUNTER", "Lcom/amazon/metricuploadservice/MetricsUploader$DatapointType;" );
}

bool MetricsUploaderBinder::record( const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints, const std::unordered_map<std::string, std::string>& metadata ) 
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_record != nullptr ) {
        ThreadContext context;

        if( context.isValid() )
        {
            //Convert datapoints vector to datapoints[]
            jobjectArray datapointsArr = context.getEnv()->NewObjectArray( datapoints.size(), m_javaClass_Datapoint.get(), NULL );
            for ( int i = 0; i < datapoints.size(); i++ ) {
                //Create jobject datapoint
                jobject datapoint = context.getEnv()->AllocObject(m_javaClass_Datapoint.get());
                jstring name = NativeLib::convert(context.getEnv(), datapoints[i].getName());
                jstring value = NativeLib::convert(context.getEnv(), datapoints[i].getValue());
                jobject type = convert(datapoints[i].getType());

                context.getEnv()->SetObjectField( datapoint, m_javaField_Datapoint_name, name);
                context.getEnv()->SetObjectField( datapoint, m_javaField_Datapoint_value, value);
                context.getEnv()->SetIntField( datapoint, m_javaField_Datapoint_count, (jint) datapoints[i].getCount());
                context.getEnv()->SetObjectField( datapoint, m_javaField_Datapoint_type, type);

                //Add jobject datapoint to array
                context.getEnv()->SetObjectArrayElement( datapointsArr, i, datapoint );

                //Cleanup references
                context.getEnv()->DeleteLocalRef(datapoint);
                context.getEnv()->DeleteLocalRef(name);
                context.getEnv()->DeleteLocalRef(value);
            }

            //Convert C++ map to Java HashMap by transferring values
            jclass hashMapClass= context.getEnv()->FindClass("java/util/HashMap");
            jmethodID hashMapInit = context.getEnv()->GetMethodID(hashMapClass, "<init>", "(I)V");
            jobject hashMapObj = context.getEnv()->NewObject(hashMapClass, hashMapInit, metadata.size());
            jmethodID hashMapPutId = context.getEnv()->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

            for (auto itr : metadata) {
                jstring key = NativeLib::convert(context.getEnv(), itr.first);
                jstring value = NativeLib::convert(context.getEnv(), itr.second);
                context.getEnv()->CallObjectMethod(hashMapObj, hashMapPutId, key, value);
                context.getEnv()->DeleteLocalRef(key);
                context.getEnv()->DeleteLocalRef(value);
            }

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_record, datapointsArr, hashMapObj );
            context.getEnv()->DeleteLocalRef(hashMapObj);
            context.getEnv()->DeleteLocalRef(hashMapClass);
            context.getEnv()->DeleteLocalRef(datapointsArr);
        }
    }
    return result;
}

jobject MetricsUploaderBinder::convert( aace::metrics::MetricsUploader::DatapointType type ) 
{
    switch( type )
    {
        case aace::metrics::MetricsUploader::DatapointType::TIMER:
            return m_enum_DatapointType_TIMER.get();
        case aace::metrics::MetricsUploader::DatapointType::STRING:
            return m_enum_DatapointType_STRING.get();
        case aace::metrics::MetricsUploader::DatapointType::COUNTER:
            return m_enum_DatapointType_COUNTER.get();
    }
}