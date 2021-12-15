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

package com.amazon.aace.apl;

import com.amazon.aace.core.PlatformInterface;

/**
 * APL should be extended to handle receiving Alexa Presentation @c RenderDocument and @c ExecuteCommands directives
 * from AVS. These directives contain metadata for rendering or operating on display cards for devices with GUI support.
 * For more information about Alexa Presentation Language (APL) see the interface overview:
 *
 * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html
 *
 * @deprecated This platform interface is deprecated.
 *             Use the Alexa Auto Services Bridge (AASB) message broker
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
abstract public class APL extends PlatformInterface {
    public APL() {}

    // aace::apl::APL

    /**
     * Enumeration of activity events that could be sent from GUI to @c AlexaPresentation.
     */
    public enum ActivityEvent {
        /**
         * GUI switched to active state.
         * @hideinitializer
         */
        ACTIVATED("ACTIVATED"),
        /**
         * GUI become inactive.
         * @hideinitializer
         */
        DEACTIVATED("DEACTIVATED"),
        /**
         * GUI processed one-time event (touch/scroll/etc).
         * @hideinitializer
         */
        ONE_TIME("ONE_TIME"),
        /**
         * Interrupt event (touch).
         * @hideinitializer
         */
        INTERRUPT("INTERRUPT"),
        /**
         * Guard option for unknown received state.
         * @hideinitializer
         */
        UNKNOWN("UNKNOWN");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ActivityEvent(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Notifies the platform implementation that a @c RenderDocument directive has been received. Once called, the
     * client should render the document based on the APL specification in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.RenderDocument directive which follows the APL
     *         specification.
     * @param [in] token The APL presentation token associated with the document in the payload.
     * @param [in] windowId The target windowId.
     */
    public void renderDocument(String jsonPayload, String token, String windowId) {}

    /**
     * Notifies the platform implementation when the client should clear the APL display card.
     * Once the card is cleared, the platform implementation should call clearCard().
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void clearDocument(String token) {}

    /**
     * Notifies the platform implementation that an ExecuteCommands directive has been received.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.ExecuteCommands directive in structured JSON
     *         format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void executeCommands(String jsonPayload, String token) {}

    /**
     * Notifies the platform implementation of a dynamic data source update. Please refer to
     * APL documentation for more information.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html
     *
     * @param [in] sourceType DataSource type.
     * @param [in] jsonPayload The payload of the directive in structured JSON format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void dataSourceUpdate(String sourceType, String jsonPayload, String token) {}

    /**
     * Notifies the platform implementation that a command execution sequence should be interrupted.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void interruptCommandSequence(String token) {}

    /**
     * Notifies the platform implementation of APL runtime environment properties that must be updated.
     * The Engine will generate these values based on the @c setAPLProperty() values. The APL runtime
     * will be affected by these values.
     *
     * @param [in] properties A JSON object in string form containing the APL runtime properties that need
     * to be updated.
     * @code {.json}
     * {
     *      "drivingState" : "parked|moving",
     *      "theme" : "light|light-gray1|light-gray2|dark|dark-black|dark-gray",
     *      "video" : "enabled|disabled"
     * }
     * @endcode
     *
     */
    public void updateAPLRuntimeProperties(String properties) {}

    /**
     * Notifies the Engine to clear the card from the screen and release any focus being held.
     */
    final protected void clearCard() {
        clearCard(getNativeRef());
    }

    /**
     * Notifies the Engine to clear all pending ExecuteCommands directives and mark them as failed.
     */
    final protected void clearAllExecuteCommands() {
        clearAllExecuteCommands(getNativeRef());
    }

    /**
     * Notifies the Engine to send @c UserEvent event to AVS.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#userevent-request
     *
     * @param [in] payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    final protected void sendUserEvent(String payload) {
        sendUserEvent(getNativeRef(), payload);
    }

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
    final protected void sendDataSourceFetchRequestEvent(String type, String payload) {
        sendDataSourceFetchRequestEvent(getNativeRef(), type, payload);
    }

    /**
     * Notifies the Engine to send an APL @c RuntimeError event to AVS
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#runtimeerror-request
     *
     * @param payload The @c RuntimeError event payload. The caller of this
     * function is responsible to pass the payload as defined by AVS.
     */
    final protected void sendRuntimeErrorEvent(String payload) {
        sendRuntimeErrorEvent(getNativeRef(), payload);
    }

    /**
     * Set the APL version supported by the runtime component.
     *
     * @param [in] aplMaxVersion The APL version supported.
     */
    final protected void setAPLMaxVersion(String aplMaxVersion) {
        setAPLMaxVersion(getNativeRef(), aplMaxVersion);
    }

    /**
     * Set a custom document idle timeout. When the idle timeout is reached,
     * @c clearDocument will be called.
     *
     * @param [in] documentIdleTimeout The timeout in milliseconds.
     * @note Will be reset for every directive received from AVS.
     */
    final protected void setDocumentIdleTimeout(long documentIdleTimeout) {
        setDocumentIdleTimeout(getNativeRef(), documentIdleTimeout);
    }

    /**
     * Notifies the Engine with the result of a @c renderDocument notification.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    final protected void renderDocumentResult(String token, boolean result, String error) {
        renderDocumentResult(getNativeRef(), token, result, error);
    }

    /**
     * Notifies the Engine with the result of an @c executeCommands notification.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     * @param [in] result Rendering result (true on executed, false on exception).
     * @param [in] error Error message provided in case result is false.
     */
    final protected void executeCommandsResult(String token, boolean result, String error) {
        executeCommandsResult(getNativeRef(), token, result, error);
    }

    /**
     * Notifies the Engine of an activity change event. The APL runtime can
     * report whether the rendered document is active or inactive. If active,
     * the idle timer is stopped and prevents @c clearDocument. If inactive, the
     * idle timer is started and @c clearDocument will be called after timer expiration.
     *
     * @param [in] source The source of the activity event.
     * @param [in] event The activity change event.
     */
    final protected void processActivityEvent(String token, ActivityEvent event) {
        processActivityEvent(getNativeRef(), token, event);
    }
    /**
     * Notifies the Engine of rendered document state. The format of the state is
     * a JSON string representing the payload object of the @c RenderedDocumentState
     * sent with APL events.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#rendereddocumentstate
     *
     * @param [in] state The visual state of the rendered components.
     */
    final protected void sendDocumentState(String state) {
        sendDocumentState(getNativeRef(), state);
    }

    /**
     * Notifies the Engine of the current window state. The format of the state is
     * a JSON string representing the payload object of the @c WindowState
     * context sent for the @c Alexa.Display.Window interface.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/display-window.html#windowstate-context-object
     *
     * @param [in] state The window state context object.
     */
    final protected void sendDeviceWindowState(String state) {
        sendDeviceWindowState(getNativeRef(), state);
    }

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
     * Name          | Values         | Description
     * ------------- | -------------- | ---------------------------------------------------------
     * drivingState  | parked, moving | Set this property when vehicle driving state changes.
     * uiMode        | day, night     | This affects the contrast of certified APL experiences.
     * themeId       | <string>       | Contact Solutions Architect for list of values.
     */
    final protected void setPlatformProperty(String name, String value) {
        setPlatformProperty(getNativeRef(), name, value);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void clearCard(long nativeRef);
    private native void clearAllExecuteCommands(long nativeRef);
    private native void sendUserEvent(long nativeRef, String payload);
    private native void sendDataSourceFetchRequestEvent(long nativeRef, String type, String payload);
    private native void sendRuntimeErrorEvent(long nativeRef, String payload);
    private native void setAPLMaxVersion(long nativeRef, String aplMaxVersion);
    private native void setDocumentIdleTimeout(long nativeRef, long documentIdleTimeout);
    private native void renderDocumentResult(long nativeRef, String token, boolean result, String error);
    private native void executeCommandsResult(long nativeRef, String token, boolean result, String error);
    private native void processActivityEvent(long nativeRef, String token, ActivityEvent event);
    private native void sendDocumentState(long nativeRef, String state);
    private native void sendDeviceWindowState(long nativeRef, String state);
    private native void setPlatformProperty(long nativeRef, String name, String value);
}
