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

package com.amazon.alexaautoclientservice;

import static com.amazon.alexaautoclientservice.util.AASBUtil.removePackageNameFromString;
import static com.amazon.alexaautoclientservice.util.FileUtil.isAudioInputTypeEnabled;
import static com.amazon.alexaautoclientservice.util.FileUtil.isAudioOutputTypeEnabled;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.aasb.AASB;
import com.amazon.aace.aasb.AASBStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.AASBConstants.AudioInput;
import com.amazon.aacsconstants.AASBConstants.AudioOutput;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.modules.alexaClient.AlexaClientMessageHandler;
import com.amazon.alexaautoclientservice.modules.audioInput.AudioInputMessageHandler;
import com.amazon.alexaautoclientservice.modules.audioOutput.AudioOutputMessageHandler;
import com.amazon.alexaautoclientservice.util.AASBUtil;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.List;

public class AASBHandler extends AASB {
    private static final String TAG = AACSConstants.AACS + "-" + AASBHandler.class.getSimpleName();
    private Context mContext;

    private AACSSender mAACSSender;
    private AudioOutputMessageHandler mAudioOutput;
    private AudioInputMessageHandler mAudioInput;
    private AlexaClientMessageHandler mAlexaClient;
    private HashMap<String, String> mAudioInputStreamMap;
    private long mCachedBufferedBytes;

    public AASBHandler(@NonNull Context context) {
        mContext = context;
        mAACSSender = new AACSSender();

        mAlexaClient = new AlexaClientMessageHandler();
        mAudioOutput = new AudioOutputMessageHandler(context, mAlexaClient);
        mAudioInput = new AudioInputMessageHandler(context, mAACSSender);
        mAudioInputStreamMap = new HashMap<>();
        int cacheCapacity = FileUtil.getIPCCacheCapacity(context);
        mAACSSender = new AACSSender(cacheCapacity);
        mCachedBufferedBytes = 0;
    }

    @Override
    public void messageReceived(String message) {
        try {
            JSONObject aasbMessage = new JSONObject(message);
            String topic = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                   .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                   .getString(AASBConstants.TOPIC);
            String action = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                    .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                    .getString(AASBConstants.ACTION);
            String messageId = aasbMessage.getJSONObject(AASBConstants.HEADER).getString(AASBConstants.ID);
            String payload = "";
            if (aasbMessage.has(AASBConstants.PAYLOAD) && !aasbMessage.isNull(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            if (!action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                Log.v(TAG, String.format("Receiving AASBMessage: Topic: %s, Action: %s", topic, action));
            }
            sendDirective(messageId, topic, action, payload, message);
        } catch (Exception e) {
            Log.e(TAG, String.format("Failed to parse AASB message: %s", message));
        }
    }

    private void sendDirective(String messageId, String topic, String action, String payload, String message) {
        if (topic.equals(Topic.AUDIO_OUTPUT)) {
            try {
                JSONObject payloadJSON = new JSONObject(payload);
                String channel = payloadJSON.getString(AudioOutput.CHANNEL);
                String type = AudioOutputMessageHandler.convertAudioChannelToAudioType(channel);
                if (isAudioOutputTypeEnabled(mContext, type)) {
                    Log.i(TAG,
                            String.format("Default audio output implementation for type=%s is enabled. "
                                            + "Routing the message to AACS-AudioOutputMessageHandler.",
                                    type));
                    mAudioOutput.handleAudioOutputMessage(messageId, topic, action, payload, this);
                    return;
                }
                if (action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                    handleGetNumBytesBuffered(messageId);
                }
            } catch (Exception e) {
                Log.e(TAG,
                        String.format("Failed to get isAudioOutputTypeEnabled for messageId=%s, error message: %s",
                                messageId, e.getMessage()));
            }
        } else if (topic.equals(Topic.AUDIO_INPUT)) {
            try {
                JSONObject payloadJSON = new JSONObject(payload);
                String streamId = payloadJSON.getString(AudioInput.STREAM_ID);
                String type = "";
                if (payloadJSON.has(AudioInput.TYPE)) {
                    type = payloadJSON.getString(AudioInput.TYPE);
                    mAudioInputStreamMap.put(streamId, type);
                } else {
                    type = mAudioInputStreamMap.get(streamId);
                    mAudioInputStreamMap.remove(streamId);
                }
                if (isAudioInputTypeEnabled(mContext, type)) {
                    Log.i(TAG,
                            String.format("Default audio input implementation for audioType=%s is enabled. "
                                            + "Routing the message to AACS-AudioInputMessageHandler.",
                                    type));
                    mAudioInput.handleAudioInputMessage(messageId, topic, action, payload, this);
                    return;
                }
            } catch (Exception e) {
                Log.e(TAG,
                        String.format("Failed to get isAudioInputTypeEnabled for messageId=%s, error message: %s",
                                messageId, e.getMessage()));
            }
        } else if (topic.equals(Topic.ALEXA_CLIENT)) {
            mAlexaClient.handleAlexaClientMessage(messageId, topic, action, payload);
        }

        List<TargetComponent> targets = ComponentRegistry.getInstance().findTarget(mContext, topic, action);

        if (targets != null) {
            mAACSSender.sendAASBMessageAnySize(message, action, topic, targets, mContext);
        } else {
            Log.w(TAG, String.format("No target found for topic=%s, action=%s, not calling sender.", topic, action));
        }
    }

    public void publish(@NonNull String topic, @NonNull String action, @NonNull String payload) {
        String message = AASBUtil.constructAASBMessage(
                "", removePackageNameFromString(topic), removePackageNameFromString(action), payload);
        if (!message.isEmpty()) {
            if (!action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                Log.v(TAG, String.format("Publishing AASB message. Topic: %s. Action: %s.", topic, action));
            }
            publish(message);
        } else {
            Log.e(TAG, "Failed to publish AASB message");
        }
    }

    public void publish(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, @NonNull String payload) {
        String message = AASBUtil.constructAASBMessage(
                replyToId, removePackageNameFromString(topic), removePackageNameFromString(action), payload);
        if (!message.isEmpty()) {
            if (!action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                Log.v(TAG, String.format("Publishing AASB message. Topic: %s. Action: %s.", topic, action));
            }
            publish(message);
        } else {
            Log.e(TAG, "Failed to publish AASB message");
        }
    }

    private void handleGetNumBytesBuffered(String replyToId) {
        JSONObject payload = new JSONObject();
        try {
            payload.put("bufferedBytes", mCachedBufferedBytes);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create getNumBytesBuffered JSON payload.");
        }

        publish(replyToId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_NUM_BYTES_BUFFERED, payload.toString());
    }

    public void updateBytesBuffered(String value) {
        mCachedBufferedBytes = Long.parseLong(value);
    }

    public void cleanUp() {
        if (mAACSSender != null) {
            mAACSSender.shutDown();
            mAACSSender = null;
        }

        if (mAlexaClient != null) {
            mAlexaClient.cleanUp();
            mAlexaClient = null;
        }

        if (mAudioInput != null) {
            mAudioInput.cleanUp();
            mAudioInput = null;
        }

        if (mAudioOutput != null) {
            mAudioOutput.cleanUp();
            mAudioOutput = null;
        }

        if (ComponentRegistry.getInstance() != null)
            ComponentRegistry.getInstance().cleanUp();
    }
}
