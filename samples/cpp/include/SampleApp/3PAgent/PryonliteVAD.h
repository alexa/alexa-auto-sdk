/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef PRYONLITE_VAD
#define PRYONLITE_VAD

#include <aal/aal.h>
#include <aal/common.h>
#include <AACE/Core/MessageBroker.h>
#include "AgentPryonListenerInterface.h"
#include "AgentHandler.h"
#include "PrlAudioFile.h"
#include "pryon_lite_PRL2010.h"
#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <memory>
#include <mutex>
#include <fstream>


namespace sampleApp{
    
namespace arbitrator{
class PryonLiteVAD {
private:
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};
    std::weak_ptr<AgentPryonListenerInterface> m_listener{};
 
protected:
    PryonLiteVAD(
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<AgentPryonListenerInterface> listener);
 
public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PryonLiteVAD> {
        return std::shared_ptr<PryonLiteVAD>(new PryonLiteVAD(args...));
    }
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   Mic Audio   //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

public:
    /**
     * Destructor for Mic Recorder
     */
    ~PryonLiteVAD();
    /**
     * Opens Mic channel and Audio
     */
    bool startMicAudio();
    /**
     * Closes Mic channel and Audio
     */
    bool stopMicAudio();
    /**
     * Pushes Audio Sample to Pryonlite
     */
    void onStreamDataCallback(const int16_t* data, const size_t length);

private:    
    /**
     * Creates a recorder for Mic Audio
     */
    aal_handle_t createRecorder(int m_moduleId, int m_sampleRate);

    // To get the ModuleId for the recorder, default module is "GStreamer"
    int getmoduleId(const std::string target);

    // name (empty string)
    std::string m_name;
    // It should be initialized with null pointer
    aal_handle_t m_recorder = nullptr;
    // Name of the device (empty string)
    std::string m_deviceName;
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   Mic Audio   //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

private:
    /**
     * Loads the given model file into memory.
     *
     * @param fileMem[out] The object into which the file will be loaded into.
     * @param filePath The path to the file to load into memory.
     * @param return @c true if the file was successfully loaded into memory and 
     *        @c false otherwise.
     */
    bool loadFileIntoMemory(std::vector<char>* fileMem, const std::string& filePath);
    /**
     * The callback that PryonLite will use to notify this class of events.
     *
     * @param handle A handle to the PryonLite engine instance.
     * @param event Structure containing information on various PryonLite event types.
     */
    static void handleEvent(PryonLiteV2Handle *handle, const PryonLiteV2Event* event);
    /**
     * Handler for Voice Activity Detection (VAD) events.
     *
     * @param handle A handle to the PryonLite engine instance.
     * @param vadEvent VAD State Transition information.
     */
    void vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent);

private:
    /// The listener interface for the handler event.
    std::weak_ptr<AgentPryonListenerInterface> listener;
    /**
     * Log entry.
     */
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    /**
     * Show Message to Console
     */
    void showMessage(const std::string& message);

public:
    /// Console
    std::weak_ptr<View> m_console{};

    /// The memory allocated to hold PryonLite model instance.
    std::vector<char> m_modelMem;

    /// The memory allocated to hold PryonLite engine instance.
    std::vector<char> m_engineMem;

public:
    /**
     * Initializes and creates PryonLite engine that the client will be primarily interacting with. This function should
     * only be called once with each new @c PryonWakeWordDetector.
     *
     * @param pathToModelFile The path to the desired model file to use for detections. If this parameter is empty, then
     * it shall use the embedded model if provided at the compilation time using the build flag
     * @param pathToFingerprintsFile The path to the desired fingerprints list file to
     * use for fingerprint-based suppression of media-induced wakes (may be an empty string if none).
     * @param pathToWatermarkCfgFile The path to the desired watermark configuration file to
     * use for watermark-based suppression of media-induced wakes (may be an empty string if none).
     * AMAZONLITE_KEY_WORD_DETECTOR_EMBEDDED_MODEL_CPP_PATH
     *
     * @return @c true if a new PryonLite engine was created successfully and @c false if there were any errors with
     * initialization.
     */
    bool init(
        std::string& pathToModelFile, 
        std::string& pathToFingerprintsFile, 
        std::string& pathToWatermarkCfgFile);
    /**
     * @brief This joins the internal feedAudio thread and resets the Pryonlite_engine.
     * 
     */
    void destroy();
    /**
     * @brief Feed Audio Method to get the VAD Callback, if voice activity is there.
     * 
     */
    void feedAudio();
    /**
     * @brief Get the Wav File Path
     * 
     * @param path 
     */
    void getWavFilePath(std::string& path);
};


} // namespace arbitrator

} // namespace sampleApp

#endif //PRYONLITE_VAD
