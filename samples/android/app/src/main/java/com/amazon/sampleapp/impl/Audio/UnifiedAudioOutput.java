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

package com.amazon.sampleapp.impl.Audio;

import android.content.Context;
import android.media.AudioManager;
import android.os.Handler;
import android.util.Log;

import com.amazon.aace.alexa.AlexaClient;
import com.amazon.aace.audio.AudioFormat;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.aace.audio.AudioStream;
import com.amazon.sampleapp.impl.AlexaClient.AuthStateObserver;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * An AudioOutput implementation that delegates to RawAudioOutputHandler and AudioOutputHandler according to
 * specified audio format.
 */
public class UnifiedAudioOutput extends AudioOutput implements AuthStateObserver, AudioOutput.MediaStateListener {
    private final Context mContext;
    private final LoggerHandler mLogger;
    private final String mName;
    private final AudioOutputProvider.AudioOutputType mAudioOutputType;

    private Float mVolume;
    private MutedState mMutedState;

    private Handler mHandler;
    private AudioOutput mInner;
    private EqualizerControllerHandler mEqualizerControllerHandler;

    UnifiedAudioOutput(Context context, LoggerHandler logger, String name,
            AudioOutputProvider.AudioOutputType audioOutputType,
            EqualizerControllerHandler equalizerControllerHandler) {
        mContext = context;
        mLogger = logger;
        mName = name;
        mAudioOutputType = audioOutputType;
        mHandler = new Handler(); // constructor should be called by a thread with associated looper
        mEqualizerControllerHandler = equalizerControllerHandler;
    }

    private static int intoStreamType(AudioOutputProvider.AudioOutputType type) {
        switch (type) {
            case NOTIFICATION:
            case ALARM:
                return AudioManager.STREAM_NOTIFICATION;
            case RINGTONE:
                return AudioManager.STREAM_RING;
            case COMMUNICATION:
            case EARCON:
                return AudioManager.STREAM_VOICE_CALL;
            case MUSIC:
            default:
                return AudioManager.STREAM_MUSIC;
        }
    }

    @Override
    public boolean prepare(AudioStream stream, boolean repeating) {
        return prepare(stream, null, repeating);
    }

    @Override
    public boolean prepare(String url, boolean repeating) {
        return prepare(null, url, repeating);
    }

    private boolean prepare(AudioStream stream, String url, boolean repeating) {
        if (mInner instanceof Releasable) {
            ((Releasable) mInner).release();
        }
        if (stream != null && stream.getAudioFormat().getEncoding() == AudioFormat.Encoding.LPCM) {
            mInner = new RawAudioOutputHandler(mLogger, mName, intoStreamType(mAudioOutputType));
        } else {
            mInner = new AudioOutputHandler(mContext, mLogger, mName, mHandler);
        }
        mInner.setMediaStateListener(this);

        boolean ok;
        if (stream != null) {
            ok = mInner.prepare(stream, repeating);
        } else {
            ok = mInner.prepare(url, repeating);
        }
        if (ok) {
            applyVolumeSettings(mInner);
            if (mAudioOutputType == AudioOutputProvider.AudioOutputType.MUSIC) {
                mEqualizerControllerHandler.setEqualizerEventListener(
                        (EqualizerControllerHandler.EqualizerProvider) mInner);
            }
        }
        return ok;
    }

    private void applyVolumeSettings(AudioOutput audioOutput) {
        if (mVolume != null) {
            audioOutput.volumeChanged(mVolume);
        }
        if (mMutedState != null) {
            audioOutput.mutedStateChanged(mMutedState);
        }
    }

    @Override
    public boolean play() {
        return mInner.play();
    }

    @Override
    public boolean stop() {
        if (mInner != null) {
            return mInner.stop();
        }
        return true;
    }

    @Override
    public boolean pause() {
        return mInner.pause();
    }

    @Override
    public boolean resume() {
        return mInner.resume();
    }

    @Override
    public long getPosition() {
        if (mInner != null) {
            return mInner.getPosition();
        }
        return TIME_UNKNOWN;
    }

    @Override
    public boolean setPosition(long position) {
        return mInner.setPosition(position);
    }

    @Override
    public long getDuration() {
        return mInner.getDuration();
    }

    @Override
    public long getNumBytesBuffered() {
        if (mInner != null) {
            return mInner.getNumBytesBuffered();
        }
        return 0;
    }

    @Override
    public boolean volumeChanged(float volume) {
        mVolume = volume;
        if (mInner != null) {
            return mInner.volumeChanged(volume);
        }
        return true;
    }

    @Override
    public boolean mutedStateChanged(AudioOutput.MutedState state) {
        mMutedState = state;
        if (mInner != null) {
            return mInner.mutedStateChanged(state);
        }
        return true;
    }

    // AuthStateObserver

    @Override
    public void onAuthStateChanged(AlexaClient.AuthState authState, AlexaClient.AuthError authError) {
        if (mInner instanceof AuthStateObserver) {
            ((AuthStateObserver) mInner).onAuthStateChanged(authState, authError);
        }
    }

    // AudioOutput.MediaStateListener

    @Override
    public void mediaError(MediaError type, String error) {
        super.mediaError(type, error);
    }

    @Override
    public void mediaStateChanged(MediaState state) {
        super.mediaStateChanged(state);
    }
}
