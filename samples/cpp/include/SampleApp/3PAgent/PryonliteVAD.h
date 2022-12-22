/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "pryon_lite_PRL2010.h"
#include "AgentPryonListenerInterface.h"
#include "AgentHandler.h"
#include "SampleApp/Logger/LoggerHandler.h"
#include <AACE/Core/MessageBroker.h>
#include "SampleApp/Activity.h"

#include <mutex>
#include <fstream>


namespace sampleApp{
    
namespace arbitrator{

class PryonLiteVAD{
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

private:
    /**
     * @brief 
     * 
     * Loads the given model file into memory.
     *
     * @param fileMem[out] The object into which the file will be loaded into.
     * @param filePath The path to the file to load into memory.
     * @param return @c true if the file was successfully loaded into memory and @c
     * false otherwise.
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
    static void vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent);
    /**
     *  Checks for VAD Begin, if not continues pushing Samples, else starts the calculation for VAD Timestamp.
     */
    void checkForVADBegin(std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>>& startTsBuffer, 
                          std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>>& endTsBuffer);

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
    /// The number of samples to feed to the PryonLite engine per iteration. This
    /// is supplied by PryonLite.
    int32_t m_pryonLiteSamplesPerPush;

    /// Console
    std::weak_ptr<View> m_console{};

    /// The memory allocated to hold PryonLite model instance.
    std::vector<char> m_modelMem;

    /// The memory allocated to hold PryonLite engine instance.
    std::vector<char> m_engineMem;

    // VAD TimestampCalculation
    void VADtimesstamp();
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
    /* 
     * Set the stopThread bool to True (For setting KWD Mode, need to stop all loops).
     */
    void returnBoolThread(bool& stopthread);
private:
    /**
     * @param m_stream The stream from which detection happens.
     * @param m_path The path to that stream.
     * 
     */
    void feedAudio(std::ifstream& m_stream, std::string& m_path);

    // File stream
    std::ifstream m_stream;

    // path to audio file
    std::string m_path;
    
    ssize_t read(char* data, const size_t size,  std::ifstream& m_stream);

    // thread for pushing audio samples
    std::thread m_feedAudioThread;
};


} // namespace arbitrator

} // namespace sampleApp

#endif //PRYONLITE_VAD
