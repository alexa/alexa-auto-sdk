/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/AmazonLite/AmazonLiteWakewordEngineAdapter.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace amazonLite {

// String to identify log entries originating from this file.
static const std::string TAG("aace.amazonLite.AmazonLiteWakewordEngineAdapter");

static const std::string DEFAULT("default");

AmazonLiteWakewordEngineAdapter::AmazonLiteWakewordEngineAdapter( const std::string& avsLocale, const std::string& rootPath, const std::map<std::string,std::string>& modelLocaleMap ) :
    m_initialized( false ),
    m_avsLocale( avsLocale ),
    m_rootPath( rootPath ),
    m_modelLocaleMap( modelLocaleMap ) {
}

std::shared_ptr<AmazonLiteWakewordEngineAdapter> AmazonLiteWakewordEngineAdapter::create( const std::string& avsLocale, const std::string& rootPath, const std::map<std::string,std::string>& modelLocaleMap) {
    try {
        auto amazonLiteWakewordEngineAdapter = std::shared_ptr<AmazonLiteWakewordEngineAdapter>( new AmazonLiteWakewordEngineAdapter( avsLocale, rootPath, modelLocaleMap ) );
        ThrowIfNull( amazonLiteWakewordEngineAdapter, "amazonLiteWakewordEngineAdapterIsNull" );

        return amazonLiteWakewordEngineAdapter;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "create").d( "reason", ex.what() ) );
        return nullptr;
    }
}

bool AmazonLiteWakewordEngineAdapter::initialize( std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream>& audioInputstream, alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat ) {
    try {
        ThrowIfNull( audioInputstream, "invalidAudioInputStream");

        m_audioInputStream = audioInputstream;
        m_audioFormat = audioFormat;

        m_initialized = true;

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "initialize" ).d( "reason", ex.what() ) );
        return false;
    }
}

bool AmazonLiteWakewordEngineAdapter::enable() {
    AACE_VERBOSE(LX(TAG,"enable"));
    try {
        ThrowIfNot( m_initialized, "notInitialized" );

        std::string pathToLocale = "";
        m_selectedLocale = DEFAULT;
        if( m_modelLocaleMap.size() == 0 ) {
            // Use the default locale
            AACE_INFO( LX( TAG, "enable" ).m( "usingDefaultWakewordModel" ) );
        } else if( m_avsLocale.empty() ) {
            // Use the default locale
            AACE_ERROR( LX( TAG, "enable" ).m( "avsLocaleIsEmptyFallingBackToDefaultLocale" ) );
        } else if( !checkIfLocaleExistsInMap( m_avsLocale ) ) {
            // Use the default locale
            AACE_WARN( LX( TAG, "enable" ).m( "localeFileNotFoundFallingBackToDefaultLocale" ) );
        } else {
            pathToLocale = getModelPathFromLocale( m_avsLocale );
            m_selectedLocale = m_avsLocale;
        }

        if( m_selectedLocale == DEFAULT ) {
            m_wakewordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create( m_audioInputStream, m_audioFormat, {}, {} );
        } else {
            m_wakewordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create( m_audioInputStream, m_audioFormat, {}, {}, pathToLocale );
        }
        ThrowIfNull( m_wakewordDetector, "createPryonLiteKeywordDetectorFailed" );

        // add the keyword observer to the wakeword detector
        m_wakewordDetector->addKeyWordObserver( shared_from_this() );

        return true; 
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "enable" ).d( "reason", ex.what() ) );
        if( m_wakewordDetector != nullptr ) {
            m_wakewordDetector.reset();
        }
        m_selectedLocale = DEFAULT;
        return false;
    }
}

bool AmazonLiteWakewordEngineAdapter::disable() {
    AACE_VERBOSE(LX(TAG,"disable"));
    try {
        ThrowIfNot( m_initialized, "notInitialized" );

        // release the wakeword detector reference
        m_wakewordDetector.reset();

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "disable" ).d( "reason", ex.what() ) );
        return false;
    }
}

