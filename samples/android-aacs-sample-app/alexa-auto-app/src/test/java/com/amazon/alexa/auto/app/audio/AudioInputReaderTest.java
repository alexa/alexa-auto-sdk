package com.amazon.alexa.auto.app.audio;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.when;

import android.media.AudioRecord;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class AudioInputReaderTest {
    private static final int WAIT_FOR_READ_CALLBACK_MS = 10;

    private AudioInputReader mClassUnderTest;

    @Mock
    AudioRecord mMockAudioRecord;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        mClassUnderTest = new AudioInputReader() {
            @Override
            AudioRecord createAudioInput(int bufferSize) {
                return mMockAudioRecord;
            }
        };
    }

    @Test
    public void testOnStartInputCaptureBytesAreReceived() throws InterruptedException {
        final int bufferSize = 10;
        when(mMockAudioRecord.read(any(byte[].class), anyInt(), anyInt())).thenReturn(bufferSize);

        CountDownLatch inputReceived = new CountDownLatch(1);
        mClassUnderTest.startInputCapture((data, size) -> {
            assertEquals(bufferSize, size.intValue());
            inputReceived.countDown();
        });

        assertTrue(inputReceived.await(WAIT_FOR_READ_CALLBACK_MS, TimeUnit.SECONDS));
    }

    @Test
    public void testOnStopInputCaptureBytesAreNoLongerReceived() {
        final int bufferSize = 10;
        when(mMockAudioRecord.read(any(byte[].class), anyInt(), anyInt())).thenReturn(bufferSize);

        final int COUNTDOWN_SIZE = 10000, STOP_STREAMING_SIZE = COUNTDOWN_SIZE - 100;

        CountDownLatch inputReceived = new CountDownLatch(COUNTDOWN_SIZE);
        mClassUnderTest.startInputCapture((data, size) -> {
            assertEquals(bufferSize, size.intValue());
            inputReceived.countDown();
        });

        // Wait until we have received at least some fixed count of callbacks.
        while (inputReceived.getCount() > STOP_STREAMING_SIZE)
            ;

        mClassUnderTest.stopInputCapture();

        long firstSampleAfterStop = inputReceived.getCount();

        // Wait until the entire loop worth of count down
        for (int count = 0; count < COUNTDOWN_SIZE; ++count)
            ;

        long secondSampleAfterStop = inputReceived.getCount();

        final int ERROR_MARGIN = 10; // Couldn't have emitted more than 10 events after stop.
        assertTrue(secondSampleAfterStop - firstSampleAfterStop < ERROR_MARGIN);
    }

    @Test
    public void testIsCapturingReflectsCurrentState() {
        final int bufferSize = 10;
        when(mMockAudioRecord.read(any(byte[].class), anyInt(), anyInt())).thenReturn(bufferSize);

        mClassUnderTest.startInputCapture((data, size) -> {});

        assertTrue(mClassUnderTest.isAudioCaptureStarted());

        mClassUnderTest.stopInputCapture();

        assertFalse(mClassUnderTest.isAudioCaptureStarted());
    }
}
