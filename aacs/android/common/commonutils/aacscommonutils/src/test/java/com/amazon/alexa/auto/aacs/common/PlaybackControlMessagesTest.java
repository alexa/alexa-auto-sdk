package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

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
public class PlaybackControlMessagesTest {
    PlaybackControlMessages classUnderTest;

    @Mock
    AACSMessageSender mockMessageSender;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mockMessageSender.sendMessage(anyString(), anyString(), anyString()))
                .thenReturn(CompletableFuture.completedFuture(true));
        when(mockMessageSender.sendReplyMessage(anyString(), anyString(), anyString(), anyString()))
                .thenReturn(CompletableFuture.completedFuture(true));

        classUnderTest = new PlaybackControlMessages(mockMessageSender);
    }

    @Test
    public void testSendToggleMessage() throws Exception {
        String toggleName = "toggle-name";
        boolean toggleEnabled = true;

        Future<Boolean> sendResult = classUnderTest.sendToggleCommandToAACS(toggleName, toggleEnabled);
        assertTrue(sendResult.get());

        Mockito.verify(mockMessageSender, Mockito.times(1))
                .sendMessage(Mockito.eq(Topic.PLAYBACK_CONTROLLER),
                        Mockito.eq(Action.PlaybackController.TOGGLE_PRESSED), Mockito.anyString());
    }

    @Test
    public void testSendButtonMessage() throws Exception {
        String btnName = "btn-name";

        Future<Boolean> sendResult = classUnderTest.sendButtonCommandToAACS(btnName);
        assertTrue(sendResult.get());

        Mockito.verify(mockMessageSender, Mockito.times(1))
                .sendMessage(Mockito.eq(Topic.PLAYBACK_CONTROLLER),
                        Mockito.eq(Action.PlaybackController.BUTTON_PRESSED), Mockito.anyString());
    }
}
