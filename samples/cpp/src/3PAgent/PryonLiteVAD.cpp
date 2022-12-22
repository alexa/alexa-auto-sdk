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
// JSON for Modern C++
#include <nlohmann/json.hpp>

#include <AACE/Engine/Core/EngineMacros.h>
#include "SampleApp/3PAgent/PryonliteVAD.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <fstream>
#include <ios>
#include <sstream>
#include <unordered_map>

namespace sampleApp{

namespace arbitrator{

using json = nlohmann::json;

#define ALIGN(n) __attribute__((aligned(n)))

/// Lock to surround initialization of the detector.
static std::mutex g_lock;

// #define TIMESTAMP_BUFFER_SIZE 30
ssize_t TIMESTAMP_BUFFER_SIZE = 30;

// flag for indicationg whether PryonLite_instance has created or not.
static bool isPryonliteInstance = false;

// bool to stop the FeedAudio thread
static bool stopThread = false;

// engine handle
static PryonLiteV2Handle sHandle = {0};

/// Constraint to push audio 10msec at a time to the PryonLite engine.
static const int NUM_SAMPLES_PER_PUSH = 160;

long long vadBeginindex = -1;

// pointer to the position from which audio should be read
ssize_t pos = 0;

// Map of PryonLite engine instance to detector objects. This should be locked with
// g_lock before access.
static std::unordered_map<PryonLiteV2Handle*, PryonLiteVAD*> g_engineMap;

PryonLiteVAD::PryonLiteVAD(
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<AgentPryonListenerInterface> listener) :
        m_loggerHandler{std::move(loggerHandler)},
        m_listener{std::move(listener)} {
}

void PryonLiteVAD::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "PryonLiteVAD", message);
}

bool PryonLiteVAD::loadFileIntoMemory(std::vector<char>* fileMem, const std::string& filePath) {
    if (!fileMem) {
        log(logger::LoggerHandler::Level::ERROR, "loadFileIntoMemoryFailed");
        return false;
    }
    std::ifstream inFile(filePath, std::ios::binary | std::ios::ate);
    if (!inFile.good()) {
        log(logger::LoggerHandler::Level::ERROR, "failedToCreateFileStreamWithGivenFilePath " + filePath);
        return false;
    }
    auto size = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    fileMem->resize(size);
    if (!inFile.read(fileMem->data(), fileMem->size())) {
        log(logger::LoggerHandler::Level::ERROR, "failedToReadFromFile");
        return false;
    }
    return true;
}

void PryonLiteVAD::returnBoolThread(bool& stopthread){
    stopThread = stopthread;
}

// ---- Voice Activity Detection ----
// VAD event handler
void PryonLiteVAD::vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent)
{
    std::lock_guard<std::mutex> lock{g_lock};
    vadBeginindex = vadEvent->beginSampleIndex;
}

void PryonLiteVAD::handleEvent(PryonLiteV2Handle *handle, const PryonLiteV2Event* event){
    // ---- Voice Activity Detection ----
    if (event->vadEvent != NULL) {
        vadEventHandler(handle, event->vadEvent);
    }
}

