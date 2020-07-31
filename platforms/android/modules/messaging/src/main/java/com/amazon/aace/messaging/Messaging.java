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

package com.amazon.aace.messaging;

import com.amazon.aace.core.PlatformInterface;

import java.util.HashMap;

/**
 * Messaging should be extended to allow a user to use Alexa to interact with a
 * device that provides Short Messaging Service (SMS) messaging capabilities such as a connected mobile phone.
 *
 * @note The device is referred to as the messaging endpoint.
 *
 * The platform implementation is responsible for uploading unread messages
 * to the cloud when the messaging endpoint is connected, and when new messages
 * are received. Messages will be read by Alexa according the order in which
 * they are uploaded. Additionally, the platform implementation is responsible
 * for sending messages, updating message status, and providing the messaging endpoint
 * connection state and permissions.
 */
abstract public class Messaging extends PlatformInterface {
    /**
     * An enum representing @c ErrorCode status error codes.
     */
    public enum ErrorCode {
        /**
         * Generic failure occurred during request processing.
         * @hideinitializer
         */
        GENERIC_FAILURE("GENERIC_FAILURE"),

        /**
         * No connection was found during request processing.
         * @hideinitializer
         */
        NO_CONNECTIVITY("NO_CONNECTIVITY"),

        /**
         * Messaging endpoint does not have needed permissions.
         * @hideinitializer
         */
        NO_PERMISSION("NO_PERMISSION");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ErrorCode(String name) {
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
     * An enum representing @c ConnectionState.
     */
    public enum ConnectionState {
        /**
         * Messaging endpoint is connected.
         * @hideinitializer
         */
        CONNECTED("CONNECTED"),

        /**
         *  Messaging endpoint is disconnected.
         * @hideinitializer
         */
        DISCONNECTED("DISCONNECTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ConnectionState(String name) {
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
     * An enum representing @c PermissionState.
     */
    public enum PermissionState {
        /**
         * Permission is granted.
         * @hideinitializer
         */
        ON("ON"),

        /**
         *  Permission is not granted.
         * @hideinitializer
         */
        OFF("OFF");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PermissionState(String name) {
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
     * Notifies the platform implementation to send an SMS message via the messaging endpoint. The message text
     * contains a short lived URL that contains the audio data from which the message was generated.
     *
     * @param [in] token The identifier for the request.
     * @param [in] message The message that should be sent through the messaging endpoint. The message includes both
     * text followed by a URL to the audio that was used to generate the text message. It is delimited by a '\n'
     * character and can optionally be omitted in the transmission of the text message.
     * @param [in] recipients JSON data containing the recipients of the message.
     * @code{.json}
     * {
     *     "recipients" : [
     *         {
     *             "address" : "{{STRING}}",
     *             "addressType" : "PhoneNumberAddress"
     *         }
     *     ]
     * }
     * @endcode
     * @li recipients (required) : The list of phone numbers where the message should be sent. If there are multiple
     * entries in the list, then the message should be sent as a group message.
     * @li address (required) : The phone number of the recipient.
     * @li addressType (required) : Describes the type of value @c address contains. Currently, only
     * "PhoneNumberAddress" is supported.
     */
    public void sendMessage(String token, String message, String recipients) {}

    /**
     * Notifies the platform implementation to upload all unread messages to the cloud. This method is called if message
     * playback is interrupted or when Alexa has finished reading all messages. This ensures that the cloud and the
     * messaging endpoint stay in sync with the status of unread messages. Messages should be uploaded via the @c
     * conversationsReport API using the token received.
     *
     * @param [in] token The identifier for the request.
     */
    public void uploadConversations(String token){};

    /**
     * Notifies the platform implementation that a messages have been read, and therefore the
     * messaging device should updated to reflect that. It is the responsibility of the platform implementation
     * to maintain the list of unread messages. A response should be sent to the cloud via @c
     * updateMessagesStatusSucceeded or
     * @c updateMessagesStatusFailed depending on the result of the status update.
     *
     * @param [in] token The identifier for the request.
     * @param [in] conversationId The identifier for the conversation whose status will be updated.
     * This is the value provided by the platform via the @c conversationsReport API.
     * @param [in] status Contains the set of message identifiers that need to be updated to the associated status.
     * @code{.json}
     * {
     *     "statusMap" : {
     *         "read" : [{{STRING}}],
     *     }
     * }
     * @endcode
     * @li read The list of message ids that were read in the associated @c conversationId.
     */
    public void updateMessagesStatus(String token, String conversationId, String status){};

    /**
     * Notifies the cloud that the @c sendMessage request succeeded.
     *
     * @param [in] token The token received from the @c sendMessage request.
     */
    final protected void sendMessageSucceeded(String token) {
        sendMessageSucceeded(getNativeRef(), token);
    }

    /**
     * Notifies the cloud that the @c sendMessage request failed.
     *
     * @param [in] token The token received from the @c sendMessage request.
     * @param [in] code The error code describing the reason for the failure.
     * @param [in] message Optional error message describing the failure.
     */
    final protected void sendMessageFailed(String token, ErrorCode code, String message) {
        sendMessageFailed(getNativeRef(), token, code, message);
    }

    /**
     * Updates the cloud with the list of unread messages that Alexa will play back. The platform implementation
     * should call this method when the messaging endpoint is connected and read permission has been granted.
     * Additionally, this method should be called to reply to a @c uploadConversations request, passing back the token
     * that was sent in that request.
     *
     * @note The format of the conversation report should be a JSON Array and therefore be enclosed in brackets `[]`.
     *
     * The list of conversations is provided as a JSON array. Each conversation must contain a unique identifier
     * that will be used by the @c updateMessagesStatus request to identify the conversation whose messages must be
     * updated.
     *
     * The "otherParticipants" field of the conversation report should contain all the phone numbers of the participants
     * in the conversation, except for the phone number of the messaging endpoint.
     *
     * @param [in] The token received from @c uploadConversations, otherwise an empty string.
     * @param [in] conversations A JSON array representing the unread messages of the messaging endpoint.
     * @code{.json}
     * [
     *     {
     *         "id": "{{STRING}}",
     *         "otherParticipants": [
     *             {
     *                 "address":"{{STRING}}",
     *                 "addressType":"PhoneNumberAddress"
     *             }
     *         ],
     *         "messages": [
     *             {
     *                 "id":"{{STRING}}",
     *                 "payload": {
     *                     "@type":"text",
     *                     "text":"{{STRING}}"
     *                 },
     *                 "status":"unread",
     *                 "createdTime":"{{STRING}}",
     *                 "sender": {
     *                     "address":"{{STRING}}",
     *                     "addressType":"PhoneNumberAddress"
     *                 }
     *             }
     *         ],
     *         "unreadMessageCount": {{INTEGER}}
     *     }
     * ]
     * @endcode
     * @li id A unique identifier generated by the application for the conversation.
     * @li otherParticipants Additional recipients of the message used for group conversation or empty.
     * @li otherParticipants.address The phone number of the recipient.
     * @li otherParticipants.addressType Hard coded string "PhoneNumberAddress" indicating the value of the @c address
     * field.
     * @li messages.id A unique identifier generated by the application for the message.
     * @li messages.payload.@type Hard coded string "text" indicating the value of the @c text field.
     * @li messages.text The text for the message.
     * @li messages.createdTime (optional) The ISO 8601 timestamp of when the message was created on the device.
     * @li messages.sender.address The phone number of the sender.
     * @li messages.sender.addressType Hard coded string "PhoneNumberAddress" indicating the value of the @c address
     * field.
     * @li unreadMessageCount The total number of unread messages in this conversation.
     */
    final protected void conversationsReport(String token, String conversations) {
        conversationsReport(getNativeRef(), token, conversations);
    }

    /**
     * Notifies the cloud that the @c updateMessagesStatus request succeeded.
     *
     * @param [in] token The token received from the @sa Messaging::updateMessagesStatus request.
     */
    final protected void updateMessagesStatusSucceeded(String token) {
        updateMessagesStatusSucceeded(getNativeRef(), token);
    }

    /**
     * Notifies the cloud that the @c updateMessagesStatus request failed.
     *
     * @param [in] token The token received from the @sa Messaging::updateMessagesStatus request.
     * @param [in] code The error code describing the reason for the failure.
     * @param [in] message Optional error message describing the failure.
     */
    final protected void updateMessagesStatusFailed(String token, ErrorCode code, String message) {
        updateMessagesStatusFailed(getNativeRef(), token, code, message);
    }

    /**
     * Updates the state of the messaging endpoint. The platform implementation needs to request
     * consent from the end user to allow sending and reading messages. Also, the connection state
     * of the messaging endpoint must be reported when it is connected or disconnected.
     *
     * @param [in] connectionState @c CONNECTED If the messaging endpoint is connected, else @c DISCONNECTED
     * @param [in] sendPermission @c ON If the messaging endpoint can send messages, else @c OFF
     * @param [in] readPermission @c ON If the messaging endpoint messages be read, else @c OFF
     */
    final protected void updateMessagingEndpointState(
            ConnectionState connectionState, PermissionState sendPermission, PermissionState readPermission) {
        updateMessagingEndpointState(getNativeRef(), connectionState, sendPermission, readPermission);
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
    private native void sendMessageSucceeded(long nativeRef, String token);
    private native void sendMessageFailed(long nativeRef, String token, ErrorCode code, String message);
    private native void conversationsReport(long nativeRef, String token, String conversations);
    private native void updateMessagesStatusSucceeded(long nativeRef, String token);
    private native void updateMessagesStatusFailed(long nativeRef, String token, ErrorCode code, String message);
    private native void updateMessagingEndpointState(long nativeRef, ConnectionState isConnected,
            PermissionState sendPermission, PermissionState readPermission);
}