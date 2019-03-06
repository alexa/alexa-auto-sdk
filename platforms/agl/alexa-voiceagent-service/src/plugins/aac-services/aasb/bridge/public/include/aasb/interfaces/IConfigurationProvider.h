/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_CONFIGURATION_PROVIDER_H_
#define AASB_CONFIGURATION_PROVIDER_H_

#include <string>

namespace aasb {
namespace bridge {

/**
 * Clients of AASB implements this interface to provide the configuration for various
 * AASB components.
 */
class IConfigurationProvider {
public:
    /**
     * Configuration pertaining to the Audio Input/Output mode and devices.
     */
    struct AudioIOConfiguration {
        /// AASB includes Audio Input Output implementation for various *nix based
        /// platforms. The client has following choices w.r.t. audio io.
        /// 1) Use the default AASB platform implementation, in which case the client
        ///    should set this flag to true and fill out the rest of this configuration
        ///    to provide device information to default implementation.
        /// 2) Implement Audio I/O on your own, in which case the client should set this
        ///    flag to false, and leave out the rest of this configuration. When client
        ///    implements Audio I/O on its own, they will get the events about media
        ///    playback, microphone capture etc. through @c IAlexaCapabilityDirectiveListener.
        bool usePlatformAudioIO;

        /// Used only if @c usePlatformAudioIO is true.
        /// Data required on a platform for creating audio i/o channel.
        void* platformData;

        /// Used only if @c usePlatformAudioIO is true.
        /// Specify the audio output device to use for playing the audio coming from
        /// Alexa speech synthesizer.
        std::string speechSynthesizerDevice;

        /// Used only if @c usePlatformAudioIO is true.
        /// Specify the audio output device to use for playing the audio coming from
        /// Alexa Audio playback directives.
        std::string audioPlayerDevice;

        /// Used only if @c usePlatformAudioIO is true.
        /// Specify the audio input device to use for capturing the audio input which can
        /// be sent to Alexa Cloud, Alexa to Alexa voice calls etc.
        std::string audioInputDevice;

        /**
         * Construct the audio io configuration object.
         */
        AudioIOConfiguration() : usePlatformAudioIO(false), platformData(NULL) {
        }
    };

    /**
     * Provides the audio I/O configuration.
     *
     * @return @c AudioIOConfiguration configuration instance.
     */
    virtual AudioIOConfiguration getAudioIOConfig() = 0;

    /**
     * Gets the path to the directory where application certificates are residing.
     */
    virtual std::string getCertificatesDirectoryPath() = 0;

    /**
     * Gets the path to the directory where various app databases are residing.
     * AACE SDK and AVS SDK will create their own databases in this directory.
     */
    virtual std::string getAppsDataDirectory() = 0;

    /**
     * Gets device serial number of the product to identify the unique instance of
     * the product.
     */
    virtual std::string getProductDSN() = 0;

    /**
     * Gets client id of the product (This can be obtained from AVS developer portal).
     */
    virtual std::string getClientId() = 0;

    /**
     * Gets product id which was registered in AVS developer portal. The product id
     * helps to identify the category of the product.
     */
    virtual std::string getProductId() = 0;

    /**
     * Tells whether to enable wakeword or not.
     *
     * @return true for enabling wakeword, false otherwise
     */
    virtual bool shouldEnableWakeword() = 0;

    /**
     * Tells whether phone call control capability should be enabled.
     *
     * @return true for enabling phone call control, false otherwise
     */
    virtual bool shouldEnablePhoneCallControl() = 0;

    /**
     * Tells whether navigation capability should be enabled.
     *
     * @return true for enabling navigation capability, false otherwise
     */
    virtual bool shouldEnableNavigation() = 0;

    /**
     * Tells whether CBL auth handler should be enabled.
     *
     * @return true for enabling CBL auth functionality, false otherwise
     */
    virtual bool shouldEnableCBL() = 0;

    /**
     * Temporarily added to obtain a location/country from configuration.
     * TODO: Remove once Location provider AAC interface is implemented by routing
     * the requests to AASB client.
     */
    virtual std::pair<float, float> getCurrentLocation() = 0;
    virtual std::string getCountry() = 0;
};

}  // namespace bridge
}  // namespace aasb
#endif  // AASB_CONFIGURATION_PROVIDER_H_
