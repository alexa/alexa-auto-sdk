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

package com.amazon.alexaautoclientservice;

import static java.lang.System.currentTimeMillis;

import android.util.Log;

import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;

public class AACSMessageLogger {
    private final String TAG = AACSConstants.AACS + "-" + AACSMessageLogger.class.getSimpleName();
    private HashMap<String, String> mReplyMessageIdActionMap;
    private HashSet<String> mReplyActionList;
    private HashMap<String, Long> mReplyMessageTimeMap;
    private String mFileLocation;
    public static final String FROM_ENGINE = "FROM_ENGINE";
    public static final String TO_ENGINE = "TO_ENGINE";
    private final int sTimeout = 500;
    private final SimpleDateFormat sTimeFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault());

    public AACSMessageLogger() {
        // Init HashSet to store all actions that need response
        String[] replyList = {Action.AddressBook.ADD_ADDRESS_BOOK, Action.AddressBook.REMOVE_ADDRESS_BOOK,
                Action.AlexaConnectivity.CONNECTIVITY_STATE_CHANGE, Action.AlexaConnectivity.GET_CONNECTIVITY_STATE,
                Action.AlexaConnectivity.GET_IDENTIFIER, Action.AudioOutput.GET_DURATION,
                Action.AudioOutput.GET_NUM_BYTES_BUFFERED, Action.AudioOutput.GET_POSITION,
                Action.AudioPlayer.GET_PLAYER_DURATION, Action.AudioPlayer.GET_PLAYER_POSITION,
                Action.AuthProvider.GET_AUTH_STATE, Action.AuthProvider.GET_AUTH_TOKEN,
                Action.Authorization.GET_AUTHORIZATION_DATA, Action.CBL.GET_REFRESH_TOKEN,
                Action.CarControl.ADJUST_CONTROLLER_VALUE, Action.CarControl.SET_CONTROLLER_VALUE,
                Action.EqualizerController.GET_BAND_LEVELS, Action.LocalMediaSource.GET_STATE,
                Action.LocationProvider.GET_COUNTRY, Action.LocationProvider.GET_LOCATION,
                Action.Navigation.GET_NAVIGATION_STATE, Action.NetworkInfoProvider.GET_NETWORK_STATUS,
                Action.NetworkInfoProvider.GET_WIFI_SIGNAL_STRENGTH, Action.PhoneCallController.CREATE_CALL_ID,
                Action.PropertyManager.GET_PROPERTY, Action.TextToSpeech.GET_CAPABILITIES};
        mReplyActionList = new HashSet<>();
        mReplyActionList.addAll(Arrays.asList(replyList));

        // HashMap to record MessageID and its corresponding Action
        mReplyMessageIdActionMap = new HashMap<>();

        // HashMap to record messageId with its time of receive
        mReplyMessageTimeMap = new HashMap<>();
    }

    public void setLogFileLocation(String filePath) {
        mFileLocation = filePath;
    }

    public void buffer(String direction, String topic, String action, String payload, @Nullable String messageId,
            @Nullable String replyToId) {
        JSONObject logJSON = new JSONObject();
        Date currentTime = Calendar.getInstance().getTime();
        Long currentTimeMill = currentTimeMillis();
        try {
            logJSON.put("timeStamp", sTimeFormat.format(currentTime));
            logJSON.put("MessageDirection", direction);
            logJSON.put("topic", topic);
            logJSON.put("action", action);

            // Add MessageId to hashMap when topic is expected to have a reply
            if (mReplyActionList.contains(action) && messageId != null && replyToId.isEmpty()) {
                mReplyMessageIdActionMap.put(messageId, action);
                mReplyMessageTimeMap.put(messageId, currentTimeMill);
                logJSON.put("replyExpected", true);
            } else if (mReplyActionList.contains(action) && messageId != null && !replyToId.isEmpty()) {
                // Remove MessageId and Log the ReplyToId when expected message reply received
                if (mReplyMessageIdActionMap.containsKey(replyToId) && mReplyMessageTimeMap.containsKey(replyToId)) {
                    if (currentTimeMill - mReplyMessageTimeMap.get(replyToId) <= sTimeout) {
                        logJSON.put("replyReceivedTimeout", false);
                    } else {
                        logJSON.put("replyReceivedTimeout", true);
                    }
                    logJSON.put("replyToId", replyToId);
                    mReplyMessageIdActionMap.remove(replyToId);
                    mReplyMessageTimeMap.remove(replyToId);
                }
            } else {
                logJSON.put("replyExpected", false);
            }
            if (messageId != null) {
                // Log regular messageId
                logJSON.put("messageId", messageId);
            }
            if (payload != null && !payload.isEmpty()) {
                logJSON.put(AASBConstants.PAYLOAD, new JSONObject(payload).toString());
            } else {
                logJSON.put(AASBConstants.PAYLOAD, "None");
            }

        } catch (JSONException e) {
            Log.e(TAG, "Error creating instrumentation JSON: ", e);
        }

        try {
            FileWriter logFile = new FileWriter(mFileLocation, true);
            logFile.write(logJSON.toString() + ",\n");
            logFile.close();
        } catch (IOException e) {
            Log.e(TAG, "Error Writing Log entry to file: ", e);
        }
    }
}
