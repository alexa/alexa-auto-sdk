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

#ifndef AACE_MESSAGING_MESSAGING_H
#define AACE_MESSAGING_MESSAGING_H

#include <memory>
#include <string>

#include "AACE/Core/PlatformInterface.h"
#include "MessagingEngineInterface.h"

namespace aace {
namespace messaging {

/**
 * Messaging should be extended to allow a user to use Alexa to interact with a
 * device that provides Short Messaging Service (SMS) capabilities, such as a connected mobile phone.
 *
 * @note The device is referred to as the messaging endpoint.
 *
 * The platform implementation is responsible for uploading unread messages
 * to the cloud when the messaging endpoint is connected, and when new messages
 * are received. Alexa reads messages in the order 
 * they are uploaded. Additionally, the platform implementation is responsible
 * for sending messages, updating message status, and providing the messaging endpoint
 * connection state and permissions.
 */
class Messaging : public aace::core::PlatformInterface {
protected:
    Messaging() = default;

public:
    /**
     * Describes the @c ErrorCode for failed responses.
     */
    using ErrorCode = MessagingEngineInterface::ErrorCode;

    /**
     * Describes the @c ConnectionState of the messaging endpoint.
     */
    using ConnectionState = MessagingEngineInterface::ConnectionState;

    /**
     * Describes the @c PermissionState of the messaging endpoint.
     */
    using PermissionState = MessagingEngineInterface::PermissionState;

    virtual ~Messaging();

    /**
     * Notifies the platform implementation to send an SMS message via the messaging endpoint. The message text
     * contains a short lived URL that contains the audio data from which the message was generated.
     *
     * @param [in] token The identifier for the request.
     * @param [in] message The body of the message, which includes the text and a URL pointing to the audio the audio
     * data from which the message was generated. The text and the URL are separated by a '\n' character.
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
     * @li recipients (required) : The list of phone numbers to which the user intends to send the message.
     * @li address (required) : The phone number of the recipient.
     * @li addressType (required) : Describes the type of value @c address contains. Currently, only "PhoneNumberAddress" is supported.
     */
    virtual void sendMessage(const std::string& token, const std::string& message, const std::string& recipients) = 0;

    /**
     * Notifies the platform implementation to upload all unread messages to the cloud. This method is called if message playback
     * is interrupted or when Alexa has finished reading all messages. This ensures that the cloud and the messaging endpoint
     * stay in sync with the status of unread messages.
     * Upload messages via the @c conversationsReport API using the token received.
     *
     * @param [in] token The identifier for the request.
     */
    virtual void uploadConversations(const std::string& token) = 0;

    /**
     * Notifies the platform implementation that a message has been read, and therefore the
     * messaging device must update the message status to reflect that. The platform implementation is responsible for
     * maintaining the list of unread messages. It must send a response to the cloud via @c updateMessagesStatusSucceeded or
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
    virtual void updateMessagesStatus(
        const std::string& token,
        const std::string& conversationId,
        const std::string& status) = 0;

    /**
     * Notifies the cloud that the @c sendMessage request succeeded.
     *
     * @param [in] token The token received from the @c sendMessage request.
     */
    void sendMessageSucceeded(const std::string& token);

    /**
     * Notifies the cloud that the @c sendMessage request failed.
     *
     * @param [in] token The token received from the @c sendMessage request.
     * @param [in] code The error code describing the reason for the failure.
     * @param [in] message Optional error message describing the failure.
     */
    void sendMessageFailed(const std::string& token, ErrorCode code, const std::string& message = "");

