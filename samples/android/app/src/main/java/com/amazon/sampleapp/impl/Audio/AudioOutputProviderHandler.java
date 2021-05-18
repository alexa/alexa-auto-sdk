/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Audio;

import android.app.Activity;
import android.media.AudioManager;

import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.sampleapp.core.AuthStateObserver;
import com.amazon.sampleapp.impl.AlexaClient.AlexaClientHandler;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.HashMap;

public class AudioOutputProviderHandler extends AudioOutputProvider {
    private static final String sTag = AudioOutputProviderHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AlexaClientHandler mAlexaClientHandler;
    private EqualizerControllerHandler mEqualizerControllerHandler;

    private HashMap<String, AudioOutput> mAudioOutputMap;

    public AudioOutputProviderHandler(Activity activity, LoggerHandler logger, AlexaClientHandler alexaClientHandler,
            EqualizerControllerHandler equalizerControllerHandler) {
        mActivity = activity;
        mLogger = logger;
        mAlexaClientHandler = alexaClientHandler;
        mAudioOutputMap = new HashMap<>();
        mEqualizerControllerHandler = equalizerControllerHandler;
    }

    public AudioOutput getOutputChannel(String name) {
        return mAudioOutputMap.containsKey(name) ? mAudioOutputMap.get(name) : null;
    }

    @Override
    public AudioOutput openChannel(String name, AudioOutputType type) {
        mLogger.postInfo(sTag, String.format("openChannel[name=%s,type=%s]", name, type.toString()));

        UnifiedAudioOutput audioOutput = new UnifiedAudioOutput(
                mActivity.getApplicationContext(), mLogger, name, type, mEqualizerControllerHandler);
        mAlexaClientHandler.registerAuthStateObserver(audioOutput);

        mAudioOutputMap.put(name, audioOutput);
        return audioOutput;
    }
}
