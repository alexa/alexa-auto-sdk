/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_CAPABILITY_AGENT_H
#define AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_CAPABILITY_AGENT_H

#include <memory>
#include <unordered_set>
#include <deque>

#include "AVSCommon/AVS/Attachment/AttachmentReader.h"
#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Engine/TextToSpeechProvider/TextToSpeechProviderInterface.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace textToSpeechProvider {

using json = nlohmann::json;

class TextToSpeechProviderCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<TextToSpeechProviderCapabilityAgent> {
public:
    static std::shared_ptr<TextToSpeechProviderCapabilityAgent> create(
        std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderInterface> textToSpeechProviderEngine,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /**
     * Destructor.
     */
    virtual ~TextToSpeechProviderCapabilityAgent() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;

    // Function to initate the prepare speech request
    void prepareSpeech(const std::string& speechId, const std::string& text, const std::string& assistantId);

private:
    TextToSpeechProviderCapabilityAgent(
        std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderInterface> textToSpeechProviderEngine,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Send @c ExceptionEncountered and report a failure to handle the @c AVSDirective.
     *
     * @param [in] info The @c AVSDirective that encountered the error and ancillary information.
     * @param [in] type The type of Exception that was encountered.
     * @param [in] message The error message to include in the ExceptionEncountered message.
     */
    void sendExceptionEncounteredAndReportFailed(
        std::shared_ptr<DirectiveInfo> info,
        const std::string& message,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type =
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

    /**
     * Remove a directive from the map of message IDs to @c DirectiveInfo instances.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective whose message ID is to be removed.
     */
    void removeDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * Send the handling completed notification and clean up the resources.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void setHandlingCompleted(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c HandleSpeech directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleHandleSpeechDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by the @c TextToSpeehProvider capability agent.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    /// Function to make the speech synthesis request on an executor
    void executePrepareSpeech(const std::string& speechId, const std::string& text, const std::string& assistantId);

    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getTextToSpeechProviderConfiguration();

    /// Function to validate the payload received in the @c HandleSpeech directive
    bool executeValidatePayload(std::shared_ptr<DirectiveInfo> info, json& payload);

private:
    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// This is the worker thread for the @c TextToSpeehProvider CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderInterface> m_textToSpeechProviderEngine;
};

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_CAPABILITY_AGENT_H
