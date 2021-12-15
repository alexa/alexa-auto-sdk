/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 *
 * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html
 *
 * @deprecated This platform interface is deprecated. 
 *             Use the Alexa Auto Services Bridge (AASB) message broker 
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
class APL : public aace::core::PlatformInterface {
protected:
    APL() = default;

public:
    using ActivityEvent = aace::apl::APLEngineInterface::ActivityEvent;

    virtual ~APL();

    /**
     * Notifies the platform implementation that a @c RenderDocument directive has been received. Once called, the
     * client should render the document based on the APL specification in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.RenderDocument directive which follows the APL specification.
     * @param [in] token The APL presentation token associated with the document in the payload.
     * @param [in] windowId The target windowId.
     */
    virtual void renderDocument(
        const std::string& jsonPayload,
        const std::string& token,
        const std::string& windowId) = 0;

    /**
     * Notifies the platform implementation when the client should clear the APL display card.
     * Once the card is cleared, the platform implementation should call clearCard().
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    virtual void clearDocument(const std::string& token) = 0;

    /**
     * Notifies the platform implementation that an ExecuteCommands directive has been received.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.ExecuteCommands directive in structured JSON format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    virtual void executeCommands(const std::string& jsonPayload, const std::string& token) = 0;

    /**
     * Notifies the platform implementation that a command execution sequence should be interrupted.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    virtual void interruptCommandSequence(const std::string& token) = 0;

    /**
     * Notifies the platform implementation of a dynamic data source update. Please refer to
     * APL documentation for more information:
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html
     *
     * @param [in] sourceType DataSource type.
     * @param [in] jsonPayload The payload of the directive in structured JSON format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    virtual void dataSourceUpdate(
        const std::string& sourceType,
        const std::string& jsonPayload,
        const std::string& token) = 0;

    /**
     * Notifies the platform implementation of APL runtime environment properties that must be updated.
     * The Engine will generate these values based on the @c setAPLProperty() values. The APL runtime
     * will be affected by these values.
     *
     * @param [in] properties A JSON object in string form containing the APL runtime properties that need
     * to be updated.
     * @code {.json}
     * {
     *      "<property name>" : "<property value>"
     * }
     * @endcode
     *
     * @attention Supported names and values.
     *
     * Name          | Value                                        | Description
     * ------------- | -------------------------------------------- | ------------------------------------------------------------
     * drivingState  | parked, moving                               | Sets the Automobile driving state runtime property.
     * theme         | light, dark, light-<themeId>, dark-<themeId> | APL viewport theme value.
     * video         | enabled, disabled                            | Indicates if video should be allowed in the APL document.
     *
     */
    virtual void updateAPLRuntimeProperties(const std::string& properties) = 0;

    /**
     * Notifies the Engine to clear the card from the screen and release any focus being held.
     */
    void clearCard();

    /**
     * Notifies the Engine to clear all pending ExecuteCommands directives and mark them as failed.
     */
    void clearAllExecuteCommands();

    /**
     * Notifies the Engine to send @c UserEvent event to AVS.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#userevent-request
     *
     * @param [in] payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendUserEvent(const std::string& payload);

    /**
     * Notifies the Engine to send a @c LoadIndexListData event to AVS.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#loadindexlistdata-request
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html
     *
     * @param type The type of data source fetch request. The only supported value is "dynamicIndexList".
     * @param payload The @c DataSourceFetchRequest event payload. The caller of this
     * function is responsible to pass the payload as defined by AVS.
     */
    void sendDataSourceFetchRequestEvent(const std::string& type, const std::string& payload);

    /**
     * Notifies the Engine to send an APL @c RuntimeError event to AVS
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#runtimeerror-request
     *
     * @param payload The @c RuntimeError event payload. The caller of this
     * function is responsible to pass the payload as defined by AVS.
     */
    void sendRuntimeErrorEvent(const std::string& payload);

    /**
     * Set the APL version supported by the runtime component.
     *
     * @param [in] aplMaxVersion The APL version supported.
     */
    void setAPLMaxVersion(const std::string& aplMaxVersion);

    /**
     * Set a custom document idle timeout. When the idle timeout is reached,
     * the Engine will call @c clearDocument().
     *
     * @param [in] documentIdleTimeout The timeout in milliseconds.
     * @note Will be reset for every directive received from AVS.
     */
    void setDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout);

    /**
     * Notifies the Engine with the result of a @c renderDocument notification.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    void renderDocumentResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Notifies the Engine with the result of an @c executeCommands notification.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    void executeCommandsResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Notifies the Engine of an activity change event. The APL runtime can
     * report whether the rendered document is active or inactive. If active,
     * the idle timer is stopped and prevents @c clearDocument. If inactive, the
     * idle timer is started and @c clearDocument will be called after timer expiration.
     *
     * @param [in] source The source of the activity event.
     * @param [in] event The activity change event.
     */
    void processActivityEvent(const std::string& source, const ActivityEvent event);

    /**
     * Notifies the Engine of rendered document state. The format of the state is
     * a JSON string representing the payload object of the @c RenderedDocumentState
     * sent with APL events.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#rendereddocumentstate
     *
     * @param [in] state The visual state of the rendered components.
     */
    void sendDocumentState(const std::string& state);

    /**
     * Notifies the Engine of the current window state. The format of the state is
     * a JSON string representing the payload object of the @c WindowState
     * context sent for the @c Alexa.Display.Window interface.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/display-window.html#windowstate-context-object
     *
     * @param [in] state The window state context object.
     */
    void sendDeviceWindowState(const std::string& state);

    /**
     * The rendered APL experience will rely on the value of some defined properties.
     * The platform implementation must report these values to provide a safe visual
     * experience in the vehicle.
     *
     * @param [in] name The name of the property.
     * @param [in] value The value of the property.
     *
     * @note Contact your Solutions Architect for an updated list of automotive themes identifiers.
     *
     * @attention Supported names and values.
     *
     * Name          | Value                                   | Description
     * ------------- | --------------------------------------- | ---------------------------------------------------------
     * drivingState  | parked, moving                          | Set this property when vehicle driving state changes.
     * uiMode        | day, night                              | This affects the contrast of certified APL experiences.
     * themeId       | (night) black, gray, (day) gray1, gray2 | Optional value that can be empty string. Valid values for day and night mode are specified.  
     */
    void setPlatformProperty(const std::string& name, const std::string& value);

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
