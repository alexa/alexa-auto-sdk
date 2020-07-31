/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.TemplateRuntime;

import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.util.Log;

import com.amazon.aace.alexa.ExternalMediaAdapter;
import com.amazon.aace.alexa.TemplateRuntime;
// import com.amazon.aace.alexa.AudioPlayer;
import com.amazon.aace.audio.AudioOutput;
// import com.amazon.sampleapp.impl.ExternalMediaPlayer.MACCPlayer;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;

public class TemplateRuntimeHandler extends TemplateRuntime {
    private static final String sTag = "TemplateRuntime";

    private final LoggerHandler mLogger;
    private final PlaybackControllerHandler mPlaybackController;
    private AudioPlayerStateHandler mAudioPlayerStateHandler = null;
    private String mCurrentAudioItemId;
    private long mDelay, mDuration, mPosition;

    public TemplateRuntimeHandler(LoggerHandler logger, @Nullable PlaybackControllerHandler playbackController) {
        mLogger = logger;
        mPlaybackController = playbackController;
        mAudioPlayerStateHandler = new AudioPlayerStateHandler();
        mDelay = 0;
        mDuration = AudioOutput.TIME_UNKNOWN;
        mPosition = AudioOutput.TIME_UNKNOWN;
    }

    @Override
    public void renderTemplate(String payload, FocusState focusState) {
        try {
            // Log payload
            JSONObject template = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, template.toString(4));
            Log.i(sTag, payload);
            // Log card
            String type = template.getString("type");
            switch (type) {
                case "BodyTemplate1":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.BODY_TEMPLATE1);
                    break;
                case "BodyTemplate2":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.BODY_TEMPLATE2);
                    break;
                case "ListTemplate1":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.LIST_TEMPLATE1);
                    break;
                case "WeatherTemplate":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.WEATHER_TEMPLATE);
                    break;
                case "LocalSearchListTemplate1":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.LOCAL_SEARCH_LIST_TEMPLATE1);
                    break;
                case "LocalSearchListTemplate2":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.LOCAL_SEARCH_LIST_TEMPLATE2);
                    break;
                case "TrafficDetailsTemplate":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.TRAFFIC_DETAILS_TEMPLATE);
                    break;
                case "LocalSearchDetailTemplate1":
                    mLogger.postDisplayCard(template, LogRecyclerViewAdapter.LOCAL_SEARCH_DETAIL_TEMPLATE1);
                    break;
                default:
                    mLogger.postError(sTag, "Unknown Template sent");
                    break;
            }
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void renderPlayerInfo(String payload, PlayerActivity audioPlayerState, long offset, FocusState focusState) {
        try {
            JSONObject playerInfo = new JSONObject(payload);
            String audioItemId = playerInfo.getString("audioItemId");
            JSONObject content = playerInfo.getJSONObject("content");
            JSONObject audioProvider = content.getJSONObject("provider");
            String providerName = audioProvider.getString("name");

            // Update playback controller buttons and player info labels
            if (mPlaybackController != null) {
                // reset visual state
                mPlaybackController.hidePlayerInfoControls();

                if (playerInfo.has("controls")) {
                    ArrayList<String> unseenEndOfQueueButtonNames =
                            new ArrayList<String>(Arrays.asList("NEXT", "PREVIOUS"));
                    JSONArray controls = playerInfo.getJSONArray("controls");
                    for (int j = 0; j < controls.length(); j++) {
                        JSONObject control = controls.getJSONObject(j);
                        if (control.getString("type").equals("BUTTON")) {
                            final boolean enabled = control.getBoolean("enabled");
                            final String name = control.getString("name");
                            mPlaybackController.updateControlButton(name, enabled);
                            unseenEndOfQueueButtonNames.remove(name);
                        } else if (control.getString("type").equals("TOGGLE")) {
                            final boolean selected = control.getBoolean("selected");
                            final boolean enabled = control.getBoolean("enabled");
                            final String name = control.getString("name");
                            mPlaybackController.updateControlToggle(name, enabled, selected);
                        }
                    }
                    for (String name : unseenEndOfQueueButtonNames) {
                        mPlaybackController.updateControlButton(name, false);
                    }
                }

                String title = content.has("title") ? content.getString("title") : "";
                String artist = content.has("titleSubtext1") ? content.getString("titleSubtext1") : "";
                mPlaybackController.setPlayerInfo(title, artist, providerName);

                // Get the media length in milliseconds (i.e. duration)
                long mediaLengthInMilliseconds = content.has("mediaLengthInMilliseconds")
                        ? content.getInt("mediaLengthInMilliseconds")
                        : AudioOutput.TIME_UNKNOWN;

                mDelay = 0;
                mDuration = mediaLengthInMilliseconds;
                if (mDuration == AudioOutput.TIME_UNKNOWN) {
                    mPosition = AudioOutput.TIME_UNKNOWN;
                } else {
                    mPosition = offset;
                }

                mPlaybackController.setTime(mPosition, mDuration);
                mAudioPlayerStateHandler.sendEmptyMessage(audioPlayerState.ordinal());
            }

            // Log only if audio item has changed
            if (!audioItemId.equals(mCurrentAudioItemId)) {
                mCurrentAudioItemId = audioItemId;

                // Log payload
                mLogger.postJSONTemplate(sTag, playerInfo.toString(4));

                // Log card
                mLogger.postDisplayCard(content, LogRecyclerViewAdapter.RENDER_PLAYER_INFO);

            } else {
                mLogger.postJSONTemplate(sTag, playerInfo.toString(4));
            }
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void clearTemplate() {
        // Handle dismissing display card here
        mLogger.postInfo(sTag, "handle clearTemplate()");
    }

    @Override
    public void clearPlayerInfo() {
        mLogger.postInfo(sTag, "handle clearPlayerInfo()");
        if (mPlaybackController != null) {
            mPlaybackController.setPlayerInfo("", "", "");
            mPlaybackController.hidePlayerInfoControls();
        }
    }

    //
    // ProgressHandler
    //

    static private int UPDATE_PROGRESS = Integer.MAX_VALUE;

    private class AudioPlayerStateHandler extends Handler {
        AudioPlayerStateHandler() {}

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == UPDATE_PROGRESS) {
                mPlaybackController.setTime(mPosition, mDuration);

                if (mDelay > 0) {
                    mPosition += mDelay;
                    if (mPosition > mDuration) {
                        mPosition = mDuration;
                    }
                }
                mDelay = 1000 - (mPosition % 1000);
                sendEmptyMessageDelayed(UPDATE_PROGRESS, mDelay);
            } else if (msg.what == TemplateRuntime.PlayerActivity.PAUSED.ordinal()) {
                removeMessages(UPDATE_PROGRESS);
            } else if (msg.what == TemplateRuntime.PlayerActivity.PLAYING.ordinal()) {
                removeMessages(UPDATE_PROGRESS);
                mPlaybackController.startWithButtonsEnabled(false);
                sendEmptyMessage(UPDATE_PROGRESS);
            } else if (msg.what == TemplateRuntime.PlayerActivity.STOPPED.ordinal()) {
                mPlaybackController.stopWithButtonsEnabled(false);
                removeMessages(UPDATE_PROGRESS);
            } else if (msg.what == TemplateRuntime.PlayerActivity.FINISHED.ordinal()) {
                mPlaybackController.reset();
                removeMessages(UPDATE_PROGRESS);
            }
        }
    }
}
