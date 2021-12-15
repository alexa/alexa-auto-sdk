package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import android.content.Intent;

import androidx.annotation.NonNull;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class AACSMessageBuilderTest {
    @Test
    public void testCanBuildAndParseSimpleMessage() {
        String topic = "topic";
        String action = "action";
        String payload = "{\"test\":\"123\"}";

        testSimpleMessage(topic, action, payload);
    }

    @Test
    public void testNullPayloadForBuildingParsingMessage() {
        String topic = "topic";
        String action = "action";
        String payload = null;

        testSimpleMessage(topic, action, payload);
    }

    @Test
    public void testCanBuildAndParseReplyMessage() {
        String replyToId = "reply-id";
        String topic = "my-topic";
        String action = "my-action";
        String payload = "{\"test\":\"123\"}";
        testReplyMessage(replyToId, topic, action, payload);
    }

    @Test
    public void testNullPayloadIsAcceptedForBuildingParsingReplyMessage() {
        String replyToId = "reply-id";
        String topic = "my-topic";
        String action = "my-action";
        String payload = null;
        testReplyMessage(replyToId, topic, action, payload);
    }

    @Test
    public void testCanBuildParseMessageFromIntent() {
        String topic = "topic";
        String action = "action";
        String payload = null;

        Optional<Intent> builtIntent = AACSMessageBuilder.buildEmbeddedMessageIntent(topic, action, payload);
        assertTrue(builtIntent.isPresent());

        Optional<AACSMessage> message = AACSMessageBuilder.parseEmbeddedIntent(builtIntent.get());
        assertTrue(message.isPresent());

        assertEquals(topic, message.get().topic);
        assertEquals(action, message.get().action);
        assertEquals(payload, message.get().payload);
    }

    private void testSimpleMessage(@NonNull String topic, @NonNull String action, String payload) {
        Optional<String> message = AACSMessageBuilder.buildMessage(topic, action, payload);
        assertTrue(message.isPresent());

        // Parse the message and match the inputs.
        Optional<AACSMessage> parsedMessage = AACSMessageBuilder.parseMessage(message.get());
        assertTrue(parsedMessage.isPresent());

        assertEquals(topic, parsedMessage.get().topic);
        assertEquals(action, parsedMessage.get().action);
        assertEquals(payload, parsedMessage.get().payload);
    }

    private void testReplyMessage(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, String payload) {
        Optional<String> message = AACSMessageBuilder.buildReplyMessage(replyToId, topic, action, payload);
        assertTrue(message.isPresent());

        // Parse the message and match the inputs.
        Optional<AACSReplyMessage> parsedMessage = AACSMessageBuilder.parseReplyMessage(message.get());
        assertTrue(parsedMessage.isPresent());

        assertEquals(replyToId, parsedMessage.get().replyToId);
        assertEquals(topic, parsedMessage.get().topic);
        assertEquals(action, parsedMessage.get().action);
        assertEquals(payload, parsedMessage.get().payload);
    }
}