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

package com.amazon.sampleapp.impl.Audio;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.amazon.aace.audio.AudioInput;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

public class AudioInputHandler extends AudioInput {
    private static final String sTag = "AudioInputHandler";

    // All audio input consumers expect PCM 16 data @ 16 Khz. We divide this consumption into 10 ms
    // chunks. It comes out at 160 samples every 10 ms to reach 16000 samples (in a second).
    private static final int sSamplesToCollectInOneCycle = 160;
    private static final int sBytesInEachSample = 2; // PCM 16 = 2 bytes per sample
    private static final int sSampleRateInHz = 16000; // 16 khz
    private static final int sAudioFramesInBuffer = 5; // Create large enough buffer for 5 audio frames.

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(1);

    private AudioRecord mAudioInput;
    private AudioReaderRunnable mReaderRunnable;

    public AudioInputHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mAudioInput = createAudioInput();
    }

    private AudioRecord createAudioInput() {
        AudioRecord audioRecord = null;
        try {
            int minBufferSize = AudioRecord.getMinBufferSize(
                    sSampleRateInHz, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
            int bufferSize = minBufferSize + (sAudioFramesInBuffer * sSamplesToCollectInOneCycle * sBytesInEachSample);
            audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sSampleRateInHz, AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, bufferSize);
        } catch (IllegalArgumentException e) {
            mLogger.postError(sTag, "Cannot create audio input. Error: " + e.getMessage());
        }
        return audioRecord;
    }

    @Override
    public boolean startAudioInput() {
        if (mAudioInput == null) {
            mLogger.postWarn(sTag, "Cannot start audio input. AudioRecord could not be created");
            return false;
        }

        if (mAudioInput.getState() != AudioRecord.STATE_INITIALIZED) {
            // Retry AudioRecord initialization.
            mAudioInput = createAudioInput();
            if (mAudioInput.getState() != AudioRecord.STATE_INITIALIZED) {
                mLogger.postWarn(sTag, "Cannot initialize AudioRecord");
                return false;
            }
        }

        return startRecording();
    }

    @Override
    public boolean stopAudioInput() {
        if (mAudioInput == null) {
            mLogger.postWarn(sTag, "stopAudioInput() called but AudioRecord was never initialized");
            return false;
        }

        // Cancel the audio reader and stop recording
        if (mReaderRunnable != null)
            mReaderRunnable.cancel();
        try {
            mAudioInput.stop();
        } catch (IllegalStateException e) {
            mLogger.postError(sTag, "AudioRecord cannot stop recording. Error: " + e.getMessage());
            return false;
        }

        return true;
    }

    private boolean startRecording() {
        if (mReaderRunnable != null && mReaderRunnable.isRunning()) {
            mLogger.postInfo(sTag, "startRecording() called but AudioRecorder thread is already running");
            return false;
        } else {
            // Start audio recording
            try {
                mAudioInput.startRecording();
            } catch (IllegalStateException e) {
                mLogger.postError(sTag, "AudioRecord cannot start recording. Error: " + e.getMessage());
                return false;
            }

            // Read recorded audio samples and pass to engine
            try {
                mExecutor.submit(mReaderRunnable = new AudioReaderRunnable()); // Submit the audio reader thread
            } catch (RejectedExecutionException e) {
                mLogger.postError(
                        sTag, "Audio reader task cannot be scheduled for execution. Error: " + e.getMessage());
                return false;
            }
            return true;
        }
    }

    //
    // AudioReader class
    //

    private class AudioReaderRunnable implements Runnable {
        private boolean mRunning = true;
        private byte[] mBuffer = new byte[sSamplesToCollectInOneCycle * sBytesInEachSample];

        void cancel() {
            mRunning = false;
        }

        boolean isRunning() {
            return mRunning;
        }

        @Override
        public void run() {
            int size;

            while (mRunning) {
                size = mAudioInput.read(mBuffer, 0, mBuffer.length);
                if (size > 0 && mRunning) {
                    write(mBuffer, size);
                }
            }
        }
    }
}
