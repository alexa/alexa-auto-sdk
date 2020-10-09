package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.content.Context;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

@RunWith(RobolectricTestRunner.class)
public class AACSMessageSenderTest {
    private AACSMessageSender classUnderTest;

    @Mock
    Context mockContext;
    @Mock
    AACSSender mockSender;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mockSender.sendAASBMessageAnySize(
                     anyString(), anyString(), anyString(), any(TargetComponent.class), eq(mockContext)))
                .thenReturn(CompletableFuture.completedFuture(true));

        classUnderTest = new AACSMessageSender(new WeakReference<Context>(mockContext), mockSender);
    }

    @Test
    @SuppressWarnings("ConstantConditions")
    public void testCanSendMessage() throws Exception {
        String topic = "test-topic";
        String action = "test-action";
        String payload = "{\"test-payload\":42}";

        assertTrue(classUnderTest.sendMessage(topic, action, payload).get());

        ArgumentCaptor<TargetComponent> targetArgCaptor = ArgumentCaptor.forClass(TargetComponent.class);
        ArgumentCaptor<String> rawMsgArgCaptor = ArgumentCaptor.forClass(String.class);

        Mockito.verify(mockSender, Mockito.times(1))
                .sendAASBMessageAnySize(rawMsgArgCaptor.capture(), Mockito.eq(action), Mockito.eq(topic),
                        targetArgCaptor.capture(), Mockito.eq(mockContext));

        Optional<AACSMessage> message = AACSMessageBuilder.parseMessage(rawMsgArgCaptor.getValue());
        assertTrue(message.isPresent());
        assertEquals(action, message.get().action);

        assertEquals(targetArgCaptor.getValue().packageName, AACSConstants.AACS_PACKAGE_NAME);
        assertEquals(targetArgCaptor.getValue().component.getClassName(), AACSConstants.AACS_CLASS_NAME);
    }

    @Test
    @SuppressWarnings("unchecked")
    public void testMessageSendFailsOnInvalidContext() throws Exception {
        WeakReference<Context> mockContext = (WeakReference<Context>) mock(WeakReference.class);
        when(mockContext.get()).thenReturn(null);

        classUnderTest = new AACSMessageSender(mockContext, mockSender);

        Future<Boolean> sendFuture = classUnderTest.sendMessage("topic", "action", null);
        assertFalse(sendFuture.get());
    }

    @Test
    public void testMessageSendFailsOnFailureToBuildMessage() throws Exception {
        classUnderTest = new AACSMessageSender(new WeakReference<>(mockContext), mockSender);

        // payload to trip message builder.
        String invalidJsonPayload = "Answer to life and universe and everything is 42";

        Future<Boolean> sendFuture = classUnderTest.sendMessage("topic", "action", invalidJsonPayload);
        assertFalse(sendFuture.get());
    }

    @Test
    @SuppressWarnings("ConstantConditions")
    public void testCanSendReply() throws Exception {
        String replyToId = "replyToId";
        String topic = "test-topic";
        String action = "test-action";
        String payload = "{\"test-payload\":42}";

        assertTrue(classUnderTest.sendReplyMessage(replyToId, topic, action, payload).get());

        ArgumentCaptor<TargetComponent> targetArgCaptor = ArgumentCaptor.forClass(TargetComponent.class);
        ArgumentCaptor<String> rawMsgArgCaptor = ArgumentCaptor.forClass(String.class);

        Mockito.verify(mockSender, Mockito.times(1))
                .sendAASBMessageAnySize(rawMsgArgCaptor.capture(), Mockito.eq(action), Mockito.eq(topic),
                        targetArgCaptor.capture(), Mockito.eq(mockContext));

        Optional<AACSReplyMessage> replyMessage = AACSMessageBuilder.parseReplyMessage(rawMsgArgCaptor.getValue());
        assertTrue(replyMessage.isPresent());
        assertEquals(replyToId, replyMessage.get().replyToId);

        assertEquals(AACSConstants.AACS_PACKAGE_NAME, targetArgCaptor.getValue().packageName);
        assertEquals(AACSConstants.AACS_CLASS_NAME, targetArgCaptor.getValue().component.getClassName());
    }

    @Test
    @SuppressWarnings("unchecked")
    public void testReplySendFailsOnInvalidContext() throws Exception {
        WeakReference<Context> mockContext = (WeakReference<Context>) mock(WeakReference.class);
        when(mockContext.get()).thenReturn(null);

        classUnderTest = new AACSMessageSender(mockContext, mockSender);

        Future<Boolean> sendFuture = classUnderTest.sendReplyMessage("replyId", "topic", "action", null);
        assertFalse(sendFuture.get());
    }

    @Test
    public void testReplySendFailsOnFailureToBuildMessage() throws Exception {
        classUnderTest = new AACSMessageSender(new WeakReference<>(mockContext), mockSender);

        // payload to trip message builder.
        String invalidJsonPayload = "Answer to life and universe and everything is 42";

        Future<Boolean> sendFuture = classUnderTest.sendReplyMessage("replyId", "topic", "action", invalidJsonPayload);
        assertFalse(sendFuture.get());
    }
}
