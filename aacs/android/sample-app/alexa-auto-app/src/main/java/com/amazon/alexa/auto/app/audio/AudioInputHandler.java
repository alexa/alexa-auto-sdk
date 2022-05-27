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
package com.amazon.alexa.auto.app.audio;

import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsipc.AACSReceiver;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * One of the workers of {@link AudioIOService}.
 *
 * The class receives audio output stream from AACS and write the microphone
 * audio input that it fetches from {@link AudioInputReader}.
 */
public class AudioInputHandler implements AudioIOServiceWorker, AACSReceiver.FetchStreamCallback {
    private static final String TAG = AudioInputHandler.class.getSimpleName();

    @NonNull
    private final AudioInputReader mAudioInputReader;

    @NonNull
    private final BehaviorSubject<Integer> mWorkerStateSubject;
    @NonNull
    private final ConcurrentHashMap<String, ParcelFileDescriptor.AutoCloseOutputStream> mAudioStreams;
    @NonNull
    private final Handler mMainThreadHandler;

    public AudioInputHandler(@NonNull AudioInputReader reader) {
        mAudioInputReader = reader;

        mWorkerStateSubject = BehaviorSubject.create();
        mAudioStreams = new ConcurrentHashMap<String, ParcelFileDescriptor.AutoCloseOutputStream>();
        mMainThreadHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    @NonNull
    public Observable<Integer> getWorkerState() {
        return mWorkerStateSubject;
    }

    /**
     * Received the stream from AACS, where we will be writing the audio input that
     * we read from microphone.
     *
     * We can have more than one active streams listening for audio input but always
     * one per audio type like VOICE, COMMS etc.
     *
     * Since on generic android devices we can only have one client that can read
     * from microphone, so targets of all audio types like VOICE, COMMS etc. can
     * only be one service. On platforms where multiple microphones input can be
     * read, we can configure AACS to send the reading request to multiple services.
     *
     * @param streamId Id of the stream.
     * @param writePipe Stream where audio input will be written.
     */
    @Override
    public void onStreamRequested(String streamId, ParcelFileDescriptor writePipe) {
        Log.d(TAG, "Stream received for writing audio input:" + streamId);

        ParcelFileDescriptor.AutoCloseOutputStream stream = toOutputStream(writePipe);
        mAudioStreams.put(streamId, stream);
        ensureAudioInputCaptureStarted();
    }

    /**
     * Stop writing audio input to stream that was sent earlier with
     * {@link #onStreamRequested)}.
     *
     * @param streamId Id of the stream.
     */
    @Override
    public void onStreamFetchCancelled(String streamId) {
        Log.d(TAG, "Stream for writing audio input removed:" + streamId);
        ParcelFileDescriptor.AutoCloseOutputStream stream = mAudioStreams.get(streamId);
        if (stream != null) {
            mAudioStreams.remove(streamId);

            try {
                stream.close();
            } catch (IOException exception) {
                // Ignore exception.
            }
        }
        checkAndStopAudioInputCaptureIfRequired();
    }

    /**
     * Start capturing the audio input if not already capturing.
     */
    private void ensureAudioInputCaptureStarted() {
        Preconditions.checkMainThread();

        if (!mAudioInputReader.isAudioCaptureStarted()) {
            mWorkerStateSubject.onNext(AudioIOServiceWorker.WORKING);

            final List<String> erroredStreams = new ArrayList<>();
            mAudioInputReader.startInputCapture((audioBytes, length) -> {
                erroredStreams.clear();
                mAudioStreams.entrySet().forEach(stream -> {
                    try {
                        stream.getValue().write(audioBytes, 0, length);
                    } catch (IOException exception) {
                        Log.w(TAG, "Failed to write on stream: " + stream.getKey() + " Error: " + exception);
                        erroredStreams.add(stream.getKey());
                    }
                });

                if (erroredStreams.size() > 0) {
                    removeErroredStreams(erroredStreams);
                }
            });
        }
    }

    /**
     * Check if audio capture should stop and stop it if required.
     */
    private void checkAndStopAudioInputCaptureIfRequired() {
        Preconditions.checkMainThread();

        if (mAudioStreams.size() == 0) {
            mAudioInputReader.stopInputCapture();
            mWorkerStateSubject.onNext(AudioIOServiceWorker.IDLE);
        }
    }

    /**
     * Remove errored streams from the map so that we don't try to write
     * into them in next run.
     *
     * @param erroredStreams Streams which have errored.
     */
    private void removeErroredStreams(@NonNull List<String> erroredStreams) {
        Log.w(TAG, "Removing failed streams");
        erroredStreams.forEach(this::onStreamFetchCancelled);
        mMainThreadHandler.post(this::checkAndStopAudioInputCaptureIfRequired);
    }

    /**
     * Converts the passed pipe handle to output stream.
     *
     * @param writePipe Handle to pipe.
     * @return An instance of {@link ParcelFileDescriptor.AutoCloseOutputStream}.
     */
    @VisibleForTesting
    ParcelFileDescriptor.AutoCloseOutputStream toOutputStream(@NonNull ParcelFileDescriptor writePipe) {
        return new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);
    }
}
