/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace vehicle {

// String to identify log entries originating from this file.
static const std::string TAG("aace.vehicle.VehicleEngineService");

// register the service
REGISTER_SERVICE(VehicleEngineService)

VehicleEngineService::VehicleEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool VehicleEngineService::configure( const std::vector<std::shared_ptr<std::istream>>& configuration )
{
    // attempt to configure each stream
    for( auto next : configuration ) {
        ReturnIf( configure( next ), true );
    }
    
    // warn if the vehicle property map has not been configured
    if( m_vehiclePropertyMap.empty() ) {
        AACE_WARN(LX(TAG,"configure").m("vehicleInfoNotConfigured"));
    }
    
    return true;
}

bool VehicleEngineService::checkProperty( rapidjson::Value& root, const char* key, bool warnIfMissing )
{
    ReturnIf( root.HasMember( key ) && root[key].IsString(), true );
    
    if( warnIfMissing ) {
        AACE_WARN(LX(TAG,"checkProperty").d("reason","missingVehicleProperty").d("property",key));
    }
    
    return false;
}

std::string VehicleEngineService::getProperty( rapidjson::Value& root, const char* key, const char* defaultValue, bool warnIfMissing )
{
    ReturnIfNot( checkProperty( root, key, warnIfMissing ), defaultValue );

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
            
            m_vehiclePropertyMap["make"] = getProperty( info, "make" );
            m_vehiclePropertyMap["model"] = getProperty( info, "model" );
            m_vehiclePropertyMap["year"] = getProperty( info, "year" );
            m_vehiclePropertyMap["trim"] = getProperty( info, "trim" );
            m_vehiclePropertyMap["geography"] = getProperty( info, "geography" );
            m_vehiclePropertyMap["version"] = getProperty( info, "version" );
            m_vehiclePropertyMap["os"] = getProperty( info, "os" );
            m_vehiclePropertyMap["arch"] = getProperty( info, "arch" );
            m_vehiclePropertyMap["language"] = getProperty( info, "language" );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::vehicle
} // aace::engine
} // aace

