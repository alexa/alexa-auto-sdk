/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.modules.audioOutput.mediaPlayer.exo;

import java.util.HashMap;
import java.util.Map;

/**
 * This class maintains the status of various audio types like ALARM, MUSIC, NOTIFICATION
 * TTS, EARCON, RINGTONE and COMMUNICATION
 */
class AudioFocusStatus {
    private static final Map<String, Boolean> mChannelFocusStatusMap = new HashMap<>();

    /**
     * Provides the audio focus status of the audio type
     * @param type Audio type from ALARM, MUSIC, NOTIFICATION, TTS, EARCON, RINGTONE and COMMUNICATION
     * @return boolean status. True means has the full audio focus, otherwise False
     */
    static synchronized boolean getAudioFocusStatus(String type) {
        if (mChannelFocusStatusMap.containsKey(type)) {
            return mChannelFocusStatusMap.get(type).booleanValue();
        } else {
            return false;
        }
    }

    /**
     * Set Audio focus of the given audio type
     * @param type Audio type from ALARM, MUSIC, NOTIFICATION, TTS, EARCON, RINGTONE and COMMUNICATION
     * @param status boolean status. True means has the full audio focus, otherwise False
     */
    static synchronized void setAudioFocusStatus(String type, boolean status) {
        mChannelFocusStatusMap.put(type, status);
    }
}
