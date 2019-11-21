/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Alexa/WakewordEngineManager.h"
#include "AACE/Engine/AmazonLite/AmazonLiteEngineService.h"

namespace aace {
namespace engine {
namespace amazonLite {

// String to identify log entries originating from this file.
static const std::string TAG("aace.amazonLite.AmazonLiteEngineService");

static const std::string FACTORY_NAME("aace.amazonLite");

// register the service
REGISTER_SERVICE( AmazonLiteEngineService )

AmazonLiteEngineService::AmazonLiteEngineService(
    const aace::engine::core::ServiceDescription &description ) : 
    aace::engine::core::EngineService( description ) {
}

bool AmazonLiteEngineService::initialize() {
    AACE_INFO( LX( TAG, "initializing" ) );
    try {
        // Get the Alexa Engine Service Handler
        auto alexaEngineService = getContext()->getService<aace::engine::alexa::AlexaEngineService>();
        ThrowIfNull( alexaEngineService, "alexaEngineServiceIsNull" );

        auto manager = alexaEngineService->getServiceInterface<alexa::WakewordEngineManager>();
        ThrowIfNull( manager, "WakewordEngineManager has not been registered" );

        auto amazonLiteWakewordEngineAdapterFactory = [this](const alexa::WakewordEngineManager::AdapterType &type) {
            auto defaultLocale = getContext()->getProperty( aace::alexa::property::LOCALE );
            if (type == alexa::WakewordEngineManager::AdapterType::PRIMARY) {
                if (!m_primaryAdapter) {
                    m_primaryAdapter = aace::engine::amazonLite::AmazonLiteWakewordEngineAdapter::create( defaultLocale, m_rootPath, m_modelLocaleMap );
                    ThrowIfNull( m_primaryAdapter, "primaryAdapterIsNull" );
                }
                return m_primaryAdapter;
            }
            if (type == alexa::WakewordEngineManager::AdapterType::SECONDARY) {
                if (!m_secondaryAdapter) {
                    m_secondaryAdapter = aace::engine::amazonLite::AmazonLiteWakewordEngineAdapter::create( defaultLocale, m_rootPath, m_modelLocaleMap );
                    ThrowIfNull( m_secondaryAdapter, "secondaryAdapterIsNull" );
                }
                return m_secondaryAdapter;
            }
            return std::shared_ptr<aace::engine::amazonLite::AmazonLiteWakewordEngineAdapter>(nullptr);
        };

        // Register the factory adapter interface
        ThrowIfNot( manager->registerFactory( FACTORY_NAME, amazonLiteWakewordEngineAdapterFactory ), "Unable to register wakeword detector adapter" );

        return true;
    } catch (std::exception &ex) {
        AACE_ERROR( LX( TAG, "initialize" ).d( "reason", ex.what() ) );
        return false;
    }
}

bool AmazonLiteEngineService::configure( std::shared_ptr<std::istream> configuration ) 
{
    try 
    {
        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;

        document.ParseStream( isw );

        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidConfigurationStream" );

        auto amazonLiteConfigRoot = document.GetObject();

        if( amazonLiteConfigRoot.HasMember( "rootPath" ) && amazonLiteConfigRoot["rootPath"].IsString() ) {
            m_rootPath = amazonLiteConfigRoot["rootPath"].GetString();
        } else {
            Throw( "mandatoryConfigElementRootPathNotPresent" );
        }

        // Populate the models. 
        if( amazonLiteConfigRoot.HasMember( "models" ) && amazonLiteConfigRoot["models"].IsArray() ) {
            for( rapidjson::Value::ConstValueIterator itr = amazonLiteConfigRoot["models"].Begin(); itr != amazonLiteConfigRoot["models"].End(); itr++ ) {
                if( (*itr).HasMember( "locale" ) && (*itr)[ "locale" ].IsString() ) {
                    if( (*itr).HasMember( "path" ) && (*itr)[ "path" ].IsString() ) {
                        m_modelLocaleMap[(*itr)[ "locale" ].GetString()] = (*itr)[ "path" ].GetString();
                    } else {
                        Throw( "modelsConfigItemPathInvalid" );
                    }
                } else {
                    Throw( "modelsConfigItemLocaleInvalid" );
                }
            }
        } else {
            Throw( "mandatoryConfigItemModelsNotPresent" );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN( LX( TAG, "configure" ).d( "reason", ex.what() ) );
        configuration->clear();
        configuration->seekg( 0 );
        return false;
    }
}

bool AmazonLiteEngineService::setup() {
    return true;
}

bool AmazonLiteEngineService::start() {
    AACE_INFO( LX( TAG, "start" ) );
    return true;
}

bool AmazonLiteEngineService::stop() {
    return true;
}

bool AmazonLiteEngineService::shutdown() {
    return true;
}

bool AmazonLiteEngineService::setProperty( const std::string& key, const std::string &value ) {
    try {
        if( key.compare( aace::engine::wakeword::property::WAKEWORD_ENGINE_LOCALE ) == 0 ) {
            if ( m_primaryAdapter ) m_primaryAdapter->setLocale( value );
            if ( m_secondaryAdapter ) m_secondaryAdapter->setLocale( value );
        }

        AACE_INFO( LX( TAG, "setProperty" ).d( "key", key ).d( "value", value ) );

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "setProperty" ).d( "reason", ex.what() ).d( "key", key ).d( "value", value ) );
        return false;
    }
}

std::string AmazonLiteEngineService::getProperty(const std::string &key) {
    return std::string();
}

} // aace::engine::amazonLite
} // aace::engine
} // aace
