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
#include <AVSCommon/SDKInterfaces/LocalPlaybackHandlerInterface.h>

#include <AACE/Engine/Alexa/PlaybackControllerEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.PlaybackControllerEngineImpl");

/// Source name for metrics from PlaybackController
static const std::string METRIC_SOURCE = "PLAYBACK_CONTROLLER-ButtonPressed";

/// Metric key for button press count
static const std::string METRIC_BUTTON_PRESSED_COUNT = "PlaybackButtonPressedCount";

/// Metric key for button control type dimension
static const std::string METRIC_BUTTON_CONTROL_TYPE = "ButtonType";

static void submitButtonPressMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    const std::string& buttonType) {
    auto metricBuilder = MetricEventBuilder().withSourceName(METRIC_SOURCE).withAlexaAgentId();
    metricBuilder.addDataPoint(CounterDataPointBuilder{}.withName(METRIC_BUTTON_PRESSED_COUNT).increment(1).build());
    metricBuilder.addDataPoint(
        StringDataPointBuilder{}.withName(METRIC_BUTTON_CONTROL_TYPE).withValue(buttonType).build());
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

PlaybackControllerEngineImpl::PlaybackControllerEngineImpl(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
    std::shared_ptr<MetricRecorderServiceInterface> metricRecorder) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_playbackControllerPlatformInterface(playbackControllerPlatformInterface),
        m_metricRecorder(metricRecorder) {
}

bool PlaybackControllerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    try {
        m_playbackControllerCapabilityAgent =
            alexaClientSDK::capabilityAgents::playbackController::PlaybackController::create(
                contextManager, messageSender);
        ThrowIfNull(m_playbackControllerCapabilityAgent, "couldNotCreateCapabilityAgent");

        m_playbackRouter = alexaClientSDK::capabilityAgents::playbackController::PlaybackRouter::create(
            m_playbackControllerCapabilityAgent);
        ThrowIfNull(m_playbackRouter, "couldNotCreatePlaybackRouter");

        // register capability configuration with the default endpoint
        capabilitiesRegistrar->withCapabilityConfiguration(m_playbackControllerCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PlaybackControllerEngineImpl> PlaybackControllerEngineImpl::create(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<MetricRecorderServiceInterface> metricRecorder) {
    std::shared_ptr<PlaybackControllerEngineImpl> playbackControllerEngineImpl = nullptr;

    try {
        ThrowIfNull(playbackControllerPlatformInterface, "invalidPlaybackControllerPlatformInterface");
        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(metricRecorder, "invalidMetricRecorder");

        playbackControllerEngineImpl = std::shared_ptr<PlaybackControllerEngineImpl>(
            new PlaybackControllerEngineImpl(playbackControllerPlatformInterface, metricRecorder));

        ThrowIfNot(
            playbackControllerEngineImpl->initialize(capabilitiesRegistrar, messageSender, contextManager),
            "initializePlaybackControllerEngineImplFailed");

        // set the platform engine interface reference
        playbackControllerPlatformInterface->setEngineInterface(playbackControllerEngineImpl);

        return playbackControllerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (playbackControllerEngineImpl != nullptr) {
            playbackControllerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void PlaybackControllerEngineImpl::doShutdown() {
    if (m_playbackRouter != nullptr) {
        m_playbackRouter->shutdown();
    }

    if (m_playbackControllerCapabilityAgent != nullptr) {
        m_playbackControllerCapabilityAgent->shutdown();
    }

    if (!m_metricRecorder.expired()) {
        m_metricRecorder.reset();
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> PlaybackControllerEngineImpl::
    getPlaybackRouter() {
    return m_playbackRouter;
}

void PlaybackControllerEngineImpl::onButtonPressed(PlaybackButton button) {
    AACE_DEBUG(LX(TAG).d("button", button));
    std::stringstream ss;
    ss << button;
    submitButtonPressMetric(m_metricRecorder.lock(), ss.str());
    if (button == PlaybackButton::PAUSE) {
        AACE_DEBUG(LX(TAG).m("Using local stop for PAUSE button press"));
        // PlaybackOperation::STOP_PLAYBACK is used rather than PlaybackOperation::PAUSE_PLAYBACK so that AudioPlayer CA
        // does not require its MediaPlayerInterface to keep the pipeline open. This is equivalent to the effect of
        // using the PlaybackController.PauseCommandIssued event instead of the local operation.
        m_playbackRouter->localOperation(
            alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface::PlaybackOperation::STOP_PLAYBACK);
    } else {
        m_playbackRouter->buttonPressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackButton>(button));
    }
}

void PlaybackControllerEngineImpl::onTogglePressed(PlaybackToggle toggle, bool action) {
    AACE_DEBUG(LX(TAG).d("toggle", toggle).d("action", action));
    std::stringstream ss;
    ss << toggle;
    submitButtonPressMetric(m_metricRecorder.lock(), ss.str());
    m_playbackRouter->togglePressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackToggle>(toggle), action);
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
