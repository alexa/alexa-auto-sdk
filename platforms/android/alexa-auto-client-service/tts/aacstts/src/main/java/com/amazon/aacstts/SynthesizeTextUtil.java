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

package com.amazon.aacstts;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;
import static com.amazon.aacstts.TTSConstants.DEFAULT_PROVIDER;
import static com.amazon.aacstts.models.TTSSynthesisFutureResponse.FutureType.WAIT_FOR_REPLY;
import static com.amazon.aacstts.models.TTSSynthesisFutureResponse.FutureType.WAIT_FOR_STREAM;

import android.content.Context;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.ParcelFileDescriptor;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.TextToSpeech;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.AACSSender.StreamFetchedFromReceiverCallback;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.aacstts.models.GetCapabilitiesPayload;
import com.amazon.aacstts.models.PrepareSpeechMessagePayload;
import com.amazon.aacstts.models.TTSSynthesisFutureResponse;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Utility class to handle sending and handling the prepare speech request.
 */
public class SynthesizeTextUtil {
    private static final String TAG = AACS_TTS_LOG_PREFIX + SynthesizeTextUtil.class.getSimpleName();

    @VisibleForTesting
    protected static final String PAYLOAD_SPEECH_ID_KEY = "speechId";
    @VisibleForTesting
    protected static final String PAYLOAD_STREAM_ID_KEY = "streamId";
    @VisibleForTesting
    protected static final String PAYLOAD_ENCODING_KEY = "encoding";

    @VisibleForTesting
    protected Map<String, TTSSynthesisFutureResponse> mMessageIdToFutureMap;

    @VisibleForTesting
    protected boolean mIsStreamingEnabled;

    private static final int DEFAULT_PROVIDER_SAMPLE_RATE_IN_HZ = 24000;
    private static final int DEFAULT_PROVIDER_AUDIO_CHANNEL_COUNT = 2;
    private static final int THREAD_JOIN_TIMEOUT_IN_MILLIS = 2000;

    // This value is used for setting the size of the byte_array while reading the data from the stream.
    // If the byte array is decoded to PCM, the size of the bytes to write is inflated 8 times by the MediaCodec.
    // To avoid an overflow in the audioAvailable method the byte-array length should be selected low enough
    // so that it would not exceed the max-buffer-size that is permitted by the synthesisCallback.
    // Note: We have also employed chunking technique to handle the buffer
    //       overflows in the audioAvailable method.
    private static final int STREAM_READ_BYTE_ARRAY_SIZE = 144;

    private static final String AUDIO_FORMAT_MP3 = "MP3";
    private static final String AUDIO_FORMAT_PCM = "PCM";

    private static Set<String> SUPPORTED_AUDIO_FORMATS;
    static {
        Set<String> mSupportedEncodings = new HashSet<>();
        mSupportedEncodings.add(AUDIO_FORMAT_MP3);
        mSupportedEncodings.add(AUDIO_FORMAT_PCM);

        SUPPORTED_AUDIO_FORMATS = Collections.unmodifiableSet(mSupportedEncodings);
    }

    private AACSSender mAACSSender;
    private TargetComponent mTarget;
    private Context mContext;
    private HandlerThread mFetchAudioHandlerThread;
    private Handler mFetchAudioHandler;

    /**
     * Prepares the utility class before it can accept any Synthesis request.
     *
     * @param aacsSender - the {@link AACSSender} used to publish the prepare speech request.
     * @param target     - the {@link TargetComponent} that need to be passed in the request to AACS.
     * @param context    - the {@link Context} - Application Context.
     */
    public SynthesizeTextUtil(AACSSender aacsSender, TargetComponent target, Context context) {
        mIsStreamingEnabled = true;
        mAACSSender = aacsSender;
        mTarget = target;
        mContext = context;
        mMessageIdToFutureMap = new HashMap<>();
        mFetchAudioHandlerThread = new HandlerThread("FetchAudioHandlerThread");
        mFetchAudioHandlerThread.start();
        mFetchAudioHandler = new Handler(mFetchAudioHandlerThread.getLooper());
    }

