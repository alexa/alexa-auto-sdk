/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import static com.amazon.alexaautoclientservice.constants.ConfigFieldConstants.UPDATE_SYSTEM_PROPERTY_ALLOWED;
import static com.amazon.alexaautoclientservice.util.AASBUtil.removePackageNameFromString;
import static com.amazon.alexaautoclientservice.util.FileUtil.isAudioInputTypeEnabled;
import static com.amazon.alexaautoclientservice.util.FileUtil.isAudioOutputTypeEnabled;
import static com.amazon.alexaautoclientservice.util.FileUtil.isDefaultImplementationEnabled;

import android.content.Context;
import android.util.Log;
import android.util.Pair;

import androidx.annotation.NonNull;

import com.amazon.aace.core.MessageBroker;
import com.amazon.aace.core.MessageStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.AASBConstants.AudioInput;
import com.amazon.aacsconstants.AASBConstants.AudioOutput;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.modules.alexaClient.AlexaClientMessageHandler;
import com.amazon.alexaautoclientservice.modules.alexaClient.AuthStateObserver;
import com.amazon.alexaautoclientservice.modules.alexaClient.ConnectionStateObserver;
import com.amazon.alexaautoclientservice.modules.audioInput.AudioInputFocusManager;
import com.amazon.alexaautoclientservice.modules.audioInput.AudioInputMessageHandler;
import com.amazon.alexaautoclientservice.modules.audioOutput.AudioOutputMessageHandler;
import com.amazon.alexaautoclientservice.modules.customDomain.CustomDomainMessageDispatcher;
import com.amazon.alexaautoclientservice.receiver.InstrumentationReceiver;
import com.amazon.alexaautoclientservice.util.AASBUtil;
import com.amazon.alexaautoclientservice.util.FileUtil;
import com.amazon.alexaautoclientservice.util.PropertyUtil;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.List;

public class AASBHandler {
    private static final String TAG = AACSConstants.AACS + "-" + AASBHandler.class.getSimpleName();
    private Context mContext;

    private MessageBroker mMessageBroker;
    private AACSSender mAACSSender;
    private AudioOutputMessageHandler mAudioOutput;
    private AudioInputMessageHandler mAudioInput;
    private AudioInputFocusManager mAudioInputFocusManager;
    private AlexaClientMessageHandler mAlexaClient;
    private CustomDomainMessageDispatcher mCustomDomainMessageDispatcher;
    private HashMap<String, String> mAudioInputStreamMap;
    private long mCachedBufferedBytes;
    private static AACSMessageLogger mAACSMessageLogger;

    public AASBHandler(@NonNull Context context, @NonNull MessageBroker messageBroker) {
        mContext = context;
        int cacheCapacity = FileUtil.getIPCCacheCapacity();

        mMessageBroker = messageBroker;
        mMessageBroker.subscribe(this::messageReceived, "*", "*");

        mAACSSender = new AACSSender(cacheCapacity);
        mAlexaClient = new AlexaClientMessageHandler();
        mAudioInputFocusManager = new AudioInputFocusManager(context, mAlexaClient);
        mAudioOutput = new AudioOutputMessageHandler(context, mAlexaClient);
        mAudioInput = new AudioInputMessageHandler(context, mAACSSender, mAudioInputFocusManager);
        mAudioInputStreamMap = new HashMap<>();
        if (BuildConfig.DEBUG) {
            mAACSMessageLogger = new AACSMessageLogger();
        }

        if (FileUtil.isDefaultImplementationEnabled(FileUtil.AACS_CONFIG_CUSTOM_DOMAIN_MESSAGE_DISPATCHER)) {
            mCustomDomainMessageDispatcher = new CustomDomainMessageDispatcher(mAACSSender, mContext);
        }
        mCachedBufferedBytes = 0;
    }

    public void messageReceived(String message) {
        handleMessage(false, message);
    }

    public MessageStream openStream(String streamId, MessageStream.Mode mode) {
        return mMessageBroker.openStream(streamId, mode);
    }

