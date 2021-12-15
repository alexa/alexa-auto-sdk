package com.amazon.alexa.auto.app.audio;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.BiConsumer;

/**
 * Helper class to read audio input from microphone using Android APIs.
 */
public class AudioInputReader {
    private static final String TAG = AudioInputReader.class.getSimpleName();

    // All audio input consumers expect PCM 16 data @ 16 Khz. We divide this consumption into 10 ms
    // chunks. It comes out at 160 samples every 10 ms to reach 16000 samples (in a second).
    private static final int sSamplesToCollectInOneCycle = 160;
    private static final int sBytesInEachSample = 2; // PCM 16 = 2 bytes per sample
    private static final int sSampleRateInHz = 16000; // 16 khz
    private static final int sAudioFramesInBuffer = 5; // number of audio frames.

    @Nullable
    private AudioRecord mAudioRecord;
    @NonNull
    private final AtomicBoolean mShouldStop;

    public AudioInputReader() {
        mShouldStop = new AtomicBoolean(false);
    }

    public void startInputCapture(@NonNull BiConsumer<byte[], Integer> audioBytesConsumer) {
        Log.i(TAG, "Starting audio input capture");
        Preconditions.checkArgument(mAudioRecord == null);

        mShouldStop.set(false);

        mAudioRecord = createAudioInput();
        mAudioRecord.startRecording();
        new Thread(() -> {
            byte[] buffer = new byte[sSamplesToCollectInOneCycle * sBytesInEachSample];
            while (true) {
                int size = mAudioRecord.read(buffer, 0, buffer.length);
                boolean stopSignaled = mShouldStop.get();

                if (size > 0 && !stopSignaled) {
                    audioBytesConsumer.accept(buffer, size);
                }

                if (stopSignaled) {
                    break;
                }
            }
        }).start();
    }

    public boolean isAudioCaptureStarted() {
        return mAudioRecord != null;
    }

    public void stopInputCapture() {
        Log.i(TAG, "Stopping audio input capture");

        mShouldStop.set(true);
        if (mAudioRecord != null) {
            mAudioRecord.stop();
            mAudioRecord = null;
        }
    }

    /**
     * Create an instance of {@link AudioRecord} with appropriate buffer size.
     *
     * @return New instance of {@link AudioRecord}.
     */
    private AudioRecord createAudioInput() {
        int minBufferSize = AudioRecord.getMinBufferSize(
                sSampleRateInHz, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
        int bufferSize = minBufferSize + (sAudioFramesInBuffer * sSamplesToCollectInOneCycle * sBytesInEachSample);

        return createAudioInput(bufferSize);
    }

    /**
     * Create an instance of {@link AudioRecord} with given buffer size.
     *
     * @param bufferSize Size of buffer backing the audio input read.
     * @return New instance of {@link AudioRecord}.
     */
    @VisibleForTesting
    AudioRecord createAudioInput(int bufferSize) {
        return new AudioRecord(MediaRecorder.AudioSource.MIC, sSampleRateInHz, AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT, bufferSize);
    }
}
