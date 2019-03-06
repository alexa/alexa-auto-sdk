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

#include "AACE/Engine/Metrics/MetricsUploaderEngineImpl.h"
#include "AACE/Engine/Logger/EngineLogger.h"
#include "AACE/Engine/Core/EngineMacros.h"

// String to identify log entries originating from this file.
static const std::string TAG("aace.metrics.MetricsUploaderEngineImpl");

namespace aace {
namespace engine {
namespace metrics {

static const std::string METRIC_RECORD_KEYWORD = "MetricEvent";
static const std::string PRIORITY_KEY = "Priority";
static const std::string PROGRAM_KEY = "Program";
static const std::string SOURCE_KEY = "Source";
static const std::string TIMER_KEY = "TI";
static const std::string STRING_KEY = "DV";
static const std::string COUNTER_KEY = "CT";

static const std::regex metricRegex("^([^:]+):([^:]+):([^:]+):(.+):(NR|HI)");
static const std::regex dataRegex("([^;=,:]+)=([^;=,:]+);([^;=,:]+);([0-9]+),");

MetricsUploaderEngineImpl::MetricsUploaderEngineImpl( std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface ) :
    aace::engine::logger::sink::Sink( TAG ),
    m_platformMetricsUploaderInterface( platformMetricsUploaderInterface ) {
}

std::shared_ptr<MetricsUploaderEngineImpl> MetricsUploaderEngineImpl::create( std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface, std::shared_ptr<aace::engine::logger::LoggerServiceInterface> loggerService )
{
    try
    {
        ThrowIfNull( platformMetricsUploaderInterface, "invalidMetricsUploaderPlatformInterface" );
        ThrowIfNull( loggerService, "invalidLoggerService" );

        std::shared_ptr<MetricsUploaderEngineImpl> metricsUploaderEngineImpl = std::shared_ptr<MetricsUploaderEngineImpl>( new MetricsUploaderEngineImpl( platformMetricsUploaderInterface ) );
        
        ThrowIfNot( metricsUploaderEngineImpl->initialize(), "inializeMetricsUploaderEngineImplFailed" );
        
        // add the uploader service impl to the logger service
        loggerService->addSink( metricsUploaderEngineImpl );
        
        return metricsUploaderEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

bool MetricsUploaderEngineImpl::initialize()
{
    try
    {
        ThrowIfNot( addRule( Level::METRIC, aace::engine::logger::sink::Rule::EMPTY, aace::engine::logger::sink::Rule::EMPTY, aace::engine::logger::sink::Rule::EMPTY ), "addRuleFailed" );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

// aace::engine::logger::sink::Sink
void MetricsUploaderEngineImpl::log( Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text )
{
    try {
        //If log is not a Metric log, then return
        if( level != Level::METRIC ) {
            return;
        }
        std::string logMessage = std::string (text);
        logMessage.erase(std::remove(logMessage.begin(), logMessage.end(), '\\'), logMessage.end());

        //If Metric log is not Recording related, then return
        if (logMessage.find(METRIC_RECORD_KEYWORD) == std::string::npos) {
            return;
        }

        //Parse program, source, datapoints to record metric
        std::smatch metricMatch;
        if ( std::regex_match(logMessage, metricMatch, metricRegex) ) {
            //Handle regex groups
            std::string programName = metricMatch[2].str();
            std::string sourceName = metricMatch[3].str();
            std::string datapoints = metricMatch[4].str();
            std::string priority = metricMatch[5].str();
            
            //Validate values are not empty/null
            if( programName.empty() || sourceName.empty() || datapoints.empty() || priority.empty() ) {
                return;
            }

            //Create metadata map
            std::unordered_map<std::string, std::string> metadata;
            metadata[PROGRAM_KEY] = programName;
            metadata[SOURCE_KEY] = sourceName;
            metadata[PRIORITY_KEY] = priority;

            //Parse each datapoint and add to vector to pass to platform implementation
            std::vector<aace::metrics::MetricsUploader::Datapoint> datapointList;
            std::smatch data_match;
            while ( std::regex_search(datapoints, data_match, dataRegex) ) {
                //Handle regex groups
                std::string name = data_match[1].str();
                std::string value = data_match[2].str();
                std::string typeStr = data_match[3].str();
                std::string countStr = data_match[4].str();

                //Define dataType
                aace::metrics::MetricsUploader::DatapointType dataType;
                if ( typeStr == TIMER_KEY ) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::TIMER;
                } else if ( typeStr == STRING_KEY ) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::STRING;
                } else if ( typeStr == COUNTER_KEY ) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::COUNTER;
                } else {
                    //No valid datatype was found
                    return;
                }
                
                //Create and add datapoint to list
                aace::metrics::MetricsUploader::Datapoint curData = aace::metrics::MetricsUploader::Datapoint(dataType, name, value, stoi(countStr));
                datapointList.push_back(curData);

                //Set datapoints string equal to next datapoint for parsing until all datapoints parsed
                datapoints = data_match.suffix();
            }
            m_platformMetricsUploaderInterface->record(datapointList, metadata);
        }
    } catch( std::exception& ex ) {
        //Exception occurred
    }
}

} // aace::engine::metrics
} // aace::engine
} // aace
