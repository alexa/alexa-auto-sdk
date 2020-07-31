/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_WAKEWORD_ENGINE_ADAPTER_H
#define AACE_ENGINE_ALEXA_WAKEWORD_ENGINE_ADAPTER_H

#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * WakewordEngineAdapter is engine interface for supporting the Wakeword Engine Integration 
 * with the Auto SDK 
 * 
 */
class WakewordEngineAdapter {
public:
    /**
     * Constructor.
     */
    WakewordEngineAdapter() = default;

    /**
     * Function to initialize the Wakeword Engine Module.
     *
     * @param audioInputStream The stream of audio data. This audio stream should be PCM
     * encoded, have 16 bits per sample, have a sample rate of 16 kHz, and be in
     * little endian format.
     * 
     * @param audioFormat The format of the audio data located within the stream.
     * 
     * @return returns @c true on successful initialization, otherwise @false.
     * 
     **/

    virtual bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream>& audioInputStream,
        alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat) = 0;

    /**
     * Enables the Wakeword detection in the Wakeword Engine.
     * 
     * @return returns @c true on successful, otherwise @false.
     **/
    virtual bool enable() = 0;

    /**
     * Disable the Wakeword detection in the Wakeword Engine.
     * 
     * @return returns @c true on successful, otherwise @false.
     **/
    virtual bool disable() = 0;

    /**
     * Adds the specified observer to the list of observers to notify of key word detection events.
     *
     * @param keyWordObserver The observer to add.
     */
    virtual void addKeyWordObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver) = 0;

    /**
     * Removes the specified observer to the list of observers to notify of key word detection events.
     *
     * @param keyWordObserver The observer to remove.
     */
    virtual void removeKeyWordObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver) = 0;

    /**
     * Destructor.
     */
    virtual ~WakewordEngineAdapter() = default;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_WAKEWORD_ENGINE_ADAPTER_H
