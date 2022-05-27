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

package com.amazon.aace.customDomain;

import com.amazon.aace.core.PlatformInterface;

/**
 * @c CustomDomain should be extended when custom interface is defined.
 * The implementation of the platform interface should handle receiving custom directives, sending custom events,
 * contexts, processing cancel directive requests from AVS, and report the handling result back to Auto SDK engine. The
 * implementation of the abstract APIs in this class MUST be thread-safe as they will be called by multiple threads when
 * there are multiple custom interfaces defined and configured.
 */
abstract public class CustomDomain extends PlatformInterface {
    public CustomDomain() {}

    /**
     * Describes the type of custom directive handling result
     */
    public enum ResultType {
        /**
         * The directive sent to your client was malformed or the payload does not conform to the directive
         * specification.
         */
        UNEXPECTED_INFORMATION_RECEIVED("UNEXPECTED_INFORMATION_RECEIVED"),

        /**
         * The operation specified by the namespace/name in the directive's header are not supported by the client.
         */
        UNSUPPORTED_OPERATION("UNSUPPORTED_OPERATION"),

        /**
         * An error occurred while the device was handling the directive and the error does not fall into the specified
         * categories.
         */
        INTERNAL_ERROR("INTERNAL_ERROR"),

        /**
         * The directive handling is successful
         */
        SUCCESS("SUCCESS");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ResultType(String name) {
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
     * Handle the action specified by the directive identified by @c directiveNamespace, @c name, and @c payload.
     * If handling this directive succeeds, this @c CustomDomain should call @c reportDirectiveHandlingResult()
     * with matching @c directiveNamespace and @c messageId and @c
     * com.amazon.aace.customDomain.CustomDomain.ResultType.SUCCESS. Otherwise, if handling this directive fails such
     * that subsequent directives with the same dialog request ID should be cancelled, this @c CustomDomain should
     * instead call
     * @c reportDirectiveHandlingResult() with a @c ResultType describing the failure.
     *
     * @note The implementation of this function MUST be thread-safe.
     * @note The implementation of this function MUST return quickly. Failure to do so blocks the processing of
     * subsequent directives
     *
     * @param [in] directiveNamespace The namespace of the directive. It must be prefixed with "Custom" and followed by
     *         the encrypted vendorId from the manufacturer.
     * @param [in] name The name of the directive
     * @param [in] payload An opaque JSON object sent to the device
     * @param [in] correlationToken An opaque token that must be included in any events responding to this directive
     * @param [in] messageId The unique ID of the directive to be handled. Used to report directive handling result.
     */
    public abstract void handleDirective(
            String directiveNamespace, String name, String payload, String correlationToken, String messageId);

    /**
     * Cancel an ongoing @c handleDirective() operation with matching @c directiveNamespace, @c name, @c
     * correlationToken, and @c messageId.
     *
     * @note The implementation of this method MUST be thread-safe.
     * @note The implementation of this method MUST return quickly. Failure to do so blocks the processing
     * of subsequent directives.
     * @note Do not call @c reportDirectiveHandlingResult for a cancelled directive.
     *
     * @param [in] directiveNamespace The namespace of the cancelled directive.
     * @param [in] name The name of the cancelled directive
     * @param [in] correlationToken The correlationToken of the cancelled directive
     * @param [in] messageId The unique ID of the directive to cancel.
     */
    public abstract void cancelDirective(
            String directiveNamespace, String name, String correlationToken, String messageId);

    /**
     * Called to query current custom states under @c contextNamespace from device.
     *
     * @note The implementation of this method MUST be thread-safe.
     *
     * @param [in] contextNamespace The namespace of the queried context
     * @return The queried context in string
     * @code{.json}
     * {
     *       "context": [
     *       {
     *           "name": "{{String}}",
     *           "value": {{Object}} | "{{String}}" | {{Long}},
     *           "timeOfSample": "{{String}}",
     *           "uncertaintyInMilliseconds": {{Long}}
     *       },{
     *           "name": "{{String}}",
     *           "value": {{Object}} | "{{String}}" | {{Long}},
     *           "timeOfSample": "{{String}}",
     *           "uncertaintyInMilliseconds": {{Long}}
     *       },
     *       ...
     *       ]
     * }
     * @endcode
     * @li name (required): The name of the context property state.
     * @li value (required): The value of the context property state. Accepted format: JSON Object, string, or number..
     * @li timeOfSample (optional): The time at which the property value was recorded in ISO-8601 representation. If
     * omitted, the default value is the current time.
     * @li uncertaintyInMilliseconds (optional): The number of milliseconds that have elapsed since the property value
     * was last confirmed. If omitted, the default value is 0.
     */
    public abstract String getContext(String contextNamespace);

    /**
     * Notifies the engine about the result of a directive handling.
     * This should be used in response to @c handleDirective() when the handling is complete.
     *
     * @param [in] directiveNamespace The namespace of the directive.
     * @param [in] messageId The messageId that uniquely identifies which directive this report is for.
     * @param [in] result The result of the handling.
     */
    public final void reportDirectiveHandlingResult(String directiveNamespace, String messageId, ResultType result) {
        reportDirectiveHandlingResult(getNativeRef(), directiveNamespace, messageId, result);
    }

    /**
     * Notifes the engine to send a custom event.
     *
     * @param [in] eventNamespace The namespace of the event.
     * @param [in] name The name of the event.
     * @param [in] payload An opaque JSON object sent to the cloud with the event.
     * @param [in] requiresContext A boolean indicating if this event must be sent with context.
     * @param [in] correlationToken Optional. The token correlating this event to a directive. Empty string if this
     *         event is not a response to any directive.
     * @param [in] customContext Optional. The context corresponding to @a eventNamespace or an empty string if this
     *         event does not require context.
     *
     * customContext is a String representation of a JSON object in the following format:
     * @code{.json}
     * {
     *       "context": [
     *       {
     *           "name": "{{String}}",
     *           "value": {{Object}} | "{{String}}" | {{Long}},
     *           "timeOfSample": "{{String}}",
     *           "uncertaintyInMilliseconds": {{Long}}
     *       },{
     *           "name": "{{String}}",
     *           "value": {{Object}} | "{{String}}" | {{Long}},
     *           "timeOfSample": "{{String}}",
     *           "uncertaintyInMilliseconds": {{Long}}
     *       },
     *       ...
     *       ]
     * }
     * @endcode
     * @li name (required): The name of the context property state.
     * @li value (required): The value of the context property state. Accepted format: JSON Object, string, or number..
     * @li timeOfSample (optional): The time at which the property value was recorded in ISO-8601 representation. If
     * omitted, the default value is the current time.
     * @li uncertaintyInMilliseconds (optional): The number of milliseconds that have elapsed since the property value
     * was last confirmed. If omitted, the default value is 0.
     */
    public final void sendEvent(String eventNamespace, String name, String payload, boolean requiresContext,
            String correlationToken, String customContext) {
        sendEvent(getNativeRef(), eventNamespace, name, payload, requiresContext, correlationToken, customContext);
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
    private native void sendEvent(long nativeRef, String eventNamespace, String name, String payload,
            boolean requiresContext, String correlationToken, String customContext);
    private native void reportDirectiveHandlingResult(
            long nativeRef, String directiveNamespace, String messageId, ResultType result);
}