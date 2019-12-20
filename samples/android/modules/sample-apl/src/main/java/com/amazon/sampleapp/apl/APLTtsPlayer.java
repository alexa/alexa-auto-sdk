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
import com.amazon.apl.android.dependencies.ITtsPlayer;
import com.amazon.apl.player.tts.TtsSpeechMarksListener;

import java.io.InputStream;
import java.net.URL;

public class APLTtsPlayer implements ITtsPlayer {

    private String sTag = "APLTtsPlayer";
    private AudioOutput channel;
    private String mSource;

    public APLTtsPlayer(AudioOutput channel) {
        this.channel = channel;
    }

    @Override
    public void setWordMarkListener(TtsSpeechMarksListener ttsSpeechMarksListener) {
        Log.i(sTag, "setWordMarkListener ");
    }

    @Override
    public void setStateChangeListener(IStateChangeListener iStateChangeListener) {
        Log.i(sTag, "setStateChangeListener ");
    }

    @Override
    public void prepare(String s, InputStream inputStream) {
        Log.i(sTag, "Prepare input stream " + s);
    }

    @Override
    public void prepare(String source, URL url) {
        mSource = source;
        Log.i(sTag, "prepare: " + mSource);
        channel.prepare(source, false);
    }

    @Override
    public void play() {
        Log.i(sTag, "play: " + mSource);
        channel.play();
    }

    @Override
    public void stop() {
        Log.i(sTag, "stop player");
        channel.stop();
    }

    @Override
    public void release() {
        Log.i(sTag, "release player");
        channel = null;
    }

    @Override
    public String getSource() {
        Log.i(sTag, "getSource: " + mSource);
        return mSource;
    }
}
