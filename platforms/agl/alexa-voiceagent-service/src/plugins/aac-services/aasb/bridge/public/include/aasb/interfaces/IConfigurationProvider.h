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
#include <vector>

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
        // If the service needs to use the Auto SDK GStreamer Module.
        // Used for AGL and other linux platforms
        bool useGStreamerAudioIO;

        /// Specify the audio input output device to use for capturing the audio input and output.
        std::string voiceInputDevice;
        std::string communicationInputDevice;
        std::string loopbackInputDevice;
        std::string ttsOutputDevice;
        std::string musicOutputDevice;
        std::string notificationOutputDevice;
        std::string alarmOutputDevice;
        std::string earconOutputDevice;
        std::string communicationOutputDevice;
        std::string ringtoneOutputDevice;

        /**
         * Construct the audio io configuration object.
         */
        AudioIOConfiguration() 
            : useGStreamerAudioIO(false),
              voiceInputDevice(""),
              communicationInputDevice(""),
              loopbackInputDevice(""),
              ttsOutputDevice(""),
              musicOutputDevice(""),
              notificationOutputDevice(""),
              alarmOutputDevice(""),
              earconOutputDevice(""),
              communicationOutputDevice(""),
              ringtoneOutputDevice("") {
        }
    };

    struct LocalMediaSourceConfiguration {
        /**
         * Platform has Bluetooth source.
         */
        bool hasBluetooth;
        /**
         * Platform has USB.
         */
        bool hasUSB;
        /**
         * Platform has FM Radio.
         */
        bool hasFMRadio;
        /**
         * Platform has AM Radio.
         */
        bool hasAMRadio;
        /**
         * Platform has Satellite Radio.
         */
        bool hasSatelliteRadio;
        /**
         * Platform has Line In.
         */
        bool hasLineIn;
        /**
         * Platform has Compact Disc.
         */
        bool hasCompactDisc;

        /**
         * Construct the Local Media Source Configuration object.
         */
        LocalMediaSourceConfiguration() :
                hasBluetooth(false),
                hasUSB(false),
                hasFMRadio(false),
                hasAMRadio(false),
                hasSatelliteRadio(false),
                hasLineIn(false),
                hasCompactDisc(false) {
        }
    };

    struct LVCConfiguration {
        /**
         * Root directory for creating sockets
         */
        std::string socketRootDirectory;

        /**
         * Construct the Local Voice Control Configuration object.
         */
        LVCConfiguration() :
                socketRootDirectory("") {
        }
    };

    struct CarControlConfiguration {
        /*
         * Zone
         */
        struct Zone {
            /*
             * Climate controls
             */
            struct Climate {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * Add sync controller
                */
                bool addSyncController;

                /**
                 * Add recirculation controller
                */
                bool addRecirculationController;

                /*
                 * Climate constructor
                 */
                Climate() :
                    enabled(false),
                    addSyncController(false),
                    addRecirculationController(false) {

                }
            };

            struct AirConditioner {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * Modes
                 */
                std::vector<std::string> modes;

                /**
                 *  Air conditioner constructor
                 */
                AirConditioner() :
                    enabled(false) {

                }
            };

            struct Heater {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * minimum
                 */
                double minimum;

                /**
                 * maximum
                 */
                double maximum;

                /**
                 * precision
                 */
                double precision;

                /**
                 * Unit
                 */
                std::string unit;

                /**
                 *  Heater
                 */
                Heater() :
                    enabled(false),
                    minimum(60),
                    maximum(80),
                    precision(1),
                    unit("FAHRENHEIT") {

                }
            };

            struct Fan {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * minimum
                 */
                double minimum;

                /**
                 * maximum
                 */
                double maximum;

                /**
                 * precision
                 */
                double precision;

                /**
                 *  Fan
                 */
                Fan() :
                    enabled(false),
                    minimum(0),
                    maximum(10),
                    precision(1) {

                }
            };

            struct Vent {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * Add vent positions
                */
                std::vector<std::string> positions;

                /**
                 *  Vent
                 */
                Vent() :
                    enabled(false) {
                }
            };

            struct Window {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * minimum
                 */
                bool defrost;

                /**
                 *  Window
                 */
                Window() :
                    enabled(false),
                    defrost(false) {

                }
            };


            struct Light {
                /**
                 * enabled
                 */
                bool enabled;

                /**
                 * Type
                 */
                std::string type;

                /**
                 * lights
                 */
                std::vector<std::string> colors;

                /**
                 *  Light
                 */
                Light() :
                    enabled(false),
                    type("LIGHT") {

                }
            };

            /*
             * Name
             */
            std::string name;

            /*
             * Climate
             */
            Climate climate;

            /*
             * Heater
             */
            Heater heater;

            /*
             * Fan
             */
            Fan fan;

            /*
             * Vent
             */
            Vent vent;

            /*
             * Window
             */
            Window window;

            /*
             * Light
             */
            Light light;

            /*
             * AirConditioner
             */
            AirConditioner airConditioner;

            /*
             * Zone constructor
             */
            Zone() :
                name("zone.all") {

            }
        };

        /**
         * Enabled or disabled
         */ 
        bool enabled;

        /**
         * Climate control zones in the car
         */
        std::vector<Zone> zones;


        /**
         * Construct the Car Control Configuration object.
         */
        CarControlConfiguration() :
                enabled(false) {
        }

    };

    /**
     * Provides the local media source configuration.
     *
     * @return @c LocalMediaSourceConfiguration configuration instance.
     */
    virtual LocalMediaSourceConfiguration getLocalMediaSourceConfig() = 0;

    /**
     * Provides the audio I/O configuration.
     *
     * @return @c AudioIOConfiguration configuration instance.
     */
    virtual AudioIOConfiguration getAudioIOConfig() = 0;

    /**
     * Provides the LVC configuration.
     *
     * @return @c LVCConfiguration configuration instance.
     */
    virtual LVCConfiguration getLocalVoiceControlConfig() = 0;

    /**
     * Provides the CarControl configuration.
     *
     * @return @c CarControl configuration instance.
     */
    virtual CarControlConfiguration getCarControlConfig() = 0;

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
     * Gets the path to external storage directory where AVS endpoint config file can reside.
     */
    virtual std::string getExternalStorageDirectory() = 0;

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
     * Tells whether LocalMediaSource should be enabled.
     *
     * @return true for enabling LocalMediaSource functionality, false otherwise
     */
    virtual bool shouldEnableLocalMediaSource() = 0;

    /**
     * Tells whether Gloria Card templates should be enabled.
     *
     * @return true for enabling Gloria Card templates, false otherwise.
     */
    virtual bool shouldEnableGloriaCard() = 0;

    /**
     * Tells whether Gloria List templates should be enabled.
     *
     * @return true for enabling Gloria List templates, false otherwise.
     */
    virtual bool shouldEnableGloriaList() = 0;

    /**
     * Tells whether Car Control should be enabled.
     *
     * @return true for enabling Car Control, false otherwise.
     */
    virtual bool shouldEnableCarControl() = 0;

    /**
     * Tells Local Voice Control should be enabled.
     *
     * @return true for enabling Local Voice Control, false otherwise.
     */
    virtual bool shouldEnableLocalVoiceControl() = 0;


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
