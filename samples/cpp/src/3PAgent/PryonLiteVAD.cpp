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
#define SAMPLE_RATE 16000

/// Constraint to push audio 10msec at a time to the PryonLite engine.
#define SAMPLES_PER_FRAME 160

#define MAX_PATH_CHARS 256

// wav file
static const std::string WAVFILEPATH = "hello.wav";

/// Lock to surround initialization of the detector.
static std::mutex g_lock;

// flag for indicationg whether PryonLite_instance has created or not.
static bool isPryonliteInstance = false;

// bool to distinguish Mic and AudioFile for Timestamp Calculation 
static bool micOrFileTimestampCalculation;

// engine handle
static PryonLiteV2Handle sHandle = {0};

// initialize Timestamp variable
static std::chrono::time_point<std::chrono::system_clock> startTsBuffer;

// check for VAD Activity
static bool isVADActive = true;

// Counter for detecting no activity
static int activityCount = 0;

// Path to wav file, initializing it with empty string
static std::string pathToShareFolder = "";

// Map of PryonLite engine instance to detector objects. This should be locked with
// g_lock before access.
static std::unordered_map<PryonLiteV2Handle*, PryonLiteVAD*> g_engineMap;

// call functions in static method
static PryonLiteVAD *m_pryonliteVADInstance = nullptr;

PryonLiteVAD::PryonLiteVAD(
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<AgentPryonListenerInterface> listener) :
        m_loggerHandler{std::move(loggerHandler)},
        m_listener{std::move(listener)} {
            m_pryonliteVADInstance = this;
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

// ---- Voice Activity Detection ----
// VAD event handler
void PryonLiteVAD::vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent){
    if (vadEvent->vadState == 1 && isVADActive){
        std::chrono::time_point<std::chrono::system_clock> beginTimeStamp;
        if(micOrFileTimestampCalculation){
            beginTimeStamp = std::chrono::system_clock::now();
        } else{
            long long vadBeginindex = vadEvent->beginSampleIndex;
            beginTimeStamp = std::chrono::milliseconds(vadBeginindex) + startTsBuffer;
        }
        isVADActive = false;
        activityCount++;
        if (auto m_listener_lock = m_listener.lock()) {
            m_listener_lock->setStartofSpeechTimeStamp(beginTimeStamp);
        }
    }
}

void PryonLiteVAD::handleEvent(PryonLiteV2Handle *handle, const PryonLiteV2Event* event){
    // ---- Voice Activity Detection ----
    if (event->vadEvent != NULL) {
        m_pryonliteVADInstance->vadEventHandler(handle, event->vadEvent);
    }
}

bool PryonLiteVAD::init(std::string& pathToModelFile, std::string& pathToFingerprintsFile, std::string& pathToWatermarkCfgFile) {
    std::lock_guard<std::mutex> lock{g_lock};

    // Configure Engine
    PryonLiteV2Config engineConfig;
    
    // ---- Wakeword ----
    PryonLiteWakewordConfig wakewordConfig = PryonLiteWakewordConfig_Default;
        try{
            loadFileIntoMemory(&m_modelMem, pathToModelFile);
            } catch (std::exception& ex){
                log(logger::LoggerHandler::Level::ERROR, ex.what());
            }
        ALIGN(4) static const char* wakewordModelBuffer = m_modelMem.data();
        wakewordConfig.model = wakewordModelBuffer; // 80  // PRLM - pointer to the model
        wakewordConfig.sizeofModel = m_modelMem.size();  //it should be 3.1482 MB size (for this case)
    
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
     *    1, enableGate 
     *    0,  aggressiveness 
     *    10, frameLenMs  
     *    300,  activationTimeMs  
     *    400,  deactivationTimeMs  
     *    1, strictStop 
     * }
    */
    PryonLiteWebRtcConfig webRtc = PryonLiteWebRtcConfig_Default;
    webRtc.enableGate = 1;
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
        if (status.publicCode != PRYON_LITE_ERROR_OK) {
            log(logger::LoggerHandler::Level::ERROR, "PryonLite_GetConfigAttributes failed with publicCode: " + std::to_string(status.publicCode));
            return false;
        }

        // m_engine.data() size required to hold the engine
        m_engineMem.resize(configAttributes.requiredMem);

        status = PryonLite_Initialize(&engineConfig, &sHandle, handleEvent, &engineEventConfig, m_engineMem.data(), configAttributes.requiredMem);
        if (status.publicCode != PRYON_LITE_ERROR_OK) {
            log(logger::LoggerHandler::Level::ERROR, "PryonLite_Initialize failed with publicCode: " + std::to_string(status.publicCode));
            return false;
        }

        if (!PryonLite_IsInitialized(&sHandle)) {
                log(logger::LoggerHandler::Level::ERROR, "PryonLite Engine is not Initialized");
                return false;
        }

        g_engineMap.insert({&sHandle, this});

        isPryonliteInstance = true;

    return true;
}

void PryonLiteVAD::getWavFilePath(std::string& path){
    pathToShareFolder = path.substr(0,path.find("share/"));
}

