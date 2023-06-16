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

#ifndef AACE_ENGINE_ALEXA_MEDIA_PLAYBACK_REQUESTOR_H
#define AACE_ENGINE_ALEXA_MEDIA_PLAYBACK_REQUESTOR_H

#include <future>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageRequestObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>

namespace aace {
namespace engine {
namespace alexa {

using MessageSenderInterface = alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface;
using ExceptionEncounteredSenderInterface =
    alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface;
using CapabilityConfiguration = alexaClientSDK::avsCommon::avs::CapabilityConfiguration;
using MessageRequestObserverInterface = alexaClientSDK::avsCommon::sdkInterfaces::MessageRequestObserverInterface;

/**
 * This class implements the @c MediaPlaybackRequestor capability agent. This agent is responsible for
 * initiating automatic resuming the media.
 *
 * @note For instances of this class to be cleaned up correctly, @c shutdown() must be called.
 */
class MediaPlaybackRequestor
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public std::enable_shared_from_this<MediaPlaybackRequestor>
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public MessageRequestObserverInterface {
private:
    /**
     * Constructor.
     *
     * @param messageSender The message sender of the capability agent.
     * @param exceptionSender The message sender for exceptions.
     * @param metricRecorder The metric recorder.
     * @return A @c std::shared_ptr to the new @c MediaPlaybackRequestor instance.
     */
    MediaPlaybackRequestor(
        std::shared_ptr<MessageSenderInterface> messageSender,
        std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder);

    /**
     * Create a MediaPlaybackRequestor metric with the specified data and submit
     * it to the @c MetricRecorderServiceInterface for recording.
     * @return @c true if recording succeeded, else @c false
     */
    bool submitMetric(
        uint32_t eventCount,
        uint32_t errorCount,
        const std::string& eventType,
        long long timeSinceBoot,
        aace::engine::metrics::DataPoint sendLatencyData,
        const std::string& errorReason = "SUCCESS");

    /// The @c MessageSenderInterface for sending events.
    std::shared_ptr<MessageSenderInterface> m_messageSender;
    std::shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionSender;
    std::promise<bool> m_sendMessagePromise;
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> m_metricRecorder;
    aace::engine::metrics::DurationDataPointBuilder m_sendLatencyDuration;
    long long m_timeSinceBootMillis;

public:
    /**
     * Creates a new @c MediaPlaybackRequestor instance.
     *
     * @param messageSender The message sender of the capability agent.
     * @param exceptionSender The message sender for exceptions
     * @param metricRecorder The metric recorder
     * @return A @c std::shared_ptr to the new @c MediaPlaybackRequestor instance.
     */
    static std::shared_ptr<MediaPlaybackRequestor> createMediaPlaybackRequestor(
        std::shared_ptr<MessageSenderInterface> messageSender,
        std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder);

    /**
     * Destructor.
     */
    virtual ~MediaPlaybackRequestor() = default;

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    void onDeregistered() override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;

    /**
     *  Sends the requestMediaPlayback event to the Alexa cloud with the invocation reason.
     * @param @c invocationReason either of 
     */
    std::future<bool> requestMediaPlayback(std::string const invocationReason, long long elapsedBootTime);

    /// @name MessageRequestObserverInterface Functions
    /// @{
    void onResponseStatusReceived(MessageRequestObserverInterface::Status status) override;
    void onSendCompleted(MessageRequestObserverInterface::Status status) override;
    void onExceptionReceived(const std::string& exceptionMessage) override;

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown();

    /**
     * Create a MediaPlaybackRequestor error metric with the specified data
     * and submit it to the @c MetricRecorderServiceInterface for recording.
     * @return @c true if recording succeeded, else @c false
     */
    bool submitMediaRequestErrorMetric(long long timeSinceBoot, const std::string& errorReason);
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_ALEXA_MEDIA_PLAYBACK_REQUESTOR_H
