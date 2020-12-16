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
package com.amazon.alexaautoclientservice.modules.audioInput;

import static com.amazon.alexaautoclientservice.util.FileUtil.getAudioExternalSourceForAudioType;
import static com.amazon.alexaautoclientservice.util.FileUtil.getAudioSourceForAudioType;

import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.amazon.aace.aasb.AASBStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.AASBHandler;
import com.amazon.alexaautoclientservice.ComponentRegistry;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONObject;

import java.io.IOException;
import java.util.HashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

public class AudioInputMessageHandler {
    private static final String TAG = AACSConstants.AACS + "-" + AudioInputMessageHandler.class.getSimpleName();
    private static final int FETCH_READ_BUFFER_MIN_CHUNK_SIZE = 20;
    private static final int SAMPLE_RATE_HZ = 16000;
    private static final int AUDIO_RECORD_BUFFER_SIZE = 1024;
    private static final int AUDIO_READER_BUFFER_SIZE = 300;

    private Context mContext;
    private AudioRecord mAudioInput;
    private AASBHandler mAASBHandler;
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(1);
    private final ExecutorService mExternalExecutor = Executors.newFixedThreadPool(2);
    private AACSSender mAACSSender;
    private TargetComponent mCommsExternalSourceTarget;
    private TargetComponent mVoiceExternalSourceTarget;
    private AASBStream mCommsStream;
    private AASBStream mVoiceStream;
    private AudioReader mReader;
    private HashMap<String, String> mStreamIdToTypeMap;
    private HashMap<String, AudioReader> mStreamIdToReaderMap; // EXTERNAL case only

    public AudioInputMessageHandler(@NonNull Context context, @NonNull AACSSender aacsSender) {
        mContext = context;
        mAACSSender = aacsSender;
        mStreamIdToTypeMap = new HashMap<>();
        mStreamIdToReaderMap = new HashMap<>();
        mCommsExternalSourceTarget = null;
        mVoiceExternalSourceTarget = null;
    }

    public void handleAudioInputMessage(@NonNull String messageId, @NonNull String topic, @NonNull String action,
            @NonNull String payload, @NonNull AASBHandler aasbHandler) {
        Log.d(TAG, "handleAudioInputMessage " + action + payload);
        mAASBHandler = aasbHandler;
        JSONObject payloadJson;
        String audioType = "";
        String streamId = "";
        if (!payload.isEmpty()) {
            try {
                payloadJson = new JSONObject(payload);
                streamId = payloadJson.getString(AASBConstants.AudioInput.STREAM_ID);
                if (payloadJson.has(AASBConstants.AudioInput.TYPE)) {
                    audioType = payloadJson.getString(AASBConstants.AudioInput.TYPE);
                }
            } catch (Exception e) {
                Log.e(TAG, String.format("Failed to parse payload. Error=%s", e.getMessage()));
            }
        }
        switch (action) {
            case Action.AudioInput.START_AUDIO_INPUT:
                handleStartAudioInput(audioType, streamId);
                break;
            case Action.AudioInput.STOP_AUDIO_INPUT:
                handleStopAudioInput(streamId);
                break;
            default:
                Log.w(TAG, action + "message not being handled.");
        }
    }

    private void fetchExternalAudioSource(String audioType, String streamId, AudioReader reader) {
        TargetComponent externalSourceTarget = getExternalSourceTargetByType(audioType);
        if (externalSourceTarget != null) {
            Log.d(TAG,
                    String.format("fetchExternalAudioSource: starting fetching audio from %s",
                            externalSourceTarget.packageName));
            mAACSSender.fetch(streamId, reader, externalSourceTarget, mContext);
        } else {
            Log.e(TAG, "fetchExternalAudioSource: fetching audio failed because the externalSource was not valid");
        }
    }

    private void handleStartAudioInput(String audioType, String streamId) {
        if (FileUtil.isAudioSourceExternal(mContext, audioType)) {
            Log.d(TAG, "handleStartAudioInput: use external audio source.");
            AudioReader reader = setupStreamsAndReader(audioType, streamId, AudioSourceType.EXTERNAL);
            fetchExternalAudioSource(audioType, streamId, reader);
            return;
        }

        Log.d(TAG, "handleStartAudioInput: use internal audio source.");
        if (mAudioInput == null) {
            Log.d(TAG, "Creating AudioRecorder");
            mAudioInput = createAudioInput(audioType);
        }

        if (mAudioInput.getState() != AudioRecord.STATE_INITIALIZED) {
            if (ActivityCompat.checkSelfPermission(mContext, android.Manifest.permission.RECORD_AUDIO)
                    == PackageManager.PERMISSION_DENIED) {
                Log.e(TAG, "Cannot start audio input. Microphone permission not granted");
                return;
            }
        }
        startRecording(audioType, streamId);
    }