bool PryonLiteVAD::init(std::string& pathToModelFile, std::string& pathToFingerprintsFile, std::string& pathToWatermarkCfgFile) {
    std::lock_guard<std::mutex> lock{g_lock};

    // Configure Engine
    PryonLiteV2Config engineConfig;
    
    // ---- Wakeword ----
    PryonLiteWakewordConfig wakewordConfig = PryonLiteWakewordConfig_Default;
    if(pathToModelFile.empty()){    
        #ifdef AMAZONLITE_USE_EMBEDED_MODEL
            engineConfig.ww->model = static_cast<void*>(prlBinaryModelData);
            engineConfig.ww->sizeofModel = prlBinaryModelLen;
            log(logger::LoggerHandler::Level::INFO, "EmbededModelFileUsed");
        #else
            log(logger::LoggerHandler::Level::ERROR, "EmbededModelFileNotAvailable");
            return false;
        #endif
    }else{
        try{
            loadFileIntoMemory(&m_modelMem, pathToModelFile);
            } catch (std::exception& ex){
                showMessage(ex.what());
            }
        ALIGN(4) static const char* wakewordModelBuffer = m_modelMem.data();
        wakewordConfig.model = wakewordModelBuffer; // 80  // PRLM - pointer to the model
        wakewordConfig.sizeofModel = m_modelMem.size();  //it should be 3.1482 MB size (for this case)
    }
    engineConfig.ww = &wakewordConfig;

    // ---- Fingerprinting ----
    /**
     * @brief If Fingerprint file is not given, Set fingerprint config value to
    *        null pointer.
    */
    PryonLiteFingerprintConfig* fingerprintConfig_ptr = nullptr; // OG
    if (!pathToFingerprintsFile.empty()) {
        // memory allocated to hold the Pryonlite Fingerprint configuration data.
        std::vector<char> fingerprintMem; // Local scope, unneeded after initialization has finished.
        if (!loadFileIntoMemory(&fingerprintMem, pathToFingerprintsFile)) {
            log(logger::LoggerHandler::Level::ERROR,"failedToLoadFingerprintsListIntoMemory");
            return false;
        }
        // if path to Fingerprint Config is given, set the values to Fingerprint default configuration
        PryonLiteFingerprintConfig fingerprintConfig = PryonLiteFingerprintConfig_Default; // OG
        fingerprintConfig_ptr = &fingerprintConfig;
        fingerprintConfig_ptr->fingerprintList =fingerprintMem.data();
        fingerprintConfig_ptr->sizeofFingerprintList =fingerprintMem.size();
    }
    engineConfig.fingerprinter = fingerprintConfig_ptr;

        // ---- Voice Activity Detection ----
    PryonLiteVadConfig vadConfig = PryonLiteVadConfig_Default;
    /**
     * @brief To set Energy Detection values in engineConfig.
     * 
     * PryonLiteEnergyDetectionConfig energyDetectionConfig = PryonLiteEnergyDetectionConfig_Default;
     * energyDetectionConfig.enableGate = 1;
     * vadConfig.energyDetection = &energyDetectionConfig;
    */    
    
    /**
     * @brief Setting the VAD config in  engineConfig with configuration.
     * PryonLiteWebRtcConfig
     * {
     *    0, enableGate 
     *    0,  aggressiveness 
     *    10, frameLenMs  
     *    300,  activationTimeMs  
     *    400,  deactivationTimeMs  
     *    1, strictStop 
     * }
    */
    PryonLiteWebRtcConfig webRtc = PryonLiteWebRtcConfig_Default;
    webRtc.activationTimeMs = 300;
    webRtc.deactivationTimeMs = 400;
    webRtc.strictStop = 1;
    // setting the WebRtc config and psssing it to VAD config
    vadConfig.webRtc = &webRtc;

    engineConfig.vad = &vadConfig;

    // ---- Watermarking ---- //
    /**
     * @brief If watermark Configuration file is not given, Set Watermark config value to
     *        null pointer.
    */
    PryonLiteWatermarkConfig *watermarkConfig_ptr = nullptr;
    if (!pathToWatermarkCfgFile.empty()) {
        /// The memory allocated to hold the PryonLite watermark configuration data.
        std::vector<char> m_watermarkMem;
        if (!loadFileIntoMemory(&m_watermarkMem, pathToWatermarkCfgFile)) {
            log(logger::LoggerHandler::Level::ERROR,"failedToLoadWatermarkCfgListIntoMemory");
            return false;
        }
        // Setting it to default value if the path for Watermark Config is given
        PryonLiteWatermarkConfig watermarkConfig = PryonLiteWatermarkConfig_Default;
        watermarkConfig_ptr = &watermarkConfig;
        watermarkConfig_ptr->config =  m_watermarkMem.data();
        watermarkConfig_ptr->sizeofConfig = m_watermarkMem.size();
    }
    engineConfig.watermark = watermarkConfig_ptr;
    
    // Engine Evnet Configuration
    PryonLiteV2EventConfig engineEventConfig;
    // ---- Fingerprinting ----
    engineEventConfig.enableFingerprintMatchEvent = 0;
    // ---- Voice Activity Detection ----
    engineEventConfig.enableVadEvent = 1; // disable VAD event, set to true to receive VAD events
    // ---- Wakeword ----
    engineEventConfig.enableWwEvent = 0; // set to 0, when VAD event is set to 1 for SiriCarplay integration

    // Initialization of pryonlite code starts
    PryonLiteV2ConfigAttributes configAttributes;
    
    PryonLiteStatus status = PryonLite_GetConfigAttributes(&engineConfig, &engineEventConfig, &configAttributes);
        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            log(logger::LoggerHandler::Level::ERROR, "PryonLite_GetConfigAttributes failed with publicCode: " + std::to_string(status.publicCode));
            return false;
        }

        // m_engine.data() size required to hold the engine
        m_engineMem.resize(configAttributes.requiredMem);

        status = PryonLite_Initialize(&engineConfig, &sHandle, handleEvent, &engineEventConfig, m_engineMem.data(), configAttributes.requiredMem);
        if (status.publicCode != PRYON_LITE_ERROR_OK)
            {
                log(logger::LoggerHandler::Level::ERROR, "PryonLite_Initialize failed with publicCode: " + std::to_string(status.publicCode));
                return false;
            }

        if (!PryonLite_IsInitialized(&sHandle))
            {
                log(logger::LoggerHandler::Level::ERROR, "PryonLite Engine is not Initialized");
                return false;
            }
            m_pryonLiteSamplesPerPush = NUM_SAMPLES_PER_PUSH;

            g_engineMap.insert({&sHandle, this});
            
            m_feedAudioThread = std::thread{&PryonLiteVAD::VADtimesstamp, this};
            
            isPryonliteInstance = true;

    return true;
}