void PryonLiteVAD::feedAudio() {
    micOrFileTimestampCalculation = false;
    prlAudioFile_format wavFormat;
	prlAudioFile_handle wavFile;
    startTsBuffer = std::chrono::system_clock::now();
    std::string filePath = pathToShareFolder  + "share/sampleapp/inputs/" + WAVFILEPATH;
    std::ifstream file(filePath);
    if (!file){
        log(logger::LoggerHandler::Level::ERROR, "Invalid path to wav file");
        return;
    }   
    // Specify format in case file extension is not .wav
	wavFormat.samplesPerFrame = 1;
	wavFormat.bitsPerSample = 16;
	wavFormat.sampleType = AUDIOFILE_WAVE_FORMAT_PCM;
	wavFormat.samplingRate = SAMPLE_RATE;
	int result;
	char* filepathChar = const_cast<char*>(filePath.c_str());

    if ((result = prlAudioFile_open(&wavFile, (char*)filepathChar, &wavFormat, MAX_PATH_CHARS)) != 0) {
		log(logger::LoggerHandler::Level::ERROR, "wav file is invalid");
		return;
	}
	if ((wavFormat.samplesPerFrame != 1) ||
		(wavFormat.bitsPerSample != 16) ||
		(wavFormat.sampleType != AUDIOFILE_WAVE_FORMAT_PCM) ||
		(wavFormat.samplingRate != SAMPLE_RATE)) {
		log(logger::LoggerHandler::Level::ERROR, "wav file should be 16k 16bit");
		return;
	}
    // Loop through all audio, pushing one 'CHUNK_SIZE' of audio into the decoder on each pass
	// Exit when we run out of audio
    activityCount = 0;
	while (true) {
        size_t framesRead = 0;
	    short samples[SAMPLES_PER_FRAME];
        framesRead = prlAudioFile_readFrames(&wavFile, (char *)&samples, SAMPLES_PER_FRAME);
   		if (framesRead) {
   			PryonLiteStatus status  = PryonLite_PushAudioSamples(&sHandle, samples, SAMPLES_PER_FRAME);
            if (status.publicCode != PRYON_LITE_ERROR_OK) {
                log(logger::LoggerHandler::Level::ERROR, "Unable to push audio data with publicCode: " + std::to_string(status.publicCode));
                return;
            }
   		}
		if (framesRead < SAMPLES_PER_FRAME) {
			break;
		}
        isVADActive = true;
	}
    prlAudioFile_close(&wavFile);
}

void PryonLiteVAD::destroy(){
    if (isPryonliteInstance) {
        
        stopMicAudio();

        // Engine cleanup
        {
            std::lock_guard<std::mutex> lock{g_lock};
            g_engineMap.erase(&sHandle);
        }
        if(activityCount == 0){
            if(isVADActive){
                showMessage("No Voice Activity Detected");
            }
        }
        if(isVADActive){
           isVADActive = false;
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
        log(logger::LoggerHandler::Level::ERROR, "No instance of Pryonlite detected");
    }
    if(!isVADActive){
        isVADActive = true;
    }
    activityCount = 0;
}

void PryonLiteVAD::showMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, message);
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   Mic Audio   //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void PryonLiteVAD::onStreamDataCallback(const int16_t* data, const size_t length) {
    PryonLiteStatus status  = PryonLite_PushAudioSamples(&sHandle, data, length);
    if (status.publicCode != PRYON_LITE_ERROR_OK) {
        log(logger::LoggerHandler::Level::ERROR, "Unable to push audio data with publicCode: " + std::to_string(status.publicCode));
        return;
    }
    isVADActive = true;
}

// clang-format off
static aal_listener_t aalListener = {
    .on_almost_done = nullptr,
    .on_data = [](const int16_t* data, const size_t length, void* user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<PryonLiteVAD*>(user_data);
        self->onStreamDataCallback(data, length);
    },
    .on_data_requested = nullptr
};

int PryonLiteVAD::getmoduleId(const std::string target) {
    int modules = aal_get_module_count();
    for (int i = 0; i < modules; i++) {
        std::string name = aal_get_module_name(i);
        // If the target is empty string, use the first module we found
        if (name == target || target.empty()) {
            return i;
        }
    }
    Throw("Module not found");
}

aal_handle_t PryonLiteVAD::createRecorder(int m_moduleId, int m_sampleRate) {
    // clang-format off
    const aal_attributes_t attr = {
        .name = m_name.c_str(),
        .device = m_deviceName.c_str(),
        .uri = nullptr,
        .listener = &aalListener,
        .user_data = this,
        .module_id = m_moduleId,
    };
    aal_lpcm_parameters_t params = {
        .sample_format = AAL_SAMPLE_FORMAT_DEFAULT,
        .channels = 0,
        .sample_rate = m_sampleRate,
    };
    return aal_recorder_create(&attr, &params);
}

bool PryonLiteVAD::startMicAudio() {
    try {
        if (m_recorder == nullptr) {
            micOrFileTimestampCalculation = true;
            activityCount = 0;
            int moduleId = getmoduleId("");
            int sampleRate = 0;
            ThrowIf(moduleId < 0 || moduleId >= aal_get_module_count(), "invalidModuleId");
            m_recorder = createRecorder(moduleId, sampleRate);
            ThrowIfNull(m_recorder, "createRecorderFailed");
        }
        aal_recorder_play(m_recorder);
        return true;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, ex.what());
        return false;
    }
}

bool PryonLiteVAD::stopMicAudio() {
    try {
        ThrowIfNull(m_recorder, "nullRecorder");
        aal_recorder_stop(m_recorder);
        return true;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, ex.what());
        return false;
    }
}

PryonLiteVAD::~PryonLiteVAD() {
    if (m_recorder) {
        aal_recorder_destroy(m_recorder);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////   Mic Audio   //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


} //arbitrator namespace

} //sampleApp namespace
