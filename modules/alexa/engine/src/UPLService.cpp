#include <chrono>
#include <ctime>

#include "AACE/Engine/Alexa/UPLService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

static const std::string ALEXA_AUTO_KEYWORD = "AlexaAuto";
static const std::string DELIMITER = "_";
static const std::string PROGRAM_NAME = ALEXA_AUTO_KEYWORD + DELIMITER + "Speech";
static const std::string OFFLINE_NAME = "Offline";
static const std::string SOURCE_NAME = "VoiceRequest";
static const std::string UPL_AUDIO_NAME = "UserPerceivedLatency.Audio";
static const std::string START_CAPTURE_TIME_NAME = "StartCaptureTimestamp";
static const std::string STOP_CAPTURE_TIME_NAME = "StopCaptureTimestamp";
static const std::string PLAYBACK_STARTED_TIME_NAME = "PlaybackStartedTimestamp";
static const std::string PLAYBACK_FINISHED_TIME_NAME = "PlaybackFinishedTimestamp";
static const std::string DIALOG_REQUEST_ID_NAME = "DialogRequestId";

static const std::string TAG("UPLService");

std::shared_ptr<UPLService> UPLService::getInstance() {
    static std::shared_ptr<UPLService> s_instance(new UPLService());
    return s_instance;
}

UPLService::UPLService() : m_dialogId{""}, m_currentState{DialogState::NONE}, m_isOnline(true) {
}

void UPLService::resetStatesForId(const std::string& dialogId) {
    m_dialogId = "";
    m_isOnline = true;
    m_currentState = DialogState::NONE;
    m_stateToTimeMap.clear();
}

void UPLService::updateDialogStateForId(const DialogState currentState, const std::string& dialogId, bool isOnline) {
    //Set dialogId if empty
    if (m_dialogId.empty()) {
        m_dialogId = dialogId;
    }

    //If dialogId doesn't match the current dialogId, then clear existing states
    if (m_dialogId.compare(dialogId) != 0) {
        resetStatesForId(m_dialogId);
        m_dialogId = dialogId;
    }

    //Save current state with current time
    double curTime = getCurrentTimeInMs();
    m_stateToTimeMap.insert({currentState, curTime});

    //Set online or offline
    m_isOnline = isOnline;
    AACE_DEBUG(LX(TAG, "Setting UPL Metric to online or offline:")
                   .d("dialogRequestId", dialogId)
                   .d("connection online", isOnline));

    //Manage state logic based on currentState
    manageStates(currentState, dialogId);
}

void UPLService::manageStates(const DialogState currentState, const std::string& dialogId) {
    //Set current state
    m_currentState = currentState;

    //If current state is the final state (Playback Started), record UPL and reset
    if (m_currentState == DialogState::PLAYBACK_FINISHED) {
        recordUPL();
        resetStatesForId(dialogId);
    }
}

void UPLService::recordUPL() {
    //If dialogId is not set, return
    if (m_dialogId.empty()) {
        AACE_CRITICAL(LX(TAG, "DialogId is empty when trying to upload UPL metric").d("connection online", m_isOnline));
        return;
    }

    //If Stop Capture or Playback Started were not captured, return
    if (m_stateToTimeMap.find(DialogState::START_CAPTURE) == m_stateToTimeMap.end() ||
        m_stateToTimeMap.find(DialogState::STOP_CAPTURE) == m_stateToTimeMap.end() ||
        m_stateToTimeMap.find(DialogState::PLAYBACK_STARTED) == m_stateToTimeMap.end() ||
        m_stateToTimeMap.find(DialogState::PLAYBACK_FINISHED) == m_stateToTimeMap.end()) {
        AACE_CRITICAL(LX(TAG,
                         "One of StartCapture, StopCapture, PlaybackStarted, PlaybackFinished states weren't stored "
                         "when trying to upload UPL metric:")
                          .d("dialogRequestId", m_dialogId)
                          .d("connection online", m_isOnline));
        return;
    }

    //If current state is not Playback Started, return
    if (m_currentState != DialogState::PLAYBACK_FINISHED) {
        AACE_CRITICAL(
            LX(TAG, "Current state is not PlaybackFinished. UPL will only be recorded after PlaybackFinished:")
                .d("dialogRequestId", m_dialogId)
                .d("connection online", m_isOnline));
        return;
    }

    //Utilize double to ensure platform independence when getting timestamp
    double startCaptureTime = m_stateToTimeMap.at(DialogState::START_CAPTURE);
    double stopCaptureTime = m_stateToTimeMap.at(DialogState::STOP_CAPTURE);
    double playbackStartedTime = m_stateToTimeMap.at(DialogState::PLAYBACK_STARTED);
    double playbackFinishedTime = m_stateToTimeMap.at(DialogState::PLAYBACK_FINISHED);

    double userPerceivedLatency = playbackStartedTime - stopCaptureTime;

    //If UPL is valid, then create a MetricEvent, add the calculated UPL and record it
    if (userPerceivedLatency > 0) {
        std::string programName = m_isOnline ? PROGRAM_NAME : PROGRAM_NAME + DELIMITER + OFFLINE_NAME;
        std::shared_ptr<aace::engine::metrics::MetricEvent> currentMetric =
            std::shared_ptr<aace::engine::metrics::MetricEvent>(
                new aace::engine::metrics::MetricEvent(programName, SOURCE_NAME));

        //Add start capture, stop capture, playback started and playback finished times to metric
        currentMetric->addTimer(START_CAPTURE_TIME_NAME, startCaptureTime);
        currentMetric->addTimer(STOP_CAPTURE_TIME_NAME, stopCaptureTime);
        currentMetric->addTimer(PLAYBACK_STARTED_TIME_NAME, playbackStartedTime);
        currentMetric->addTimer(PLAYBACK_FINISHED_TIME_NAME, playbackFinishedTime);

        //Add UPL timer, dialogId and record metric
        currentMetric->addTimer(UPL_AUDIO_NAME, userPerceivedLatency);
        currentMetric->addString(DIALOG_REQUEST_ID_NAME, m_dialogId);
        currentMetric->record();
    } else {
        AACE_CRITICAL(
            LX(TAG, "UserPerceivedLatency value was not valid (less than 0) when trying to upload UPL metric:")
                .d("dialogRequestId", m_dialogId)
                .d("connection online", m_isOnline));
    }
}

double UPLService::getCurrentTimeInMs() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    double duration = now_ms.count();
    return duration;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace