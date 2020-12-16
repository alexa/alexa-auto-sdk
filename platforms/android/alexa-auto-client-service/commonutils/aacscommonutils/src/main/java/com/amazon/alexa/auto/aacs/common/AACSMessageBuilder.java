package com.amazon.alexa.auto.aacs.common;

import static com.amazon.aacsconstants.AACSConstants.PAYLOAD;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsipc.IPCConstants;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Optional;
import java.util.UUID;

/**
 * Builder class to convert AACS message from message json to object and
 * vice versa.
 */
public class AACSMessageBuilder {
    private static final String TAG = AACSMessageBuilder.class.getSimpleName();

    /**
     * Parse the message json to construct @c AACSMessage object from it.
     *
     * @param message Message Json.
     * @return Parsed object, if json is valid, empty otherwise.
     */
    public static Optional<AACSMessage> parseMessage(@NonNull String message) {
        try {
            JSONObject aasbMessage = new JSONObject(message);
            String topic = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                   .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                   .getString(AASBConstants.TOPIC);
            String action = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                    .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                    .getString(AASBConstants.ACTION);
            String messageId = aasbMessage.getJSONObject(AASBConstants.HEADER).getString(AASBConstants.ID);
            String payload = null;
            if (aasbMessage.has(AASBConstants.PAYLOAD) && !aasbMessage.isNull(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            return Optional.of(new AACSMessage(messageId, topic, action, payload));
        } catch (JSONException exception) {
            Log.d(TAG, "Invalid json. Error:" + exception);
            return Optional.empty();
        }
    }

    /**
     * Parse the Intent to construct @c AACSMessage object from it. Only embedded
     * intents are parsable using this method. For non embedded intents please use
     * AACSReceiver class.
     *
     * @param intent Android Intent.
     * @return Parsed object, if message could be extracted from intent, empty otherwise.
     */
    public static Optional<AACSMessage> parseEmbeddedIntent(@NonNull Intent intent) {
        Bundle payloadBundle = intent.getBundleExtra(PAYLOAD);
        String messageRaw =
                payloadBundle != null ? payloadBundle.getString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE) : null;

        return messageRaw == null ? Optional.<AACSMessage>empty() : parseMessage(messageRaw);
    }

    /**
     * Parse the reply json to construct @c AACSReplyMessage object from it.
     *
     * @param message Message Json.
     * @return Parsed object, if json is valid, empty otherwise.
     */
    public static Optional<AACSReplyMessage> parseReplyMessage(@NonNull String message) {
        try {
            JSONObject aasbMessage = new JSONObject(message);

            String messageId = aasbMessage.getJSONObject(AASBConstants.HEADER).getString(AASBConstants.ID);
            JSONObject messageObj =
                    aasbMessage.getJSONObject(AASBConstants.HEADER).getJSONObject(AASBConstants.MESSAGE_DESCRIPTION);
            String replyToId = messageObj.getString(AASBConstants.REPLY_TO_ID);
            String topic = messageObj.getString(AASBConstants.TOPIC);
            String action = messageObj.getString(AASBConstants.ACTION);

            String payload = null;
            if (aasbMessage.has(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            return Optional.of(new AACSReplyMessage(messageId, replyToId, topic, action, payload));
        } catch (JSONException exception) {
            Log.d(TAG, "Invalid json. Error:" + exception);
            return Optional.empty();
        }
    }

    /**
     * Build a AACS Reply Message JSON from its constituents.
     *
     * @param replyToId If a message is a response to earlier message.
     * @param topic Topic of the message.
     * @param action Action of the message.
     * @param payload Payload of the message.
     *
     * @return Message encoded as JSON.
     */
    public static Optional<String> buildReplyMessage(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, String payload) {
        String uniqueID = UUID.randomUUID().toString();
        try {
            String aasbMessage = "{\n"
                    + "  \"header\" : {\n"
                    + "    \"version\" : \"1.0\",\n"
                    + "    \"messageType\" : \"Reply\",\n"
                    + "    \"id\" : \"" + uniqueID + "\",\n"
                    + "    \"messageDescription\" : {\n"
                    + "      \"topic\" : \"" + topic + "\",\n"
                    + "      \"action\" : \"" + action + "\",\n"
                    + "      \"replyToId\" : \"" + replyToId + "\"\n"
                    + "    }\n"
                    + "  }\n"
                    + "}";
            JSONObject msgObj = new JSONObject(aasbMessage);
            if (payload != null && !payload.isEmpty()) {
                JSONObject payloadJson = new JSONObject(payload);
                msgObj.put("payload", payloadJson);
            }
            return Optional.of(msgObj.toString());
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct AACS message. Error: " + e);
            return Optional.empty();
        }
    }

    /**
     * Build a AACS Message JSON from its constituents.
     *
     * @param topic Topic of the message.
     * @param action Action of the message.
     * @param payload Payload of the message.
     *
     * @return Message encoded as JSON.
     */
    public static Optional<String> buildMessage(@NonNull String topic, @NonNull String action, String payload) {
        String uniqueID = UUID.randomUUID().toString();
        try {
            String aasbMessage = "{\n"
                    + "  \"header\" : {\n"
                    + "    \"version\" : \"1.0\",\n"
                    + "    \"messageType\" : \"Publish\",\n"
                    + "    \"id\" : \"" + uniqueID + "\",\n"
                    + "    \"messageDescription\" : {\n"
                    + "      \"topic\" : \"" + topic + "\",\n"
                    + "      \"action\" : \"" + action + "\"\n"
                    + "    }\n"
                    + "  }\n"
                    + "}";

            JSONObject msgObj = new JSONObject(aasbMessage);
            if (payload != null && !payload.isEmpty()) {
                JSONObject payloadJson = new JSONObject(payload);
                msgObj.put("payload", payloadJson);
            }
            return Optional.of(msgObj.toString());
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct AACS message. Error: " + e);
            return Optional.empty();
        }
    }

    /**
     * Build AACS Embedded Intent to carry the message.
     *
     * @param topic Topic of the message.
     * @param action Action of the message.
     * @param payload Payload of the message.
     *
     * @return Message encoded as JSON.
     */
    public static Optional<Intent> buildEmbeddedMessageIntent(
            @NonNull String topic, @NonNull String action, String payload) {
        return buildMessage(topic, action, payload).map(message -> {
            Intent intent = new Intent(action);
            intent.addCategory(topic);

            Bundle payloadBundle = new Bundle();
            payloadBundle.putString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE, message);
            intent.putExtra(PAYLOAD, payloadBundle);

            return intent;
        });
    }
}
