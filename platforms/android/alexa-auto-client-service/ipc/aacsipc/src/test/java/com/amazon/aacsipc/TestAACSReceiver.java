package com.amazon.aacsipc;

import static org.junit.Assert.assertEquals;
import static org.robolectric.annotation.LooperMode.Mode.LEGACY;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.annotation.LooperMode;
import org.robolectric.shadows.ShadowLooper;

import java.nio.charset.StandardCharsets;
import java.util.concurrent.CompletableFuture;

@RunWith(RobolectricTestRunner.class)
@LooperMode(LEGACY)
public class TestAACSReceiver {
    String mVerifyAASBMessage;
    String mVerifyResourceId;
    Intent mTestIntent;
    Bundle mTestBundleEmbedded;
    Bundle mTestBundleStreamed;

    ShadowLooper mLooper;

    @Before
    public void setup() {
        mVerifyAASBMessage = "test aasb message";

        mVerifyResourceId = "test resource id";

        mTestIntent = new Intent("com.amazon.aacs.aasb.action");
        mTestIntent.addCategory("com.amazon.aacs.aasb.category");

        mTestBundleEmbedded = new Bundle();
        mTestBundleEmbedded.putString("type", "embedded");
        mTestBundleEmbedded.putString("message", mVerifyAASBMessage);

        mTestBundleStreamed = new Bundle();
        mTestBundleStreamed.putString("type", "streamed");
        mTestBundleStreamed.putString("messageTransferId", "123456");
        mTestBundleStreamed.putString("resourceId", mVerifyResourceId);

        mLooper = Shadows.shadowOf(Looper.getMainLooper());
    }

    @Test
    public void receive_embeddedAASBIntent() throws Exception {
        // Initialize
        mTestIntent.putExtra("payload", mTestBundleEmbedded);

        final CompletableFuture<String> resultMessage = new CompletableFuture<>();

        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        AACSReceiver receiver = builder.withAASBCallback((message) -> { resultMessage.complete(message); }).build();

        // Execute
        receiver.receive(mTestIntent, null);
        String checkMessage = resultMessage.get();

        // Verify
        assertEquals(mVerifyAASBMessage, checkMessage);
    }

    @Test
    public void receive_streamedAASBIntent() throws Exception {
        // Initialize
        CompletableFuture<String> checkResourceId = new CompletableFuture<>();
        CompletableFuture<String> ackState = new CompletableFuture<>();
        CompletableFuture<String> ackResourceId = new CompletableFuture<>();
        Messenger mSendMessenger = new Messenger(new Handler() {
            public void handleMessage(Message msg) {
                String state = msg.getData().getString("state");
                if (state == null) {
                    checkResourceId.complete(msg.getData().getString("resourceId"));
                    ParcelFileDescriptor writeTo = (ParcelFileDescriptor) msg.getData().get("writeTo");
                    try (ParcelFileDescriptor.AutoCloseOutputStream stream =
                                    new ParcelFileDescriptor.AutoCloseOutputStream(writeTo)) {
                        stream.write(mVerifyAASBMessage.getBytes(StandardCharsets.UTF_8));
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                } else {
                    ackState.complete(state);
                    ackResourceId.complete(msg.getData().getString("resourceId"));
                }
            }
        });
        mTestBundleStreamed.putBinder("messenger", mSendMessenger.getBinder());
        mTestIntent.putExtra("payload", mTestBundleStreamed);

        final CompletableFuture<String> resultMessage = new CompletableFuture<>();

        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        AACSReceiver receiver = builder.withAASBCallback((message) -> { resultMessage.complete(message); }).build();

        // Execute
        receiver.receive(mTestIntent, null);
        while (!resultMessage.isDone()) {
            mLooper.runToEndOfTasks();
        }
        String checkMessage = resultMessage.get();

        // Verify
        assertEquals(mVerifyResourceId, checkResourceId.get());
        assertEquals(mVerifyAASBMessage, checkMessage);
    }

    @Test
    public void receive_streamedAASBIntentHuge() throws Exception {
        // Initialize
        String verifyAASBMessageHuge = TestUtils.generateTestString(100000);
        CompletableFuture<String> checkResourceId = new CompletableFuture<>();
        CompletableFuture<String> ackState = new CompletableFuture<>();
        CompletableFuture<String> ackResourceId = new CompletableFuture<>();
        Messenger mSendMessenger = new Messenger(new Handler() {
            public void handleMessage(Message msg) {
                String state = msg.getData().getString("state");
                if (state == null) {
                    checkResourceId.complete(msg.getData().getString("resourceId"));
                    ParcelFileDescriptor writeTo = (ParcelFileDescriptor) msg.getData().get("writeTo");
                    ParcelFileDescriptor.AutoCloseOutputStream stream =
                            new ParcelFileDescriptor.AutoCloseOutputStream(writeTo);
                    try {
                        stream.write(verifyAASBMessageHuge.getBytes(StandardCharsets.UTF_8));
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                } else {
                    ackState.complete(state);
                    ackResourceId.complete(msg.getData().getString("resourceId"));
                }
            }
        });
        mTestBundleStreamed.putBinder("messenger", mSendMessenger.getBinder());
        mTestIntent.putExtra("payload", mTestBundleStreamed);

        final CompletableFuture<String> resultMessage = new CompletableFuture<>();

        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        AACSReceiver receiver = builder.withAASBCallback((message) -> { resultMessage.complete(message); }).build();

        // Execute
        receiver.receive(mTestIntent, null);
        while (!resultMessage.isDone()) {
            mLooper.runToEndOfTasks();
        }
        String checkMessage = resultMessage.get();

        // Verify
        assertEquals(mVerifyResourceId, checkResourceId.get());
        assertEquals(verifyAASBMessageHuge, checkMessage);
    }
}