    private void startRecording(String audioType, String streamId) {
        Log.i(TAG, "startRecording() ");
        try {
            mAudioInput.startRecording();
        } catch (IllegalStateException e) {
            Log.e(TAG, "AudioRecord cannot start recording. Error: " + e.getMessage());
            return;
        }
        // Read recorded audio samples and pass to engine
        try {
            setupStreamsAndReader(audioType, streamId, AudioSourceType.INTERNAL);
        } catch (RejectedExecutionException e) {
            Log.e(TAG, "Audio reader task cannot be scheduled for execution. Error: " + e.getMessage());
        }
    }

    private AudioReader setupStreamsAndReader(String audioType, String streamId, AudioSourceType sourceType) {
        if (audioType.equals(AASBConstants.AudioInput.AudioType.COMMUNICATION) && mCommsStream == null) {
            mCommsStream = mAASBHandler.openStream(streamId, AASBStream.Mode.WRITE);
        } else if (audioType.equals(AASBConstants.AudioInput.AudioType.VOICE) && mVoiceStream == null) {
            mVoiceStream = mAASBHandler.openStream(streamId, AASBStream.Mode.WRITE);
        }
        mStreamIdToTypeMap.put(streamId, audioType);

        if (sourceType.equals(AudioSourceType.EXTERNAL)) {
            AudioReader reader = new AudioReader(sourceType);
            reader.setStreamId(streamId);
            mStreamIdToReaderMap.put(streamId, reader);
            return reader;
        } else {
            if (mReader == null || !mReader.isRunning()) {
                mReader = new AudioReader(sourceType);
                mExecutor.submit((Runnable) mReader);
            }
        }
        return mReader;
    }

    private AudioRecord createAudioInput(String audioType) {
        Log.d(TAG, "createAudioInput");
        AudioRecord audioRecord = null;
        try {
            audioRecord = new AudioRecord(getAudioSourceForAudioType(mContext, audioType), SAMPLE_RATE_HZ,
                    AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, AUDIO_RECORD_BUFFER_SIZE);

        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Cannot create audio input. Error: " + e.getMessage());
        }
        return audioRecord;
    }

    private TargetComponent getExternalSourceTargetByType(String audioType) {
        if (audioType.equals(AASBConstants.AudioInput.AudioType.COMMUNICATION)) {
            if (mCommsExternalSourceTarget == null) {
                createTargetComponentForExternalSource(audioType);
            }
            return mCommsExternalSourceTarget;
        } else if (audioType.equals(AASBConstants.AudioInput.AudioType.VOICE)) {
            if (mVoiceExternalSourceTarget == null) {
                createTargetComponentForExternalSource(audioType);
            }
            return mVoiceExternalSourceTarget;
        }
        return null;
    }

    private void createTargetComponentForExternalSource(String audioType) {
        JSONObject externalSourceTarget = getAudioExternalSourceForAudioType(mContext, audioType);
        if (externalSourceTarget == null) {
            Log.e(TAG,
                    String.format(
                            "External audio source for %s retrieved from configuration was not valid", audioType));
            return;
        }
        String packageName = externalSourceTarget.optString("package");
        String className = externalSourceTarget.optString("class");
        className = (className.charAt(0) == '.') ? packageName + className : className;
        String typeName = externalSourceTarget.optString("type");
        if (audioType.equals(AASBConstants.AudioInput.AudioType.COMMUNICATION)) {
            mCommsExternalSourceTarget = TargetComponent.withComponent(
                    new ComponentName(packageName, className), ComponentRegistry.getTargetType(typeName));
        } else if (audioType.equals(AASBConstants.AudioInput.AudioType.VOICE)) {
            mVoiceExternalSourceTarget = TargetComponent.withComponent(
                    new ComponentName(packageName, className), ComponentRegistry.getTargetType(typeName));
        }
    }

    private void handleStopAudioInput(String streamId) {
        Log.d(TAG, "handleStopAudioInput");

        boolean noStreams = mVoiceStream == null && mCommsStream == null;
        if (mStreamIdToReaderMap.containsKey(streamId)) {
            // EXTERNAL case only
            AudioReader reader = mStreamIdToReaderMap.get(streamId);
            if (reader != null) {
                reader.cancel();
            }
            mStreamIdToReaderMap.remove(streamId);
        } else if (mReader != null && noStreams) {
            mReader.cancel();
            stopAudioRecord();
        }

        if (mStreamIdToTypeMap.containsKey(streamId)) {
            String type = mStreamIdToTypeMap.get(streamId);
            if (type != null) {
                if (type.equals(AASBConstants.AudioInput.AudioType.COMMUNICATION)) {
                    mCommsStream = null;
                } else if (type.equals(AASBConstants.AudioInput.AudioType.VOICE)) {
                    mVoiceStream = null;
                }
            }
            mStreamIdToTypeMap.remove(streamId);
        }
    }

