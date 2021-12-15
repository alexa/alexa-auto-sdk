package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class ConnectionStatusChangedMessagesTest {
    @Test
    public void testParseConnectionStatus() {
        Optional<String> connectionStatusPayload =
                Optional.of("{\"reason\":\"ACL_CLIENT_REQUEST\",\"status\": \"CONNECTED\"}");

        Optional<String> dialogState =
                ConnectionStatusChangedMessages.parseConnectionStatus(connectionStatusPayload.get());
        assertTrue(dialogState.isPresent());
    }
}
