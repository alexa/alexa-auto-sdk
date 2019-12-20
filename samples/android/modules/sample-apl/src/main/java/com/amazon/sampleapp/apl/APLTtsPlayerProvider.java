/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.apl;

import android.util.Log;

import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioOutputProvider;

import com.amazon.apl.android.dependencies.ITtsPlayer;
import com.amazon.apl.android.dependencies.TtsSourceProvider;
import com.amazon.apl.android.providers.ITtsPlayerProvider;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

public class APLTtsPlayerProvider implements ITtsPlayerProvider {

    private static final String sTag = "APLTtsPlayerProvider";
    private String name = "APLTTSPlayer";
    private APLTtsPlayer mTtsPlayer;
    private AudioOutputProvider mAudioProvider;

    public APLTtsPlayerProvider(AudioOutputProvider audioProvider) {
        mAudioProvider = audioProvider;
    }

    public ITtsPlayer getPlayer() {
        if (mTtsPlayer == null) {
            Log.i(sTag, "getPlayer: Creating instance");
            try {
                AudioOutput outputChannel = mAudioProvider.openChannel(name, AudioOutputProvider.AudioOutputType.TTS);
                mTtsPlayer = new APLTtsPlayer(outputChannel);
            } catch(Exception e) {
                Log.e(sTag, "getPlayer: Failed to create instance: " + e.getMessage());
            }
        }

        return mTtsPlayer;
    }


    public void prepare(String source, TtsSourceProvider ttsSourceProvider) {
        try {
            Log.i(sTag, "prepare: " + source );
            ttsSourceProvider.onSource(new URL(source));
        } catch (MalformedURLException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    @Override
    public void onDocumentFinish() {
        if (mTtsPlayer != null) {
            mTtsPlayer.release();
            mTtsPlayer = null;
        }
    }
}
