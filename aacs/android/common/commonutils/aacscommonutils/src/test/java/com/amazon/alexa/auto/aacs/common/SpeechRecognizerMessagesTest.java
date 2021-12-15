package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

@RunWith(RobolectricTestRunner.class)
public class SpeechRecognizerMessagesTest {
    SpeechRecognizerMessages classUnderTest;

    @Mock
    AACSMessageSender mockMessageSender;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mockMessageSender.sendMessage(anyString(), anyString(), anyString()))
                .thenReturn(CompletableFuture.completedFuture(true));

        classUnderTest = new SpeechRecognizerMessages(mockMessageSender);
    }

    @Test
    public void testCanRequestAudioCapture() throws Exception {
        Future<Boolean> sendResult =
                classUnderTest.sendStartCapture(AASBConstants.SpeechRecognizer.SPEECH_INITIATOR_TAP_TO_TALK);
        assertTrue(sendResult.get());

        verify(mockMessageSender, Mockito.times(1))
                .sendMessage(Mockito.eq(Topic.SPEECH_RECOGNIZER), Mockito.eq(Action.SpeechRecognizer.START_CAPTURE),
                        Mockito.anyString());
    }
}