    private void stopAudioRecord() {
        if (mAudioInput == null) {
            Log.w(TAG, "stopAudioInput() called but AudioRecord was never initialized");
            return;
        }
        try {
            mAudioInput.stop();
        } catch (IllegalStateException e) {
            Log.e(TAG, "AudioRecord cannot stop recording. Error: " + e.getMessage());
        }
    }

    public void cleanUp() {
        if (mAudioInput != null) {
            mAudioInput.release();
            mAudioInput = null;
        }

        if (mReader != null) {
            mReader.cancel();
            mReader = null;
        }

        if (mVoiceStream != null)
            mVoiceStream = null;

        if (mCommsStream != null)
            mCommsStream = null;

        mStreamIdToTypeMap.clear();
        mStreamIdToReaderMap.clear();
    }

    enum AudioSourceType { INTERNAL, EXTERNAL }

    /**
     * AudioReader class is used to buffer Audio data and send it to AASB.
     */
    private class AudioReader implements Runnable, AACSSender.StreamFetchedFromReceiverCallback {
        private boolean mRunning = true;
        private byte[] mBuffer = new byte[AUDIO_READER_BUFFER_SIZE];
        private AudioSourceType mReaderType; // INTERNAL or EXTERNAL
        private String mStreamId;

        public AudioReader(AudioSourceType readerType) {
            mReaderType = readerType;
        }

        public boolean isRunning() {
            return mRunning;
        }

        public void setStreamId(String id) {
            mStreamId = id;
        }

        @Override
        public void run() {
            Log.d(TAG, "AudioRecord run() ");
            int size;
            if (mAASBHandler == null) {
                Log.w(TAG, "mAASBHandler is null.");
                return;
            }
            while (mRunning) {
                size = mAudioInput.read(mBuffer, 0, mBuffer.length);
                if (size > 0 && mRunning) {
                    if (mCommsStream != null)
                        mCommsStream.write(mBuffer);

                    if (mVoiceStream != null)
                        mVoiceStream.write(mBuffer);
                }
            }
        }

        @Override
        public void onStreamFetchedFromServer(ParcelFileDescriptor readPipe) {
            Log.d(TAG, "AudioReader.onStreamFetchedFromServer: callback triggered.");
            ParcelFileDescriptor.AutoCloseInputStream stream = new ParcelFileDescriptor.AutoCloseInputStream(readPipe);

            mExternalExecutor.submit(() -> {
                Log.d(TAG, "AudioReader.onStreamFetchedFromServer: starts reading.");
                try (ParcelFileDescriptor.AutoCloseInputStream inputStream = stream) {
                    int bytesAvailable = Math.max(
                            FETCH_READ_BUFFER_MIN_CHUNK_SIZE, Math.min(inputStream.available(), mBuffer.length));
                    int bytesRead;
                    while (mRunning && (bytesRead = inputStream.read(mBuffer, 0, bytesAvailable)) > 0) {
                        if (mStreamId != null) {
                            String type = mStreamIdToTypeMap.get(mStreamId);
                            if (type != null) {
                                if (type.equals(AASBConstants.AudioInput.AudioType.COMMUNICATION)) {
                                    if (mCommsStream != null) {
                                        mCommsStream.write(mBuffer, 0, bytesRead);
                                    }

                                } else if (type.equals(AASBConstants.AudioInput.AudioType.VOICE)) {
                                    if (mVoiceStream != null) {
                                        mVoiceStream.write(mBuffer, 0, bytesRead);
                                    }
                                }
                            } else {
                                Log.e(TAG, "Can't find audio type that matches stream id: " + mStreamId);
                            }
                        } else {
                            Log.e(TAG, "Stream id for external reader is null!");
                        }
                    }
                    Log.d(TAG, "AudioReader.onStreamFetchedFromServer: returning");
                } catch (IOException e) {
                    Log.i(TAG, "onStreamFetchedFromServer: FAILED to read from stream." + e.toString());
                }
                if (mRunning) {
                    Log.d(TAG, "AudioReader.onStreamFetchedFromServer: IPC disconnected. Retry fetching...");
                    fetchExternalAudioSource(mStreamIdToTypeMap.get(mStreamId), mStreamId, this);
                }
            });
        }

        public void cancel() {
            mRunning = false;
            if (isExternal()) {
                String type = mStreamIdToTypeMap.get(mStreamId);
                if (type == null) {
                    Log.e(TAG, "Type cannot be found for Stream Id: " + mStreamId);
                    return;
                }
                TargetComponent externalSourceTarget = getExternalSourceTargetByType(type);
                if (externalSourceTarget != null) {
                    Log.d(TAG, "AudioReader.cancelFetch started");
                    mAACSSender.cancelFetch(mStreamId, externalSourceTarget, mContext);
                } else {
                    Log.e(TAG, "AudioReader.cancelFetch failed because the externalSource was not valid");
                }
            }
        }

        public boolean isExternal() {
            return mReaderType.equals(AudioSourceType.EXTERNAL);
        }
    }
}