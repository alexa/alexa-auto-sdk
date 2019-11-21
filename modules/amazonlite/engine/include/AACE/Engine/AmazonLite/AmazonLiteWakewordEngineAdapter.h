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

#ifndef AACE_ENGINE_AMAZONLITE_AMAZONLITE_WAKEWORD_ENGINE_ADAPTER_H
#define AACE_ENGINE_AMAZONLITE_AMAZONLITE_WAKEWORD_ENGINE_ADAPTER_H

#include <map>

#include <KWD/AbstractKeywordDetector.h>
#include <AmazonLite/PryonLiteKeywordDetector.h>

#include "AACE/Engine/Alexa/WakewordEngineAdapter.h"

namespace aace {
namespace engine {
namespace amazonLite {

class AmazonLiteWakewordEngineAdapter : 
    public aace::engine::alexa::WakewordEngineAdapter,
    public alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface,
    public std::enable_shared_from_this<AmazonLiteWakewordEngineAdapter> {
    
private:
    AmazonLiteWakewordEngineAdapter( const std::string& defaultLocale, const std::string& rootPath, const std::map<std::string,std::string>& modelLocaleMap );
    
public:
    static std::shared_ptr<AmazonLiteWakewordEngineAdapter> create( const std::string& defaultLocale, const std::string& rootPath, const std::map<std::string,std::string>& modelLocaleMap );

    // WakewordEngineAdapter
    bool initialize( std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream>& audioInputStream, alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat ) override;
    bool enable() override;
    bool disable() override;
    void addKeyWordObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver ) override;
    void removeKeyWordObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver ) override;

    // KeyWordObserverInterface
    void onKeyWordDetected(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
        std::string keyword,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex = KeyWordObserverInterface::UNSPECIFIED_INDEX,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex = KeyWordObserverInterface::UNSPECIFIED_INDEX,
        std::shared_ptr<const std::vector<char>> KWDMetadata = nullptr ) override;

    bool setLocale( const std::string& locale );

private:
    /// Selected Locale
    std::string m_selectedLocale;

    /// Check if the locale is present in the map.
    bool checkIfLocaleExistsInMap( const std::string& locale );

    /// To get the model file path using the configuration elements.
    std::string getModelPathFromLocale( const std::string& locale );

    /// Variable to hold the initialization state of this class.
    bool m_initialized;

    /// Variable to hold the default Locale stored in AVS.
    const std::string m_avsLocale;

    /// Reference to the AbstractKeywordDetector of PryonLite
    std::shared_ptr<alexaClientSDK::kwd::AbstractKeywordDetector> m_wakewordDetector;

    /// Reference to the AudioInputStream provided at the time of initialization
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> m_audioInputStream;

    /// Format of the Audio provided via the AudioInputStream
    alexaClientSDK::avsCommon::utils::AudioFormat m_audioFormat;

    /// The observers to notify on key word detections. This is used in conjuction with m_keyWordObserversMutex
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface>> m_keyWordObservers;

    /// Lock to protect m_keyWordObservers when users wish to add or remove observers
    mutable std::mutex m_keyWordObserversMutex;

    /// configuration elements provided to the EngineService
    const std::string m_rootPath;
    const std::map<std::string,std::string> m_modelLocaleMap;

};

} // aace::engine::amazonLite
} // aace::engine
} // aace

#endif // AACE_ENGINE_AMAZONLITE_AMAZONLITE_WAKEWORD_ENGINE_ADAPTER_H