    /**
     * Updates the cloud with the list of unread messages that Alexa will play back. 
     * Call this method when the messaging endpoint is connected and read permission has been granted.
     * Additionally, use this method to reply to a @c uploadConversations request, passing back the token
     * that was sent in that request.
     *
     * @note The conversation report is a JSON Array and therefore must be enclosed in brackets `[]`.
     *
     * The list of conversations is a JSON array. Each conversation must contain a unique identifier
     * that will be used by the @c updateMessagesStatus request to identify the conversation whose messages must be
     * updated.
     *
     * The "otherParticipants" field of the conversation report contains the phone numbers of all participants
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
     * @li otherParticipants.addressType Hard coded string "PhoneNumberAddress" indicating the value of the @c address field.
     * @li messages.id A unique identifier generated by the application for the message.
     * @li messages.payload.@type Hard coded string "text" indicating the value of the @c text field.
     * @li messages.text The text for the message.
     * @li messages.createdTime (optional) The ISO 8601 timestamp of when the message was created on the device.
     * @li messages.sender.address The phone number of the sender.
     * @li messages.sender.addressType Hard coded string "PhoneNumberAddress" indicating the value of the @c address field.
     * @li unreadMessageCount The total number of unread messages in this conversation.
     */
    void conversationsReport(const std::string& token, const std::string& conversations);

    /**
     * Notifies the cloud that the @c updateMessagesStatus request succeeded.
     *
     * @param [in] token The token received from the @sa Messaging::updateMessagesStatus request.
     */
    void updateMessagesStatusSucceeded(const std::string& token);

    /**
     * Notifies the cloud that the @c updateMessagesStatus request failed.
     *
     * @param [in] token The token received from the @sa Messaging::updateMessagesStatus request.
     * @param [in] code The error code describing the reason for the failure.
     * @param [in] message Optional error message describing the failure.
     */
    void updateMessagesStatusFailed(const std::string& token, ErrorCode code, const std::string& message = "");

    /**
     * Allows the platform implementation to provide the connection state of the messaging endpoint,
     * and whether send and read permissions are granted by the user.
     *
     * @param [in] connectionState @c CONNECTED If the messaging endpoint is connected, else @c DISCONNECTED
     * @param [in] sendPermission @c ON If the messaging endpoint can send messages, else @c OFF
     * @param [in] readPermission @c ON If the messaging endpoint messages be read, else @c OFF
     */
    void updateMessagingEndpointState(
        ConnectionState connectionState,
        PermissionState sendPermission,
        PermissionState readPermission);

    /**
     * @internal
     * Sets the Engine interface delegate
     *
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<MessagingEngineInterface> messagingEngineInterface);

private:
    std::shared_ptr<MessagingEngineInterface> m_messagingEngineInterface;
};

/**
 * Converts an enum to a string.
 *
 * @param code The @c ErrorCode.
 * @return The string form of the enum.
 */
inline std::string errorCodeToString(Messaging::ErrorCode code) {
    switch (code) {
        case Messaging::ErrorCode::GENERIC_FAILURE:
            return "GENERIC_FAILURE";
        case Messaging::ErrorCode::NO_CONNECTIVITY:
            return "NO_CONNECTIVITY";
        case Messaging::ErrorCode::NO_PERMISSION:
            return "NO_PERMISSION";
    }

    return "UNKNOWN";
}

/**
 * Converts an enum to a string.
 *
 * @param code The @c ConnectionState.
 * @return The string form of the enum.
 */
inline std::string connectionStateToString(Messaging::ConnectionState connection) {
    switch (connection) {
        case Messaging::ConnectionState::CONNECTED:
            return "CONNECTED";
        case Messaging::ConnectionState::DISCONNECTED:
            return "DISCONNECTED";
    }

    return "UNKNOWN";
}

/**
 * Converts an enum to a string.
 *
 * @param code The @c PermissionState.
 * @return The string form of the enum.
 */
inline std::string permissionStateToString(Messaging::PermissionState permission) {
    switch (permission) {
        case Messaging::PermissionState::ON:
            return "ON";
        case Messaging::PermissionState::OFF:
            return "OFF";
    }

    return "UNKNOWN";
}

}  // namespace messaging
}  // namespace aace

#endif  // AACE_MESSAGING_MESSAGING_H