void AmazonLiteWakewordEngineAdapter::addKeyWordObserver(std::shared_ptr<KeyWordObserverInterface> keyWordObserver) {
    std::lock_guard<std::mutex> lock( m_keyWordObserversMutex );
    m_keyWordObservers.insert( keyWordObserver );
}

void AmazonLiteWakewordEngineAdapter::removeKeyWordObserver(std::shared_ptr<KeyWordObserverInterface> keyWordObserver) {
    std::lock_guard<std::mutex> lock(m_keyWordObserversMutex);
    m_keyWordObservers.erase(keyWordObserver);
}

void AmazonLiteWakewordEngineAdapter::onKeyWordDetected(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
        std::string keyword,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex,
        std::shared_ptr<const std::vector<char>> KWDMetadata ) {
    AACE_INFO( LX( TAG, "onKeyWordDetected").d( "keyword", keyword).d("beginIndex",beginIndex).d("endIndex",endIndex));
    std::lock_guard<std::mutex> lock(m_keyWordObserversMutex);
    for( auto keyWordObserver : m_keyWordObservers ) {
        keyWordObserver->onKeyWordDetected( stream, keyword, beginIndex, endIndex, KWDMetadata );
    }
}

bool AmazonLiteWakewordEngineAdapter::setLocale( const std::string& locale ) {
    try {
        ThrowIfNull( m_wakewordDetector, "wakewordDetectorReferenceInvalid" );

        if( m_modelLocaleMap.size() == 0 ) {
            AACE_INFO( LX( TAG, "setLocale" ).m( "switchingWakewordLocaleNotSupported" ) );
            return true;
        }

        if( m_selectedLocale.compare( locale ) != 0 ) {
            std::string pathToLocale = "";
            m_selectedLocale = DEFAULT;
            if( locale.empty() ) {
                // fallback to default locale
                AACE_ERROR( LX( TAG, "setLocale" ).m( "localeIsEmptyFallingBackToDefaultLocale" ) );
            } else if( !checkIfLocaleExistsInMap( locale ) ){
                // fallback to default locale
                AACE_WARN( LX( TAG, "setLocale" ).m( "localeFileNotFoundFallingBackToDefaultLocale" ) );
            } else {
                pathToLocale = getModelPathFromLocale( locale );
                m_selectedLocale = locale;
            }

            m_wakewordDetector.reset();
            if( m_selectedLocale == DEFAULT ) {
                m_wakewordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create( m_audioInputStream, m_audioFormat, {}, {} );
            } else {
                m_wakewordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create( m_audioInputStream, m_audioFormat, {}, {}, pathToLocale );
            }
            ThrowIfNull( m_wakewordDetector, "couldNotCreateWakewordDetector" );

            m_wakewordDetector->addKeyWordObserver( shared_from_this() );
        }

        return true;
    }  catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "setLocale" ).d( "reason", ex.what() ) );
        return false;
    }
}

std::string AmazonLiteWakewordEngineAdapter::getModelPathFromLocale( const std::string& locale ) {
    try{
        ThrowIf( locale.empty(), "localeIsNull" );

        ThrowIf( !checkIfLocaleExistsInMap( locale ), "localeNotFoundInConfiguration" );

        ThrowIf( m_rootPath.empty(), "rootPathIsEmpty" );

        auto it = m_modelLocaleMap.find( locale );
        auto pathToLocale = m_rootPath + "/" + it->second;
        AACE_INFO( LX( TAG, "getModelPathFromLocale" ).d( "pathToLocale", pathToLocale ) );

        return pathToLocale;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX( TAG, "getModelPathFromLocale" ).d( "locale", locale ).d( "reason", ex.what() ) );
        return std::string();
    }
}

bool AmazonLiteWakewordEngineAdapter::checkIfLocaleExistsInMap( const std::string& locale ){
    auto it = m_modelLocaleMap.find( locale );
    return !( it == m_modelLocaleMap.end() );
}

} // aace::engine::amazonLite
} // aace::engine
} // aace