    /**
     * Builds the AASB message and handles the Synthesizing of the request
     *
     * @param networkTimeout - the maximum timeout the synthesis request should be honoured.
     * @param payload        - the {@link PrepareSpeechMessagePayload} payload to be bundled in the Prepare speech
     *         request.
     * @param topic          - the Topic to be sent this request to.
     * @param action         - the action to be performed with this request.
     * @param callback       - the {@link SynthesisCallback} that would be used to handle the stream returned in
     *         response to the Prepare speech request.
     */
    public void sendPrepareSpeechMessage(int networkTimeout, PrepareSpeechMessagePayload payload, String topic,
            String action, SynthesisCallback callback) {
        Optional<String> message = buildMessage(topic, action, payload);

        if (!message.isPresent()) {
            String error = String.format("The construction of AASB message "
                            + "failed for synthesisRequest for action: [%s] and payload: [%s]",
                    TTSConstants.Action.PREPARE_SPEECH, payload);
            Log.e(TAG, error);
            return;
        }

        mIsStreamingEnabled = true;

        Log.d(TAG,
                String.format(
                        "Sending %s.%s message with payload [%s] and expecting reply.", topic, action, message.get()));
        mAACSSender.sendAASBMessageAnySize(message.get(), action, topic, mTarget, mContext);

        CompletableFuture<Object> waitForReply = new CompletableFuture<>();
        CompletableFuture<Object> waitForStream = new CompletableFuture<>();

        TTSSynthesisFutureResponse ttsSynthesisFutureResponse =
                new TTSSynthesisFutureResponse(waitForReply, waitForStream, callback, mAACSSender, mTarget, mContext);

        mMessageIdToFutureMap.put(payload.getSpeechId(), ttsSynthesisFutureResponse);

        try {
            waitForReply.get(networkTimeout, TimeUnit.MILLISECONDS);
            Log.d(TAG,
                    String.format(
                            "PrepareSpeechCompleted message was received for speechId=%s", payload.getSpeechId()));
        } catch (CancellationException | ExecutionException | InterruptedException e) {
            Log.d(TAG, "Error occurred during wait task execution: " + e.getMessage());
            callback.error(TextToSpeech.ERROR_SYNTHESIS);
            removeSpeechIdFromMap(payload.getSpeechId());
            return;
        } catch (TimeoutException e) {
            Log.d(TAG, "Stopping wait for speechId : " + payload.getSpeechId());
            callback.error(TextToSpeech.ERROR_NETWORK_TIMEOUT);
            removeSpeechIdFromMap(payload.getSpeechId());
            return;
        }

        try {
            waitForStream.get();
            Log.d(TAG, String.format("Stream was fetched for speechId=%s", payload.getSpeechId()));
        } catch (CancellationException | ExecutionException | InterruptedException e) {
            Log.d(TAG, "Error occurred during wait task execution: " + e.getMessage());
            callback.error(TextToSpeech.ERROR_SYNTHESIS);
        }
        removeSpeechIdFromMap(payload.getSpeechId());
        return;
    }

    private void removeSpeechIdFromMap(String speechId) {
        if (mMessageIdToFutureMap.containsKey(speechId)) {
            mMessageIdToFutureMap.remove(speechId);
        }
    }

    /**
     * Handles the prepare speech response
     *
     * @param payload - the payload returned by the Prepare Speech request.
     */
    public void handlePrepareSpeechResponse(String payload) {
        try {
            JSONObject obj = new JSONObject(payload);

            if (!obj.has(PAYLOAD_SPEECH_ID_KEY)) {
                String errorMsg = String.format("The payload:[%s] did not consist of any speechId", payload);
                Log.e(TAG, errorMsg);
                return;
            }

            String speechId = obj.getString(PAYLOAD_SPEECH_ID_KEY);
            TTSSynthesisFutureResponse ttsSynthesisFutureResponse = mMessageIdToFutureMap.getOrDefault(speechId, null);

            if (ttsSynthesisFutureResponse == null) {
                // The response might have timed out, so a response contains an outdated streamId.
                if (obj.has(PAYLOAD_STREAM_ID_KEY)) {
                    String streamId = obj.getString(PAYLOAD_STREAM_ID_KEY);
                    Log.w(TAG,
                            String.format(
                                    "The request for streamId %s was dropped as the request was timed out.", streamId));
                    mAACSSender.cancelFetch(streamId, mTarget, mContext);
                    return;
                }
                Log.e(TAG, String.format("No waiting thread was found for speechId:[%s]", speechId));
                return;
            }

            SynthesisCallback synthesisCallback = ttsSynthesisFutureResponse.getSynthesisCallback();
            if (!obj.has(PAYLOAD_STREAM_ID_KEY)) {
                Log.e(TAG, "The Synthesis request for AACS failed with error payload" + payload);
                ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_REPLY);
                sendGetCapabilitiesAACSMessage(ttsSynthesisFutureResponse);
                return;
            }

            String streamId = obj.getString(PAYLOAD_STREAM_ID_KEY);
            if (!mIsStreamingEnabled) {
                ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_REPLY);
                mAACSSender.cancelFetch(streamId, mTarget, mContext);
                return;
            }

