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

import static android.media.MediaCodec.BUFFER_FLAG_END_OF_STREAM;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;

import android.media.AudioFormat;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.util.Log;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import java.nio.ByteBuffer;

/**
 * Wrapper class over {@link MediaCodec} responsible to decode Audio bytes.
 */
public class AudioDecoder {
    private String TAG = AACS_TTS_LOG_PREFIX + AudioDecoder.class.getSimpleName();

    private static long TIMEOUT_IN_MICROSECONDS = 10_000; // 10 milliseconds
    private static int BITS_IN_A_BYTE = 8;

    private MediaCodec mDecoder;
    private FormatChangedCallback mCallback;

    /**
     * Configuring the Audio Decoder to be used to decoding.
     * @param mimeType : The mime type of the input data. One of the mime types specified here : {@link MediaFormat}
     * @param sampleRateInHz : The sampling rate of the content.
     * @param channelCount : the audio channel count.
     * @param callback: The callback handler to specify the action to be performed when Audio Format changes.
     * @return boolean: True if the decoder was configured successfully else False.
     */
    public boolean configureDecoder(
            @NonNull String mimeType, int sampleRateInHz, int channelCount, @NonNull FormatChangedCallback callback) {
        try {
            final MediaFormat format = MediaFormat.createAudioFormat(mimeType, sampleRateInHz, channelCount);
            mDecoder = MediaCodec.createDecoderByType(mimeType);
            mDecoder.configure(format,
                    null, // surface : required only for video encoding
                    null, // crypto : required for encryted data
                    0 // 0 indicates that we are configuring to use this as decoder, whereas 1 would have indicated that
                      // we want to use this as an encoder.
            );
            mCallback = callback;
            return true;
        } catch (final Exception e) {
            mDecoder = null; // clearing the decoder as its not configured correctly.
            Log.e(TAG, "Exception occurred while trying to configure the decoder. for ");
            return false;
        }
    }

    /**
     * If the decoder is configured accurately, start the decoder process.
     * @return true if decoder is successfully started.
     */
    public boolean start() {
        if (mDecoder != null) {
            mDecoder.start();
            return true;
        }
        return false;
    }

    /**
     *  Takes in the byte array with the data that needs to be decoded.
     *
     * @param inputData - the byte array which is filled with data to be decoded.
     * @param bytesToDecode - the total amount of data to be decoded.
     * @return the byte array which consists of the decoded data.
     */
    public byte[] decode(byte[] inputData, int bytesToDecode) {
        // The decoder is not able to provide an input buffer if the
        // byte array is not in the multiples of 8.
        if (inputData.length % BITS_IN_A_BYTE != 0)
            return null;

        while (true) {
            /*
             * Loading the input data into the input buffers.
             */
            int inputBufferIndex = mDecoder.dequeueInputBuffer(TIMEOUT_IN_MICROSECONDS);
            if (inputBufferIndex >= 0) {
                mDecoder.getInputBuffer(inputBufferIndex).put(inputData);
                if (bytesToDecode < 0) {
                    mDecoder.queueInputBuffer(inputBufferIndex, 0, 0, 0, BUFFER_FLAG_END_OF_STREAM);
                } else {
                    int presentationTime = 0; // Only required if we are synchronizing audio and video.
                    mDecoder.queueInputBuffer(inputBufferIndex, 0, bytesToDecode, presentationTime, 0);
                }
            } else {
                Log.e(TAG, "Input Buffer is not available");
            }

            /*
             * Checking if output buffers are ready with the decoded data
             * and returning when they are ready
             */
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            int outputBufferIndex = mDecoder.dequeueOutputBuffer(bufferInfo, TIMEOUT_IN_MICROSECONDS);
            if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                MediaFormat outputFormat = mDecoder.getOutputFormat();
                int sampleRate = outputFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE);
                int channelCount = outputFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT);
                int encoding = outputFormat.getInteger(MediaFormat.KEY_PCM_ENCODING);
                mCallback.onOutputFormatChanged(sampleRate, encoding, channelCount);
            } else if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                return null;
            } else if (outputBufferIndex > 0) {
                if (bufferInfo.flags == BUFFER_FLAG_END_OF_STREAM) {
                    stopDecode();
                    return null;
                }

                mDecoder.getOutputBuffer(outputBufferIndex).position(0);
                ByteBuffer outputDataBuffer = mDecoder.getOutputBuffer(outputBufferIndex);
                byte[] decodedData = new byte[bufferInfo.size];
                // transfer the data from the output buffers to the byte array to be returned.
                outputDataBuffer.get(decodedData);
                mDecoder.releaseOutputBuffer(outputBufferIndex, false);
                return decodedData;
            }
        }
    }

    /**
     * Stops the decoder and free up resources used by the codec instance
     */
    public void stopDecode() {
        if (mDecoder != null) {
            mDecoder.stop();
            mDecoder.release();
            mDecoder = null;
        }
    }

    /**
     * Interface to provide callback operations when the output format changes.
     */
    public interface FormatChangedCallback {
        /**
         * This is called when the output format is changed
         * @param sampleRateInHz - The sample rate of the new format in Hz
         * @param audioFormat - the changed audioFormat.
         * @param channelCount - the number of channels supported.
         */
        void onOutputFormatChanged(int sampleRateInHz, int audioFormat, @IntRange(from = 1, to = 2) int channelCount);
    }
}