    public void handleMessage(boolean isToEngine, String message) {
        try {
            JSONObject aasbMessage = new JSONObject(message);
            String topic = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                   .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                   .getString(AASBConstants.TOPIC);
            String action = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                    .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                    .getString(AASBConstants.ACTION);
            String replyToId = "";
            if (aasbMessage.getJSONObject(AASBConstants.HEADER)
                            .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                            .has(AASBConstants.REPLY_TO_ID)) {
                replyToId = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                    .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                    .getString(AASBConstants.REPLY_TO_ID);
            }

            String messageId = aasbMessage.getJSONObject(AASBConstants.HEADER).getString(AASBConstants.ID);
            String payload = "";
            if (aasbMessage.has(AASBConstants.PAYLOAD) && !aasbMessage.isNull(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            if (!action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                Log.v(TAG, String.format("Receiving AASBMessage: Topic: %s, Action: %s", topic, action));
            }

            if (BuildConfig.DEBUG) {
                if (InstrumentationReceiver.mIsLogEnabled && !message.isEmpty()) {
                    mAACSMessageLogger.setLogFileLocation(InstrumentationReceiver.mFileLocation);
                    if (isToEngine) {
                        mAACSMessageLogger.buffer(
                                AACSMessageLogger.TO_ENGINE, topic, action, payload, messageId, replyToId);
                    } else {
                        mAACSMessageLogger.buffer(
                                AACSMessageLogger.FROM_ENGINE, topic, action, payload, messageId, replyToId);
                    }
                }
            }
            if (isToEngine) {
                publishMessage(messageId, topic, action, payload, message);
            } else {
                sendDirective(messageId, topic, action, payload, message);
            }

        } catch (Exception e) {
            Log.e(TAG, String.format("Failed to handle AASB message: %s. Exception: %s", message, e.getMessage()));
        }
    }

    private void publishMessage(String messageId, String topic, String action, String payload, String message) {
        if (Topic.AUDIO_OUTPUT.equals(topic) && Action.AudioOutput.MEDIA_STATE_CHANGED.equals(action)) {
            try {
                JSONObject jsonPayload = new JSONObject(payload);
                String channel = jsonPayload.optString(MediaConstants.CHANNEL);
                String state = jsonPayload.optString(MediaConstants.STATE);
                mAudioInputFocusManager.setMediaState(channel, state);
            } catch (Exception e) {
                Log.e(TAG, e.getMessage());
            }
        }
        mMessageBroker.publish(message);
    }

    private void sendDirective(String messageId, String topic, String action, String payload, String message) {
        if (topic.equals(Topic.AUDIO_OUTPUT)) {
            try {
                JSONObject payloadJSON = new JSONObject(payload);
                String channel = payloadJSON.getString(AudioOutput.CHANNEL);
                String type = AudioOutputMessageHandler.convertAudioChannelToAudioType(channel);
                if (isAudioOutputTypeEnabled(type)) {
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
                if (isAudioInputTypeEnabled(type)) {
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
        } else if (topic.equals(Topic.CUSTOM_DOMAIN)) {
            if (isDefaultImplementationEnabled(FileUtil.AACS_CONFIG_CUSTOM_DOMAIN_MESSAGE_DISPATCHER)) {
                mCustomDomainMessageDispatcher.handleCustomDomainMessage(message, messageId, topic, action, payload);
                return;
            }
        } else if (topic.equals(Topic.PROPERTY_MANAGER) && action.equals(Action.PropertyManager.PROPERTY_CHANGED)) {
            if (FileUtil.isEnabledInAACSGeneralConfig(UPDATE_SYSTEM_PROPERTY_ALLOWED)) {
                try {
                    JSONObject payloadJSON = new JSONObject(payload);
                    PropertyUtil.updateSystemProperty(mContext,
                            payloadJSON.getString(AASBConstants.PropertyManager.PROPERTY_NAME),
                            payloadJSON.getString(AASBConstants.PropertyManager.PROPERTY_NEW_VALUE));
                } catch (JSONException e) {
                    Log.e(TAG,
                            String.format("Failed to parse propertyChanged message payload, error message: %s",
                                    e.getMessage()));
                }
            }
        }

        List<TargetComponent> targets = ComponentRegistry.getInstance().findAASBMessageTargets(mContext, topic, action);

        if (targets != null) {
            mAACSSender.sendAASBMessageAnySize(message, action, topic, targets, mContext);
        } else {
            Log.w(TAG, String.format("No target found for topic=%s, action=%s, not calling sender.", topic, action));
        }
    }

    public void publish(@NonNull String topic, @NonNull String action, @NonNull String payload) {
        publish("", topic, action, payload);
    }

    public void publish(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, @NonNull String payload) {
        Pair<String, String> messageWithID = AASBUtil.constructAASBMessageReturnID(replyToId,
                removePackageNameFromString(mContext.getPackageName(), topic),
                removePackageNameFromString(mContext.getPackageName(), action), payload);
        String messageID = messageWithID.first;
        String message = messageWithID.second;

        if (!message.isEmpty()) {
            if (!action.equals(Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                Log.v(TAG, String.format("Publishing AASB message. Topic: %s. Action: %s.", topic, action));
            }

            if (BuildConfig.DEBUG) {
                if (InstrumentationReceiver.mIsLogEnabled && !message.isEmpty()) {
                    mAACSMessageLogger.setLogFileLocation(InstrumentationReceiver.mFileLocation);
                    mAACSMessageLogger.buffer(
                            AACSMessageLogger.TO_ENGINE, topic, action, payload, messageID, replyToId);
                }
            }

            mMessageBroker.publish(message);
        } else {
            Log.e(TAG, "Failed to publish AASB message");
        }
    }

    public void registerAuthStateObserver(AuthStateObserver authStateObserver) {
        if (authStateObserver != null) {
            mAlexaClient.registerAuthStateObserver(authStateObserver);
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