            String audioFormat = obj.optString(PAYLOAD_ENCODING_KEY);
            if (!SUPPORTED_AUDIO_FORMATS.contains(audioFormat)) {
                Log.e(TAG, String.format("The Audio [%s] format is not supported", audioFormat));
                ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_REPLY);
                mAACSSender.cancelFetch(streamId, mTarget, mContext);
                return;
            }

            boolean requiresMP3Encoding = AUDIO_FORMAT_MP3.equals(audioFormat);

            // Construct a Callback to handle streaming of the audio.
            StreamFetchedFromReceiverCallback streamCallback = readPipe -> {
                mFetchAudioHandler.post(() -> {
                    Log.d(TAG, "onStreamFetchedFromServer: callback triggered.");

                    ParcelFileDescriptor.AutoCloseInputStream stream =
                            new ParcelFileDescriptor.AutoCloseInputStream(readPipe);
                    try (ParcelFileDescriptor.AutoCloseInputStream inputStream = stream) {
                        AudioDecoder decoder = null;
                        if (requiresMP3Encoding) {
                            Optional<AudioDecoder> audioDecoderOptional =
                                    constructAudioDecoderForMP3(synthesisCallback);
                            if (audioDecoderOptional.isPresent()) {
                                decoder = audioDecoderOptional.get();
                            } else {
                                stream.close();
                                ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_STREAM);
                                return;
                            }
                        }

                        boolean isReading = true;

                        while (isReading && mIsStreamingEnabled) {
                            byte[] byteArray = new byte[STREAM_READ_BYTE_ARRAY_SIZE];
                            int bytesAvailable = Math.max(1, Math.min(inputStream.available(), byteArray.length));
                            int bytesRead = inputStream.read(byteArray, 0, bytesAvailable);
                            Log.v(TAG, String.format(" reading [%s] bytes", bytesRead));
                            byte[] bytesToBeWritten;
                            if (requiresMP3Encoding) {
                                bytesToBeWritten = decoder.decode(byteArray, bytesRead);
                            } else {
                                bytesToBeWritten = byteArray;
                            }
                            if (bytesToBeWritten != null && bytesToBeWritten.length > 0) {
                                Log.d(TAG,
                                        String.format("Bytes to be written: [%s], bytes read: [%s]",
                                                bytesToBeWritten.length, bytesRead));
                                int offset = 0;
                                while (offset < bytesToBeWritten.length) {
                                    int bytesToWrite = Math.min(
                                            synthesisCallback.getMaxBufferSize(), bytesToBeWritten.length - offset);
                                    Log.d(TAG, String.format("offset:[%s] and writing: [%s]", offset, bytesToWrite));
                                    synthesisCallback.audioAvailable(bytesToBeWritten, offset, bytesToWrite);
                                    offset += bytesToWrite;
                                }
                            }

                            if (bytesRead < 0) {
                                isReading = false;
                            }

                            // cancel fetching from the stream.
                            if (!mIsStreamingEnabled) {
                                mAACSSender.cancelFetch(streamId, mTarget, mContext);
                                ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_STREAM);
                                return;
                            }
                        }
                        closePipe(readPipe);
                        ttsSynthesisFutureResponse.completeFuture(WAIT_FOR_STREAM);
                    } catch (IOException e) {
                        Log.e(TAG, "onStreamFetchedFromServer: FAILED to read from stream.", e);
                        closePipe(readPipe);
                        ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_STREAM);
                    }
                });
            };
            mAACSSender.fetch(streamId, streamCallback, mTarget, mContext);
            ttsSynthesisFutureResponse.completeFuture(WAIT_FOR_REPLY);
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error occurred while trying to parse payload: %s", payload));
        }
    }

    private void sendGetCapabilitiesAACSMessage(TTSSynthesisFutureResponse ttsSynthesisFutureResponse) {
        // Load the capabilities of the TextToSpeech and
        // update the cache with the supported languages.
        GetCapabilitiesPayload getCapabilitiesPayload = new GetCapabilitiesPayload();
        getCapabilitiesPayload.setProvider(DEFAULT_PROVIDER);

        Optional<String> message = AACSMessageBuilder.buildMessage(
                TTSConstants.TOPIC, TTSConstants.Action.GET_CAPABILITIES, getCapabilitiesPayload.toJsonString());

        if (message.isPresent()) {
            AACSSender aacsSender = ttsSynthesisFutureResponse.getAACSSender();
            TargetComponent target = ttsSynthesisFutureResponse.getTargetComponent();
            Context context = ttsSynthesisFutureResponse.getContext();
            try {
                aacsSender.sendAASBMessageAnySize(
                        message.get(), TTSConstants.Action.GET_CAPABILITIES, TTSConstants.TOPIC, target, context);
            } catch (Exception e) {
                String errorMsg = String.format("Error occurred while "
                                + "trying to send %s.%s message with payload %s",
                        TTSConstants.TOPIC, TTSConstants.Action.GET_CAPABILITIES,
                        getCapabilitiesPayload.toJsonString());
                Log.e(TAG, errorMsg, e);
            }
        }
    }

    /**
     * Clean up the utility class map and cancel all the futures that were not done or cancelled.
     * <p>
     * This method needs to be synchronized as this function can be called from multiple threads.
     */
    public synchronized void cleanUp() {
        mIsStreamingEnabled = false;
        mAACSSender = null;
        mTarget = null;
        mContext = null;

        for (Map.Entry<String, TTSSynthesisFutureResponse> entry : mMessageIdToFutureMap.entrySet()) {
            TTSSynthesisFutureResponse ttsSynthesisFutureResponse = entry.getValue();
            ttsSynthesisFutureResponse.cancelFuture(WAIT_FOR_REPLY);
            mMessageIdToFutureMap.remove(entry.getKey());
        }

        if (mFetchAudioHandlerThread != null) {
            mFetchAudioHandlerThread.quitSafely();
            try {
                mFetchAudioHandlerThread.join(THREAD_JOIN_TIMEOUT_IN_MILLIS);
                mFetchAudioHandlerThread = null;
            } catch (InterruptedException e) {
                Log.e(TAG, "unable to finish EngineHandler thread: " + e);
            }
        }
    }

    /**
     * Constructs the {@link AudioDecoder} to decode MP3 data.
     * @param synthesisCallback - to be called when the format of data is changed.
     * @return {@link AudioDecoder}
     */
    private Optional<AudioDecoder> constructAudioDecoderForMP3(SynthesisCallback synthesisCallback) {
        AudioDecoder decoder = new AudioDecoder();

        AudioDecoder.FormatChangedCallback formatChangedCallback = new AudioDecoder.FormatChangedCallback() {
            @Override
            public void onOutputFormatChanged(int sampleRateInHz, int audioFormat, int channelCount) {
                Log.i(TAG,
                        String.format(
                                "AudioDecoder.onOutputFormatChanged: sampleRate=%s, audioFormat=%s, channelCount=%s.",
                                sampleRateInHz, audioFormat, channelCount));
                synthesisCallback.start(sampleRateInHz, audioFormat, channelCount);
            }
        };

        if (!decoder.configureDecoder(MediaFormat.MIMETYPE_AUDIO_MPEG, DEFAULT_PROVIDER_SAMPLE_RATE_IN_HZ,
                    DEFAULT_PROVIDER_AUDIO_CHANNEL_COUNT, formatChangedCallback)) {
            Log.e(TAG, "Decoder could not be configured.");
            return Optional.empty();
        }

        if (!decoder.start()) {
            Log.e(TAG, "Decoder could not start up.");
            return Optional.empty();
        }
        return Optional.of(decoder);
    }

    /**
     * Stop any ongoing synthesis.
     */
    public synchronized void stopSynthesis() {
        mIsStreamingEnabled = false;
    }

    private Optional<String> buildMessage(String topic, String action, PrepareSpeechMessagePayload payload) {
        Optional<String> message;
        try {
            message = AACSMessageBuilder.buildMessage(topic, action, payload.toJsonString());
            return message;
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct the message payload", e);
            return Optional.empty();
        }
    }

    private void closePipe(ParcelFileDescriptor pipe) {
        if (pipe != null) {
            try {
                pipe.close();
            } catch (IOException e) {
                Log.e(TAG, "Failed to close pipe");
            }
        }
    }
}