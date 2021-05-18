/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.interfaces;

/**
 * Interface to emit APL related events such as data fetch, user,
 * runtime error, activity, etc.
 */
public interface IAPLEventSender {
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
     * Notifies to clear the card from the screen and release any focus being held.
     */
    public void sendClearCard();

    /**
     * Notifies to clear all pending ExecuteCommands directives and mark them as failed.
     */
    public void sendClearAllExecuteCommands();

    /**
     * Notifies to send @c UserEvent event to AVS.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#userevent-request
     *
     * @param payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    public void sendUserEventRequest(String payload);

    /**
     * Notifies to send a @c LoadIndexListData event to AVS.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#loadindexlistdata-request
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html
     *
     * @param type The type of data source fetch request. The only supported value is "dynamicIndexList".
     * @param payload The @c DataSourceFetchRequest event payload. The caller of this
     * function is responsible to pass the payload as defined by AVS.
     */
    public void sendDataSourceFetchEventRequest(String type, String payload);

    /**
     * Notifies the Engine to send an APL @c RuntimeError event to AVS
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#runtimeerror-request
     *
     * @param payload The @c RuntimeError event payload. The caller of this
     * function is responsible to pass the payload as defined by AVS.
     */
    public void sendRuntimeErrorEventRequest(String payload);

    /**
     * Notifies the result of a @c renderDocument notification.
     *
     * @param token The APL presentation token associated with the current rendered document.
     * @param result Rendering result (true on executed, false on exception).
     * @param error Error message provided in case result is false.
     */
    public void sendRenderDocumentResult(String token, boolean result, String error);

    /**
     * Notifies the result of an @c executeCommands notification.
     *
     * @param token The APL presentation token associated with the current rendered document.
     * @param result Rendering result (true on executed, false on exception).
     * @param error Error message provided in case result is false.
     */
    public void sendExecuteCommandsResult(String token, boolean result, String error);

    /**
     * The APL runtime can report whether the rendered document is active or inactive. If active,
     * the idle timer is stopped and prevents @c clearDocument. If inactive, the
     * idle timer is started and @c clearDocument will be called after timer expiration.
     *
     * @param token The APL presentation token associated with the current rendered document.
     * @param event The activity change event.
     */
    public void sendActivityEventRequest(String token, ActivityEvent event);

    /**
     * Sends the current visual context reported by the APL runtime. It is a JSON object containing
     * the presentation token, and components visible on screen.
     *
     * @param context The visual context.
     */
    public void sendContext(String context);

    /**
     * Sends the current device window state.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/display-window.html#windowstate-context-object
     *
     * @param state The payload for the device window state.
     */
    public void sendWindowState(String state);
}
