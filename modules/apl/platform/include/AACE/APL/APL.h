/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_APL_APL_H
#define AACE_APL_APL_H

#include <chrono>

#include <AACE/Core/PlatformInterface.h>

#include "APLEngineInterface.h"

/** @file */

namespace aace {
namespace apl {

/**
 * APL should be extended to handle receiving Alexa Presentation @c RenderDocument and @c ExecuteCommands directives from AVS.
 * These directives contain metadata for rendering or operating on display cards for devices with GUI support.
 * For more information about Alexa Presentation Language (APL) see the interface overview: 
 * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html
 */
class APL : public aace::core::PlatformInterface {
protected:
    APL() = default;

public:
    using ActivityEvent = aace::apl::APLEngineInterface::ActivityEvent;

    virtual ~APL();

    /**
     * Retrieve the visual context from the platform implementation.
     * 
     * @return the current VisualContext payload.
     */
    virtual std::string getVisualContext() = 0;

    /**
     * Notifies the platform implementation that a @c RenderDocument directive has been received. Once called, the
     * client should render the document based on the APL specification in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.RenderDocument directive which follows the APL specification.
     * @param [in] token The APL presentation token associated with this payload.
     * @param [in] windowId The target windowId.
     */
    virtual void renderDocument(
        const std::string& jsonPayload,
        const std::string& token,
        const std::string& windowId) = 0;

    /**
     * Notifies the platform implementation when the client should clear the APL display card.
     * Once the card is cleared, the platform implementation should call clearCard().
     */
    virtual void clearDocument() = 0;

    /**
     * Notifies the platform implementation that an ExecuteCommands directive has been received.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.ExecuteCommands directive in structured JSON format.
     * @param [in] token Directive token used to bind result processing.
     */
    virtual void executeCommands(const std::string& jsonPayload, const std::string& token) = 0;

    /**
     * Notifies the platform implementation that a command execution sequence should be interrupted.
     */
    virtual void interruptCommandSequence() = 0;

    /**
     * Notifies the Engine to clear the card from the screen and release any focus being held.
     */
    void clearCard();

    /**
     * Notifies the Engine to clear all pending ExecuteCommands directives and mark them as failed.
     */
    void clearAllExecuteCommands();

    /**
     * Notifies the Engine to send @c UserEvent to AVS.
     *
     * @param [in] payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendUserEvent(const std::string& payload);

    /**
     * Set the APL version supported by the runtime component.
     *
     * @param [in] aplMaxVersion The APL version supported.
     */
    void setAPLMaxVersion(const std::string& aplMaxVersion);

    /**
     * Set a custom document idle timeout.
     *
     * @param [in] documentIdleTimeout The timeout in milliseconds.
     * @note Will be reset for every directive received from AVS.
     */
    void setDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout);

    /**
     * Notifies the Engine with the result of a @c renderDocument notification.
     *
     * @param [in] token The document presentation token.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    void renderDocumentResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Notifies the Engine with the result of an @c executeCommands notification.
     *
     * @param [in] token The document presentation token.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    void executeCommandsResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Notifies the Engine of an activity change event.
     *
     * @param [in] source The source of the activity event.
     * @param [in] event The activity change event.
     */
    void processActivityEvent(const std::string& source, const ActivityEvent event);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<APLEngineInterface> aplEngineInterface);

private:
    std::shared_ptr<APLEngineInterface> m_aplEngineInterface;
};

}  // namespace apl
}  // namespace aace

#endif
