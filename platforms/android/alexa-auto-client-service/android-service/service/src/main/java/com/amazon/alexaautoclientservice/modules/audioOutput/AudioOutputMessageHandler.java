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
package com.amazon.alexaautoclientservice.modules.audioOutput;

import static com.amazon.aacsconstants.AASBConstants.AudioOutput;

import android.content.Context;
import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.amazon.aace.aasb.AASBStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.alexaautoclientservice.AASBHandler;
import com.amazon.alexaautoclientservice.mediaPlayer.AACSMediaPlayer;
import com.amazon.alexaautoclientservice.mediaPlayer.exo.ExoPlayerHandler;
import com.amazon.alexaautoclientservice.mediaPlayer.raw.RawAudioOutputHandler;
import com.amazon.alexaautoclientservice.modules.alexaClient.AlexaClientMessageHandler;
import com.amazon.alexaautoclientservice.modules.alexaClient.AuthStateObserver;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

public class AudioOutputMessageHandler {
    public static final String TAG = AACSConstants.AACS + "-" + AudioOutputMessageHandler.class.getSimpleName();
    private Context mContext;
    private Map<String, AACSMediaPlayer> mChannelMap;
    private AACSMediaPlayer mCurrentMediaPlayer;
    private AlexaClientMessageHandler mAlexaClient;

    public AudioOutputMessageHandler(@NonNull Context context, @NonNull AlexaClientMessageHandler alexaClient) {
        mContext = context;
        mChannelMap = new HashMap<>();
        mAlexaClient = alexaClient;
    }

