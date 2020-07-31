/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import com.amazon.aace.audio.AudioInput;
import com.amazon.aace.audio.AudioInputProvider;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class AudioInputProviderHandler extends AudioInputProvider {
    private static final String sTag = "AudioInputProviderHandler";

    private AudioInput mDefaultAudioInput = null;
    private final Activity mActivity;
    private final LoggerHandler mLogger;

    public AudioInputProviderHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
    }

    @Override
    public AudioInput openChannel(String name, AudioInputType type) {
        mLogger.postInfo(sTag, String.format("openChannel() for type %s", type));

        if (type == AudioInputType.VOICE || type == AudioInputType.COMMUNICATION) {
            return getDefaultAudioInput();
        } else {
            return null;
        }
    }

    private AudioInput getDefaultAudioInput() {
        if (mDefaultAudioInput == null) {
            mDefaultAudioInput = new AudioInputHandler(mActivity, mLogger);
        }
        return mDefaultAudioInput;
    }
}
