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

#include "AACE/Engine/Vehicle/VehicleEngineService.h"
#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"
#include "AACE/Engine/Storage/LocalStorageInterface.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include "AACE/Vehicle/VehicleProperties.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace vehicle {

// String to identify log entries originating from this file.
static const std::string TAG("aace.vehicle.VehicleEngineService");

// name of the table used for the local storage database
static const std::string VEHICLE_SERVICE_LOCAL_STORAGE_TABLE = "aace.vehicle";

// register the service
REGISTER_SERVICE(VehicleEngineService)

VehicleEngineService::VehicleEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

std::string VehicleEngineService::getVehicleProperty( VehiclePropertyType type ) {
    auto it = m_vehiclePropertyMap.find( type );
    return it != m_vehiclePropertyMap.end() ? it->second : "";
}

std::string VehicleEngineService::getVehiclePropertyAttribute( VehiclePropertyType property ) {
    switch( property )
    {
        case VehiclePropertyType::MAKE: return "Make";
        case VehiclePropertyType::MODEL: return "Model";
        case VehiclePropertyType::YEAR: return "Year";
        case VehiclePropertyType::TRIM: return "Trim";
        case VehiclePropertyType::GEOGRAPHY: return "Geography";
        case VehiclePropertyType::VERSION: return "SWVersion";
        case VehiclePropertyType::OPERATING_SYSTEM: return "OS";
        case VehiclePropertyType::HARDWARE_ARCH: return "HW";
        case VehiclePropertyType::LANGUAGE: return "Language";
        case VehiclePropertyType::MICROPHONE: return "Microphone";
        case VehiclePropertyType::COUNTRY_LIST: return "Countries";
        case VehiclePropertyType::VEHICLE_IDENTIFIER: return "VehicleIdentifier";
    }
    return "";
}

bool VehicleEngineService::initialize() {
    ThrowIfNot( registerServiceInterface<VehiclePropertyInterface>( shared_from_this() ), "registerVehiclePropertyInterfaceFailed" );
    return true;
}

