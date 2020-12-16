package com.amazon.alexa.auto.app.audio;

import static com.amazon.alexa.auto.app.audio.AudioIOServiceWorker.IDLE;
import static com.amazon.alexa.auto.app.audio.AudioIOServiceWorker.WORKING;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.ParcelFileDescriptor;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import java.io.IOException;
import java.util.function.BiConsumer;

import io.reactivex.rxjava3.observers.TestObserver;

public class AudioInputHandlerTest {
    private AudioInputHandler mClassUnderTest;

    @Mock
    AudioInputReader mMockReader;
    @Mock
    ParcelFileDescriptor mMockWritePipe1;
    @Mock
    ParcelFileDescriptor.AutoCloseOutputStream mMockOutputStream1;
    @Mock
    ParcelFileDescriptor mMockWritePipe2;
    @Mock
    ParcelFileDescriptor.AutoCloseOutputStream mMockOutputStream2;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        mClassUnderTest = new AudioInputHandler(mMockReader) {
            @Override
            ParcelFileDescriptor.AutoCloseOutputStream toOutputStream(@NonNull ParcelFileDescriptor writePipe) {
                if (writePipe == mMockWritePipe1)
                    return mMockOutputStream1;
                if (writePipe == mMockWritePipe2)
                    return mMockOutputStream2;
                return null;
            }
        };

        when(mMockReader.isAudioCaptureStarted()).thenAnswer(new Answer<Boolean>() {
            int mCount = 0;
            @Override
            public Boolean answer(InvocationOnMock invocation) throws Throwable {
                // First invocation, not capturing, subsequent invocation, capturing.
                return mCount++ != 0;
            }
        });
    }

    @Test
    public void testAudioInputIsDeliveredToStreams() throws IOException {
        String streamId1 = "test-stream-id-1";
        String streamId2 = "test-stream-id-2";
        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);
        mClassUnderTest.onStreamRequested(streamId2, mMockWritePipe2);

        BiConsumer<byte[], Integer> audioConsumer = verifyStartCapture();
        byte[] fewRawBytes = {0, 1, 2};
        audioConsumer.accept(fewRawBytes, fewRawBytes.length);

        // The bytes should have been committed to the output stream.
        verify(mMockOutputStream1, times(1)).write(fewRawBytes, 0, fewRawBytes.length);
        verify(mMockOutputStream2, times(1)).write(fewRawBytes, 0, fewRawBytes.length);
    }

    @Test
    public void testAudioInputIsDeliveredToOneStreamAfterRemovingAnother() throws IOException {
        String streamId1 = "test-stream-id-1";
        String streamId2 = "test-stream-id-2";
        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);
        mClassUnderTest.onStreamRequested(streamId2, mMockWritePipe2);

        mClassUnderTest.onStreamFetchCancelled(streamId1);

        BiConsumer<byte[], Integer> audioConsumer = verifyStartCapture();
        byte[] fewRawBytes = {0, 1, 2};
        audioConsumer.accept(fewRawBytes, fewRawBytes.length);

        // The bytes should have been committed to the output stream.
        verify(mMockOutputStream1, times(0)).write(fewRawBytes, 0, fewRawBytes.length);
        verify(mMockOutputStream2, times(1)).write(fewRawBytes, 0, fewRawBytes.length);
    }

    @Test
    public void testOnRemovingAllStremsAudioCaptureIsStopped() {
        String streamId1 = "test-stream-id-1";
        String streamId2 = "test-stream-id-2";
        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);
        mClassUnderTest.onStreamRequested(streamId2, mMockWritePipe2);

        verifyStartCapture();

        // Remove 1st stream.
        mClassUnderTest.onStreamFetchCancelled(streamId1);

        // Verify we are still capturing.
        verify(mMockReader, times(0)).stopInputCapture();

        // Remove 2nd stream.
        mClassUnderTest.onStreamFetchCancelled(streamId2);

        // Verify capturing is stopped.
        verify(mMockReader, times(1)).stopInputCapture();
    }

    @Test
    public void testWorkerStateIsTransitionedCorrectly() {
        String streamId1 = "test-stream-id-1";
        String streamId2 = "test-stream-id-2";

        TestObserver<Integer> workerStateStream = mClassUnderTest.getWorkerState().test();
        workerStateStream.assertValueCount(0); // Nothing must have been emitted.

        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);
        workerStateStream.assertValueAt(0, WORKING); // Worker must have transitioned to Working.

        mClassUnderTest.onStreamRequested(streamId2, mMockWritePipe2);
        workerStateStream.assertValueCount(1); // Only WORKING must have been emitted so far.

        // Remove 1st stream.
        mClassUnderTest.onStreamFetchCancelled(streamId1);
        workerStateStream.assertValueCount(1); // Only WORKING must have been emitted so far.

        // Remove 2nd stream.
        mClassUnderTest.onStreamFetchCancelled(streamId2);

        workerStateStream.assertValueAt(1, IDLE); // Should have transitioned to idle.
    }

    @Test
    public void testOnRemovingStreamItIsClosed() throws IOException {
        String streamId1 = "test-stream-id-1";
        String streamId2 = "test-stream-id-2";
        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);
        mClassUnderTest.onStreamRequested(streamId2, mMockWritePipe2);

        // Verify streams have not been closed.
        verify(mMockOutputStream1, times(0)).close();
        verify(mMockOutputStream2, times(0)).close();

        // Remove Streams
        mClassUnderTest.onStreamFetchCancelled(streamId1);
        mClassUnderTest.onStreamFetchCancelled(streamId2);

        // Verify they have been closed.
        verify(mMockOutputStream1, times(1)).close();
        verify(mMockOutputStream2, times(1)).close();
    }

    @Test
    public void testOnWriteFailureStreamIsRemovedFromDispatchList() throws IOException {
        String streamId1 = "test-stream-id-1";
        mClassUnderTest.onStreamRequested(streamId1, mMockWritePipe1);

        doThrow(new IOException()).when(mMockOutputStream1).write(any(byte[].class), anyInt(), anyInt());

        BiConsumer<byte[], Integer> audioConsumer = verifyStartCapture();
        byte[] fewRawBytes = {0, 1, 2};
        audioConsumer.accept(fewRawBytes, fewRawBytes.length);

        // Verify that on account of exception, the output stream has been
        // closed.
        verify(mMockOutputStream1, times(1)).close();

        // And audio input capture is stopped.
        verify(mMockReader, times(1)).stopInputCapture();
    }

    /**
     * Verify that {@link AudioInputReader##startInputCapture} is called.
     *
     * @return Returns the lambda that was supplied to startInputCapture.
     */
    private BiConsumer<byte[], Integer> verifyStartCapture() {
        ArgumentCaptor<BiConsumer<byte[], Integer>> consumerCaptor = ArgumentCaptor.forClass(BiConsumer.class);
        verify(mMockReader, times(1)).startInputCapture(consumerCaptor.capture());
        return consumerCaptor.getValue();
    }
}