void PryonLiteVAD::VADtimesstamp(){
    ssize_t timesAudioFed = 0;
    std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>> startTsBuffer;
    std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>> endTsBuffer;
    if(startTsBuffer.empty()){
        while (timesAudioFed < TIMESTAMP_BUFFER_SIZE) {
            if(stopThread){
                break;
            }
            startTsBuffer.push_back(std::make_pair(timesAudioFed*160, std::chrono::system_clock::now()));
            feedAudio(m_stream, m_path);
            endTsBuffer.push_back(std::make_pair(timesAudioFed*160+160, std::chrono::system_clock::now()));
            timesAudioFed++;
        }
    }
    // checkForVADBegin(startTsBuffer, endTsBuffer);
}

void PryonLiteVAD::checkForVADBegin(std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>>& startTsBuffer,
                                    std::vector<std::pair<ssize_t, std::chrono::time_point<std::chrono::system_clock>>>& endTsBuffer){
    if (vadBeginindex != -1){
        auto n = startTsBuffer.size();
        for(decltype(n)  i = 0; i < n; i++){
            if(vadBeginindex >= startTsBuffer[i].first && vadBeginindex < endTsBuffer[i].first){
                ssize_t diff = vadBeginindex - startTsBuffer[i].first;
                auto beginIndex = startTsBuffer[i].second + std::chrono::milliseconds(diff/16);   // Dividing by 16 to get Samples -> milliseconds 
                if (auto m_listener_lock = m_listener.lock()) {
                    m_listener_lock->setStartofSpeechTimeStamp(beginIndex);
                }
            }
        }
        return;
    } else if(stopThread){
            return;
    } else{
        if(!startTsBuffer.empty()){
            startTsBuffer.erase(startTsBuffer.begin());
            startTsBuffer.push_back(std::make_pair(startTsBuffer.back().first + 160, std::chrono::system_clock::now()));
            feedAudio(m_stream, m_path);
            endTsBuffer.erase(endTsBuffer.begin());
            endTsBuffer.push_back(std::make_pair(endTsBuffer.back().first + 160, std::chrono::system_clock::now()));
            checkForVADBegin(startTsBuffer, endTsBuffer);
        }
    }
}

void PryonLiteVAD::feedAudio(std::ifstream& m_stream, std::string& m_path) {
    if (m_stream.is_open()) {
        m_stream.close();
    }
    m_stream.open(m_path, std::ios::binary);
    int16_t buffer[NUM_SAMPLES_PER_PUSH] = {0};
    ssize_t bsize = NUM_SAMPLES_PER_PUSH * 2; // 320 bytes
    ssize_t wordsReadSoFar = 0;
    m_stream.seekg(pos);
    while (wordsReadSoFar < NUM_SAMPLES_PER_PUSH) {
        if(stopThread){
            break;
        }
        //ssize_t wordsReadThisIteration = 0;

        // read from file stream
        auto wordsReadThisIteration = read((char*)buffer, bsize, m_stream);

        if (wordsReadThisIteration > 0) {
            wordsReadSoFar += wordsReadThisIteration;
        }
        if (wordsReadThisIteration < bsize) {
            std::memset(((char*)buffer) + wordsReadThisIteration, 0, bsize - wordsReadThisIteration);
        }
    }
    if (wordsReadSoFar == m_pryonLiteSamplesPerPush) {
        PryonLiteStatus status = PryonLite_PushAudioSamples(&sHandle, buffer, m_pryonLiteSamplesPerPush);
        if (status.publicCode != PRYON_LITE_ERROR_OK) {
            log(logger::LoggerHandler::Level::ERROR, "Unable to push audio data with publicCode: " + std::to_string(status.publicCode));
            return;
        }
    }    
    pos += NUM_SAMPLES_PER_PUSH;
}
 
ssize_t PryonLiteVAD::read(char* data, const size_t size, std::ifstream& m_stream) {
    if (m_stream.eof()) {
        return 0;
    }
    // read the data from the stream
    m_stream.read(data, size);
    if (m_stream.bad()) {
        return 0;
    }
    // get the number of bytes read
    ssize_t count = m_stream.gcount();
    m_stream.tellg();
    return count;
}

void PryonLiteVAD::destroy(){
    if (isPryonliteInstance) {
        if(m_feedAudioThread.joinable()){
            m_feedAudioThread.join();
        }
        // STEP 3.1 - Functionality-specific cleanup should be implemented below
        if(m_stream.is_open()){
            m_stream.close();
        }
        // STEP 3.2 - Engine cleanup
        {
            std::lock_guard<std::mutex> lock{g_lock};
            g_engineMap.erase(&sHandle);
        }
        
        // Pryonlite engine destroy
        PryonLiteStatus status = PryonLite_Destroy(&sHandle);
        if (status.publicCode != PRYON_LITE_ERROR_OK) {
            log(logger::LoggerHandler::Level::ERROR, "Pryonlite is not Destroyed with publicCode" + std::to_string(status.publicCode));
            return;
        };
        isPryonliteInstance = false;
        showMessage("Pryonlite instance destroyed successfully");
    } else{
        showMessage("No instance of Pryonlite detected");
    }
    if (stopThread) {
        stopThread = false;
    }
}

void PryonLiteVAD::showMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, message);
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}


} //arbitrator namespace

} //sampleApp namespace