bool VehicleEngineService::configure( const std::vector<std::shared_ptr<std::istream>>& configuration )
{
    // attempt to configure each stream
    for( auto next : configuration ) {
        configure( next );
    }
    
    // warn if the vehicle property map has not been configured
    if( m_vehiclePropertyMap.empty() ) {
        AACE_WARN(LX(TAG,"configure").m("vehicleInfoNotConfigured"));
    } else {
        m_vehiclePropertiesMetric = generateVehiclePropertiesMetric();
    }
    
    // get the operating country from the settings
    auto localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>( "aace.storage" );
    
    if( localStorage != nullptr && localStorage->containsKey( VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry" ) ) {
        m_operatingCountry = localStorage->get( VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry", m_operatingCountry );
    }
    
    return true;
}

bool VehicleEngineService::setup()
{
    try
    {
        if ( m_vehiclePropertiesMetric != nullptr ) {
            m_vehiclePropertiesMetric->record();
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setup").d("reason", ex.what()));
        return false;
    }

    return true;
}

bool VehicleEngineService::checkVehicleConfigProperty( rapidjson::Value& root, const char* key, bool warnIfMissing )
{
    ReturnIf( root.HasMember( key ) && root[key].IsString(), true );
    
    if( warnIfMissing ) {
        AACE_WARN(LX(TAG,"checkProperty").d("reason","missingVehicleProperty").d("property",key));
    }
    
    return false;
}

std::string VehicleEngineService::getVehicleConfigProperty( rapidjson::Value& root, const char* key, const char* defaultValue, bool warnIfMissing )
{
    ReturnIfNot( checkVehicleConfigProperty( root, key, warnIfMissing ), defaultValue );

    std::string value = root[key].GetString();
    
    // log the vehicle property
    AACE_INFO(LX(TAG,"VehicleProperty").d(key,value));
    
    return value;
}

bool VehicleEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    try
    {
        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;
        
        document.ParseStream( isw );
        
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidConfigurationStream" );
        
        auto root = document.GetObject();
        
        ReturnIfNot( root.HasMember( "aace.vehicle" ) && root["aace.vehicle"].IsObject(), false );
        
        auto vehicleConfigRoot = root["aace.vehicle"].GetObject();

        if( vehicleConfigRoot.HasMember( "info" ) && vehicleConfigRoot["info"].IsObject() )
        {
            rapidjson::Value info = vehicleConfigRoot["info"].GetObject();
            
            m_vehiclePropertyMap[VehiclePropertyType::MAKE] = getVehicleConfigProperty( info, "make" );
            m_vehiclePropertyMap[VehiclePropertyType::MODEL] = getVehicleConfigProperty( info, "model" );
            m_vehiclePropertyMap[VehiclePropertyType::YEAR] = getVehicleConfigProperty( info, "year" );
            m_vehiclePropertyMap[VehiclePropertyType::TRIM] = getVehicleConfigProperty( info, "trim" );
            m_vehiclePropertyMap[VehiclePropertyType::GEOGRAPHY] = getVehicleConfigProperty( info, "geography" );
            m_vehiclePropertyMap[VehiclePropertyType::VERSION] = getVehicleConfigProperty( info, "version" );
            m_vehiclePropertyMap[VehiclePropertyType::OPERATING_SYSTEM] = getVehicleConfigProperty( info, "os" );
            m_vehiclePropertyMap[VehiclePropertyType::HARDWARE_ARCH] = getVehicleConfigProperty( info, "arch" );
            m_vehiclePropertyMap[VehiclePropertyType::LANGUAGE] = getVehicleConfigProperty( info, "language" );
            m_vehiclePropertyMap[VehiclePropertyType::MICROPHONE] = getVehicleConfigProperty(info, "microphone" );
            m_vehiclePropertyMap[VehiclePropertyType::COUNTRY_LIST] = getVehicleConfigProperty(info, "countries" );
            m_vehiclePropertyMap[VehiclePropertyType::VEHICLE_IDENTIFIER] = getVehicleConfigProperty(info, "vehicleIdentifier");
        }
        
        if( vehicleConfigRoot.HasMember( "operatingCountry" ) && vehicleConfigRoot["operatingCountry"].IsString() ) {
            m_operatingCountry = vehicleConfigRoot["operatingCountry"].GetString();
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

bool VehicleEngineService::setProperty( const std::string& key, const std::string& value )
{
    try
    {
        if( key.compare( aace::vehicle::property::OPERATING_COUNTRY ) == 0 )
        {
            auto localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>( "aace.storage" );

            ThrowIfNull( localStorage, "localStorageInterfaceInvalid" );
            ThrowIfNot( localStorage->put( VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry", value ), "setLocalStorageFailed" );
            
            m_operatingCountry = value;
        }
        else {
            return false;
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setProperty").d("reason", ex.what()).d("key",key).d("value",value));
        return false;
    }
}

std::string VehicleEngineService::getProperty( const std::string& key )
{
    try
    {    
        if( key.compare( aace::vehicle::property::OPERATING_COUNTRY ) == 0 ) {
            return m_operatingCountry;
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getProperty").d("reason", ex.what()).d("key",key));
    }
    
    return std::string();
}

std::shared_ptr<aace::engine::metrics::MetricEvent> VehicleEngineService::generateVehiclePropertiesMetric() {
    std::string program = "AlexaAuto_Vehicle";
    std::string source = "VehicleConfiguration";
    std::shared_ptr<aace::engine::metrics::MetricEvent> currentMetric = 
        std::shared_ptr<aace::engine::metrics::MetricEvent>(new aace::engine::metrics::MetricEvent(program, source));

    for( auto itr : m_vehiclePropertyMap ) {
        VehiclePropertyType property = itr.first;
        std::string dataPointName = getVehiclePropertyAttribute(property);
        std::string dataPointValue = itr.second;

        // sanitize any delimiter characters from dataPointValue to maintain metric formatting
        char delimiters[] = ";=,:";
        for (char delimiter : delimiters)
        {
            std::replace( dataPointValue.begin(), dataPointValue.end(), delimiter, '-');
        }

        currentMetric->addString(dataPointName, dataPointValue);
    }
    return currentMetric;
}

} // aace::engine::vehicle
} // aace::engine
} // aace