    public static String convertAudioChannelToAudioType(@NonNull String audioChannel) {
        switch (audioChannel) {
            case AudioOutput.Channel.ALERTS:
                return AudioOutput.AudioType.ALARM;
            case AudioOutput.Channel.AUDIO_PLAYER:
            case AudioOutput.Channel.SPOTIFY_ESDK_CHANNEL:
                return AudioOutput.AudioType.MUSIC;
            case AudioOutput.Channel.NOTIFICATIONS:
                return AudioOutput.AudioType.NOTIFICATION;
            case AudioOutput.Channel.SPEECH_SYNTHESIZER:
                return AudioOutput.AudioType.TTS;
            case AudioOutput.Channel.SYSTEM_SOUND_PLAYER:
                return AudioOutput.AudioType.EARCON;
            case AudioOutput.Channel.COMMS_COMMUNICATION:
                return AudioOutput.AudioType.COMMUNICATION;
            case AudioOutput.Channel.COMMS_RINGTONE:
                return AudioOutput.AudioType.RINGTONE;
            default:
                Log.w(TAG, "Cannot find an audio type associated with channel " + audioChannel);
                return "";
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public void handleAudioOutputMessage(@NonNull String messageId, @NonNull String topic, @NonNull String action,
            @NonNull String payload, @NonNull AASBHandler aasbHandler) {
        Log.d(TAG, "handleAudioOutputMessage " + action + payload);
        JSONObject payloadJson;
        if (!payload.isEmpty()) {
            try {
                payloadJson = new JSONObject(payload);
                mCurrentMediaPlayer = getMediaPlayer(payloadJson, aasbHandler);
                if (mCurrentMediaPlayer == null) {
                    Log.e(TAG, "Failed to get the media player, exiting");
                    return;
                }

                switch (action) {
                    case Action.AudioOutput.PREPARE:
                        handlePrepare(payloadJson, aasbHandler);
                        break;
                    case Action.AudioOutput.PLAY:
                        handlePlay(payloadJson);
                        break;
                    case Action.AudioOutput.PAUSE:
                        handlePause(payloadJson);
                        break;
                    case Action.AudioOutput.RESUME:
                        handleResume(payloadJson);
                        break;
                    case Action.AudioOutput.STOP:
                        handleStop(payloadJson);
                        break;
                    case Action.AudioOutput.GET_POSITION:
                        handleGetPosition(payloadJson, messageId);
                        break;
                    case Action.AudioOutput.GET_DURATION:
                        handleGetDuration(payloadJson, messageId);
                        break;
                    case Action.AudioOutput.GET_NUM_BYTES_BUFFERED:
                        handleGetNumBytesBuffered(payloadJson, messageId);
                    case Action.AudioOutput.SET_POSITION:
                        handleSetPosition(payloadJson);
                        break;
                    case Action.AudioOutput.VOLUME_CHANGED:
                        handleVolumeChanged(payloadJson);
                        break;
                    case Action.AudioOutput.MUTED_STATE_CHANGED:
                        handleMutedStateChanged(payloadJson);
                        break;
                    default:
                        Log.w(TAG, action + " message not being handled.");
                }

            } catch (Exception e) {
                Log.e(TAG, "Failed to parse payload");
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private AACSMediaPlayer getMediaPlayer(JSONObject payloadJson, AASBHandler aasbHandler) {
        if (payloadJson == null) {
            Log.e(TAG, "failed to get the mediaPlayer because the payload JSONObject was null.");
            return null;
        }

        String channel = "";
        String encoding = "";
        AACSMediaPlayer mediaPlayer;
        try {
            channel = payloadJson.getString(AudioOutput.CHANNEL);
            String type = convertAudioChannelToAudioType(channel);
            Log.i(TAG, "channel: " + channel);
            if (!mChannelMap.containsKey(channel)) {
                if (type.equals(AudioOutput.AudioType.COMMUNICATION)) {
                    mediaPlayer = new RawAudioOutputHandler(mContext, channel, type, aasbHandler::publish);
                } else {
                    mediaPlayer = new ExoPlayerHandler(mContext, channel, type, aasbHandler::publish);
                }
                mChannelMap.put(channel, mediaPlayer);
                if (mediaPlayer instanceof AuthStateObserver) {
                    mAlexaClient.registerAuthStateObserver((AuthStateObserver) mediaPlayer);
                }
            } else {
                mediaPlayer = mChannelMap.get(channel);
            }
            return mediaPlayer;
        } catch (Exception e) {
            Log.e(TAG, String.format("failed to get the mediaPlayer, because %s", e.getMessage()));
            return null;
        }
    }

    private void handlePrepare(JSONObject payloadJson, AASBHandler aasbHandler) {
        String token = "";
        Boolean repeating;
        try {
            token = payloadJson.getString(AudioOutput.TOKEN);
            repeating = payloadJson.getBoolean(AudioOutput.REPEATING);
            if (payloadJson.has(AudioOutput.STREAM_ID)) {
                String streamId = payloadJson.getString(AudioOutput.STREAM_ID);
                if (!streamId.isEmpty()) {
                    AASBStream currentOutputStream = aasbHandler.openStream(streamId, AASBStream.Mode.READ);
                    mCurrentMediaPlayer.prepare(currentOutputStream, repeating, token);
                }
            } else {
                String url = payloadJson.getString(AudioOutput.URL);
                mCurrentMediaPlayer.prepare(url, repeating, token);
            }

        } catch (Exception e) {
            Log.e(TAG, "failed to prepare");
        }
    }

    private void handlePlay(JSONObject payloadJson) {
        mCurrentMediaPlayer.play();
    }

    private void handlePause(JSONObject payloadJson) {
        mCurrentMediaPlayer.pause();
    }

    private void handleResume(JSONObject payloadJson) {
        mCurrentMediaPlayer.resume();
    }

    private void handleStop(JSONObject payloadJson) {
        mCurrentMediaPlayer.stop();
    }

    private void handleGetPosition(JSONObject payloadJson, String messageId) {
        mCurrentMediaPlayer.getPosition(messageId);
    }

    private void handleGetDuration(JSONObject payloadJson, String messageId) {
        mCurrentMediaPlayer.getDuration(messageId);
    }

    private void handleGetNumBytesBuffered(JSONObject payloadJson, String messageId) {
        mCurrentMediaPlayer.getNumBytesBuffered(messageId);
    }

    private void handleSetPosition(JSONObject payloadJson) {
        try {
            if (!(mCurrentMediaPlayer instanceof RawAudioOutputHandler)) {
                long position = payloadJson.getLong(AudioOutput.POSITION);
                if (position > 0)
                    mCurrentMediaPlayer.setPosition(position);
            }
        } catch (Exception e) {
            Log.e(TAG, "failed to set position");
        }
    }

    private void handleVolumeChanged(JSONObject payloadJson) {
        try {
            double volume = payloadJson.getDouble(AudioOutput.VOLUME);
            mCurrentMediaPlayer.volumeChanged((float) volume);
        } catch (Exception e) {
            Log.e(TAG, "failed to handle volume changed");
        }
    }

    private void handleMutedStateChanged(JSONObject payloadJson) {
        try {
            String state = payloadJson.getString(AudioOutput.STATE);
            mCurrentMediaPlayer.mutedStateChanged(state);
        } catch (Exception e) {
            Log.e(TAG, "failed to handle muted state changed");
        }
    }

    /**
     * This method is called when service is destroyed.
     */
    public void cleanUp() {
        for (Map.Entry<String, AACSMediaPlayer> item : mChannelMap.entrySet()) {
            if (item != null && item.getValue() != null) {
                AACSMediaPlayer player = item.getValue();
                player.cleanUp();
            }
        }
    }
}
