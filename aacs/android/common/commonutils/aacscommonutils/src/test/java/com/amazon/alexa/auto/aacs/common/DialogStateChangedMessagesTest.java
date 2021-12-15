package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class DialogStateChangedMessagesTest {
    @Test
    public void testParseDialogState() {
        Optional<String> dialogStatePayload = Optional.of("{\"state\": \"SPEAKING\"}");

        Optional<String> dialogState = DialogStateChangedMessages.parseDialogState(dialogStatePayload.get());
        assertTrue(dialogState.isPresent());
    }
}
