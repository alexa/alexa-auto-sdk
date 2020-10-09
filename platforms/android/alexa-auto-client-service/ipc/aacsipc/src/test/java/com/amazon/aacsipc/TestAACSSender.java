package com.amazon.aacsipc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.robolectric.annotation.LooperMode.Mode.LEGACY;

import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.annotation.LooperMode;
import org.robolectric.shadows.ShadowActivity;
import org.robolectric.shadows.ShadowLooper;
import org.robolectric.shadows.ShadowService;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

@RunWith(RobolectricTestRunner.class)
@LooperMode(LEGACY)
public class TestAACSSender {
    private Context mActivityContext;
    private AACSSender mAACSSender;
    private ShadowActivity mShadowActivity;
    private ShadowLooper mShadowLooperMain;
    private ComponentName mComponentName1;
    private ComponentName mComponentName2;
    private TargetComponent mTargetService;
    private TargetComponent mTarget;
    private List<TargetComponent> mTargets;

    @Before
    public void setup() {
        mAACSSender = new AACSSender();
        Activity activity = Robolectric.buildActivity(Activity.class).create().start().resume().get();
        mActivityContext = activity;
        mShadowActivity = Shadows.shadowOf(activity);
        mShadowLooperMain = Shadows.shadowOf(Looper.getMainLooper());

        mComponentName1 = new ComponentName("TestPackage_ToActivity", "TestClass_ToActivity");
        mComponentName2 = new ComponentName("TestPackage_ToService", "TestClass_ToService");
        mTarget = TargetComponent.withComponent(mComponentName1, TargetComponent.Type.ACTIVITY);
        mTargetService = TargetComponent.withComponent(mComponentName2, TargetComponent.Type.SERVICE);
        mTargets = new ArrayList<>();
        mTargets.add(mTarget);
        mTargets.add(mTargetService);
    }

    @Test
    public void send_verifySmallAASBIntentToService() {
        // Initialize (Ignores initialized fields from setup, as this tests sending to service code path)
        Service service = Robolectric.buildService(DummyService.class).get();
        Context context = service;
        ShadowService shadowService = Shadows.shadowOf(service);
        ComponentName componentName = new ComponentName("TestPackage_ToService", "TestClass_ToService");
        TargetComponent targetComponent = TargetComponent.withComponent(componentName, TargetComponent.Type.SERVICE);
        String testMessage = "test message";
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";

        // Execute
        mAACSSender.sendAASBMessageEmbedded(testMessage, testAction, testTopic, targetComponent, context);
        Intent sentIntent = shadowService.getNextStartedService();

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), sentIntent.getCategories());
        assertEquals(aasbMessagePrefix + testAction, sentIntent.getAction());
        assertEquals(componentName, sentIntent.getComponent());

        Bundle bundle = sentIntent.getExtras().getBundle("payload");
        assertNotNull(bundle);
        assertEquals("embedded", bundle.getString("type"));
        assertEquals(testMessage, bundle.getString("message"));
    }

    @Test
    public void send_verifySmallAASBIntentToActivity() {
        // Initialize (Ignores initialized fields from setup, as this tests sending to service code path)
        String testMessage = "test message";
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";

        // Execute
        mAACSSender.sendAASBMessageEmbedded(testMessage, testAction, testTopic, mTarget, mActivityContext);
        Intent sentIntent = mShadowActivity.getNextStartedActivity();

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), sentIntent.getCategories());
        assertEquals(aasbMessagePrefix + testAction, sentIntent.getAction());
        assertEquals(mComponentName1, sentIntent.getComponent());

        Bundle bundle = sentIntent.getExtras().getBundle("payload");
        assertNotNull(bundle);
        assertEquals("embedded", bundle.getString("type"));
        assertEquals(testMessage, bundle.getString("message"));
    }

    @Test
    public void send_verifyStreamedAASBIntentToActivitySmall() {
        // Execute
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";
        String testMessage = TestUtils.generateTestString(10000);
        Intent sentIntent = sendMessage(testMessage, testAction, testTopic).sentIntent;

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), sentIntent.getCategories());
        assertEquals(aasbMessagePrefix + testAction, sentIntent.getAction());
        assertEquals(mComponentName1, sentIntent.getComponent());
        assertEquals("embedded", sentIntent.getExtras().getBundle("payload").getString("type"));
        assertEquals(testMessage, sentIntent.getExtras().getBundle("payload").getString("message"));
    }

    @Test
    public void send_verifyStreamedAASBIntentToActivityLarge() {
        // Execute
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";
        String testMessage = TestUtils.generateTestString(1000000);
        Intent sentIntent = sendMessage(testMessage, testAction, testTopic).sentIntent;

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), sentIntent.getCategories());
        assertEquals(aasbMessagePrefix + testAction, sentIntent.getAction());
        assertEquals(mComponentName1, sentIntent.getComponent());
        assertEquals("streamed", sentIntent.getExtras().getBundle("payload").getString("type"));
    }

    @Test
    public void send_verifyStreamedMessageSentToActivity() throws IOException, RemoteException, InterruptedException {
        // Initialize
        String verificationMessage = TestUtils.generateTestString(1000000);

        // Execute
        SendMessageResult sendMessageResult = sendMessage(verificationMessage, "untested action", "untested topic");
        ReplyToSenderResult replyToSenderResult = replyToSender(sendMessageResult.sentIntent);

        // Verify
        assertEquals(false, sendMessageResult.sendCompletionStatusFuture.isDone());
        assertEquals(verificationMessage.getBytes().length, replyToSenderResult.bytesRead);
        assertEquals(verificationMessage, replyToSenderResult.messageFromSender);
    }

    @Test
    public void send_verifyCompletionStatus()
            throws IOException, RemoteException, InterruptedException, ExecutionException {
        // Execute
        String unusedLargedDummyMessage = TestUtils.generateTestString(1000000);
        SendMessageResult sendMessageResult =
                sendMessage(unusedLargedDummyMessage, "untested action", "untested topic");
        ReplyToSenderResult replyToSenderResult = replyToSender(sendMessageResult.sentIntent);
        ackMessage(replyToSenderResult.resourceId, replyToSenderResult.messengerFromIntent);

        // Verify
        assertEquals(true, sendMessageResult.sendCompletionStatusFuture.isDone());
        assertEquals(true, sendMessageResult.sendCompletionStatusFuture.get());
    }

    @Test
    public void send_verifySmallAASBIntentToMultipleTargets() {
        // Initialize (Ignores initialized fields from setup, as this tests sending to service code path)
        Service service = Robolectric.buildService(DummyService.class).get();
        ShadowService shadowService = Shadows.shadowOf(service);
        String testMessage = "test message";
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";

        // Execute
        mAACSSender.sendAASBMessageEmbedded(testMessage, testAction, testTopic, mTargets, service);
        Intent serviceIntent = shadowService.getNextStartedService();
        Intent activityIntent = shadowService.getNextStartedActivity();

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), serviceIntent.getCategories());
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), activityIntent.getCategories());

        assertEquals(aasbMessagePrefix + testAction, serviceIntent.getAction());
        assertEquals(aasbMessagePrefix + testAction, activityIntent.getAction());
        assertEquals(mComponentName1, activityIntent.getComponent());
        assertEquals(mComponentName2, serviceIntent.getComponent());

        Bundle serviceBundle = serviceIntent.getExtras().getBundle("payload");
        Bundle activityBundle = activityIntent.getExtras().getBundle("payload");
        assertNotNull(serviceBundle);
        assertNotNull(activityBundle);
        assertEquals("embedded", serviceBundle.getString("type"));
        assertEquals("embedded", activityBundle.getString("type"));
        assertEquals(testMessage, serviceBundle.getString("message"));
        assertEquals(testMessage, activityBundle.getString("message"));
    }

    @Test
    public void send_verifyLargeAASBIntentToMultipleTargets() {
        Service service = Robolectric.buildService(DummyService.class).get();
        ShadowService shadowService = Shadows.shadowOf(service);
        String testMessage = TestUtils.generateTestString(1000000);
        String testAction = "test action";
        String testTopic = "test topic";
        String aasbMessagePrefix = "com.amazon.aacs.aasb.";

        // Execute
        mAACSSender.sendAASBMessageAnySize(testMessage, testAction, testTopic, mTargets, service);
        Intent serviceIntent = shadowService.getNextStartedService();
        Intent activityIntent = shadowService.getNextStartedActivity();

        // Verify
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), serviceIntent.getCategories());
        assertEquals(new HashSet<>(Arrays.asList(aasbMessagePrefix + testTopic)), activityIntent.getCategories());
        assertEquals(aasbMessagePrefix + testAction, serviceIntent.getAction());
        assertEquals(aasbMessagePrefix + testAction, activityIntent.getAction());
        assertEquals(mComponentName1, activityIntent.getComponent());
        assertEquals(mComponentName2, serviceIntent.getComponent());

        assertEquals("streamed", serviceIntent.getExtras().getBundle("payload").getString("type"));
        assertEquals("streamed", activityIntent.getExtras().getBundle("payload").getString("type"));
    }

    @Test
    public void send_verifyCacheCapacity() {
        assertEquals(mAACSSender.getCacheCapacity(), IPCConstants.DEFAULT_CACHE_CAPACITY);
        AACSSender testSender = new AACSSender(10);
        assertEquals(testSender.getCacheCapacity(), 10);
    }

    private SendMessageResult sendMessage(String message, String action, String topic) {
        Future<Boolean> sendResult =
                mAACSSender.sendAASBMessageAnySize(message, action, topic, mTarget, mActivityContext);
        Intent sentIntent = mShadowActivity.getNextStartedActivity();
        return new SendMessageResult(sendResult, sentIntent);
    }

    class SendMessageResult {
        public Future<Boolean> sendCompletionStatusFuture;
        public Intent sentIntent;

        public SendMessageResult(Future<Boolean> sendCompletionStatusFuture, Intent sendIntent) {
            this.sendCompletionStatusFuture = sendCompletionStatusFuture;
            this.sentIntent = sendIntent;
        }
    }

    private ReplyToSenderResult replyToSender(Intent sendIntent)
            throws IOException, RemoteException, InterruptedException {
        Bundle bundleFromSender = sendIntent.getExtras().getBundle("payload");
        String messageTransferId = bundleFromSender.getString("messageTransferId");
        String resourceId = bundleFromSender.getString("resourceId");

        Messenger messengerFromIntent = new Messenger(bundleFromSender.getBinder("messenger"));

        Bundle bundleToSender = new Bundle();
        bundleToSender.putString("messageTransferId", messageTransferId);
        bundleToSender.putString("resourceId", resourceId);

        ParcelFileDescriptor[] pipe = ParcelFileDescriptor.createPipe();
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];
        bundleToSender.putParcelable("writeTo", writePipe);

        Message msg = Message.obtain();
        msg.setData(bundleToSender);

        messengerFromIntent.send(msg);
        mShadowLooperMain.runToEndOfTasks(); // will execute 1 iteration of the message loop.

        byte[] byteArray = new byte[1000000];
        ParcelFileDescriptor.AutoCloseInputStream stream = new ParcelFileDescriptor.AutoCloseInputStream(readPipe);

        // Reading from streams in a robolectric context works differently from Android devices.
        // On Android, the read will not return -1 until the EOF is hit.  The EOF is not inserted
        // into the stream until the writePipe has been closed.
        // In Robolectric, the EOF is hit upon the first read, regardless what the writer has
        // inserted.  We sleep for this duration to ensure that the writing stream has had
        // enough time to complete the write.
        Thread.sleep(500);

        int bytesRead = stream.read(byteArray);
        String messageFromSender = new String(Arrays.copyOfRange(byteArray, 0, bytesRead), StandardCharsets.UTF_8);
        stream.close();

        return new ReplyToSenderResult(messageFromSender, resourceId, bytesRead, messengerFromIntent);
    }

    class ReplyToSenderResult {
        public String messageFromSender;
        public String resourceId;
        public int bytesRead;
        public Messenger messengerFromIntent;

        public ReplyToSenderResult(
                String messageFromSender, String resourceId, int bytesRead, Messenger messengerFromIntent) {
            this.messageFromSender = messageFromSender;
            this.resourceId = resourceId;
            this.bytesRead = bytesRead;
            this.messengerFromIntent = messengerFromIntent;
        }
    }

    private void ackMessage(String resourceID, Messenger messengerFromIntent) throws RemoteException {
        Message ackMessage = Message.obtain();
        Bundle bundleAck = new Bundle();
        bundleAck.putString("state", "success");
        bundleAck.putString("resourceId", resourceID);
        ackMessage.setData(bundleAck);

        messengerFromIntent.send(ackMessage);

        mShadowLooperMain.runToEndOfTasks();
    }

    @Test
    public void fetch_verifyIntentToActivity() {
        // Initialize
        String verificationStreamId = "Test streamID";
        AACSSender.StreamFetchedFromReceiverCallback fetchCallback = (inputStream) -> {};

        // Execute
        Intent fetchIntent = fetchMessage(verificationStreamId, fetchCallback);

        // Verify
        assertEquals(new HashSet<>(Arrays.asList("com.amazon.aacs.service")), fetchIntent.getCategories());
        assertEquals("com.amazon.aacs.service.fetch", fetchIntent.getAction());
        assertEquals(mComponentName1, fetchIntent.getComponent());
        assertEquals(verificationStreamId, fetchIntent.getExtras().getBundle("payload").getString("streamId"));
    }

    @Test
    public void fetch_verifyStreamReceivedFromActivity()
            throws IOException, RemoteException, ExecutionException, InterruptedException {
        // Initialize
        String verificationFetchedString = "Fetched string";
        final CompletableFuture<String> resultString = new CompletableFuture<>();
        AACSSender.StreamFetchedFromReceiverCallback fetchCallback = (readPipe) -> {
            ParcelFileDescriptor.AutoCloseInputStream inputStream =
                    new ParcelFileDescriptor.AutoCloseInputStream(readPipe);
            try {
                byte[] byteArray = new byte[100000];
                int bytesRead = inputStream.read(byteArray);
                String fetchedMessage = new String(Arrays.copyOfRange(byteArray, 0, bytesRead), StandardCharsets.UTF_8);
                resultString.complete(fetchedMessage);
            } catch (Exception e) {
                throw new RuntimeException("failed on exception " + e);
            } finally {
                try {
                    inputStream.close();
                } catch (Exception e) {
                    throw new RuntimeException("failed on exception " + e);
                }
            }
        };

        // Execute
        Intent fetchIntent = fetchMessage("testID123", fetchCallback);
        fetchReplyToSender(fetchIntent, verificationFetchedString);

        // Verify
        assertEquals(verificationFetchedString, resultString.get());
    }

    private Intent fetchMessage(String streamId, AACSSender.StreamFetchedFromReceiverCallback fetchCallback) {
        mAACSSender.fetch(streamId, fetchCallback, mTarget, mActivityContext);
        Intent fetchIntent = mShadowActivity.getNextStartedActivity();
        return fetchIntent;
    }

    private void fetchReplyToSender(Intent fetchIntent, String stringToFetch) throws IOException, RemoteException {
        Bundle bundleFromSender = fetchIntent.getExtras().getBundle("payload");
        String messageTransferId = bundleFromSender.getString("messageTransferId");
        String streamId = bundleFromSender.getString("streamId");

        Messenger messengerFromIntent = new Messenger(bundleFromSender.getBinder("messenger"));

        Bundle bundleToSender = new Bundle();
        bundleToSender.putString("messageTransferId", messageTransferId);
        bundleToSender.putString("streamId", streamId);

        ParcelFileDescriptor[] pipe = ParcelFileDescriptor.createPipe();
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];

        ParcelFileDescriptor.AutoCloseOutputStream stream = new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);
        stream.write(stringToFetch.getBytes(StandardCharsets.UTF_8));
        stream.close();

        bundleToSender.putParcelable("readFrom", readPipe);

        Message msg = Message.obtain();
        msg.setData(bundleToSender);

        messengerFromIntent.send(msg);
        mShadowLooperMain.runToEndOfTasks();
    }

    @Test
    public void push_verifyIntentToActivity() {
        // Initialize
        String verificationStreamId = "Test streamID";
        AACSSender.PushToStreamIdCallback pushCallback = (streamId, writePipe) -> {};

        // Execute
        Intent pushIntent = pushMessage(verificationStreamId, pushCallback);

        // Verify
        assertEquals(new HashSet<>(Arrays.asList("com.amazon.aacs.service")), pushIntent.getCategories());
        assertEquals("com.amazon.aacs.service.push", pushIntent.getAction());
        assertEquals(mComponentName1, pushIntent.getComponent());
        assertEquals(verificationStreamId, pushIntent.getExtras().getBundle("payload").getString("streamId"));
    }

    @Test
    public void push_verifyStreamReceivedFromActivity()
            throws IOException, RemoteException, ExecutionException, InterruptedException {
        // Initialize
        String verificationFetchedString = "Pushed string";
        AACSSender.PushToStreamIdCallback pushCallback = (streamId, writePipe) -> {
            ParcelFileDescriptor.AutoCloseOutputStream outputStream =
                    new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);
            try {
                outputStream.write(verificationFetchedString.getBytes(StandardCharsets.UTF_8));
            } catch (Exception e) {
                throw new RuntimeException("failed on exception " + e);
            } finally {
                try {
                    outputStream.close();
                } catch (Exception e) {
                    throw new RuntimeException("failed on exception " + e);
                }
            }
        };

        // Execute
        Intent pushIntent = pushMessage("testID123", pushCallback);
        ReplyToPushResult replyToPushResult = pushReplyToSender(pushIntent);

        // Verify
        assertEquals(verificationFetchedString.getBytes().length, replyToPushResult.bytesRead);
        assertEquals(verificationFetchedString, replyToPushResult.messageFromSender);
    }

    private Intent pushMessage(String streamId, AACSSender.PushToStreamIdCallback pushCallback) {
        mAACSSender.push(streamId, pushCallback, mTarget, mActivityContext);
        Intent pushIntent = mShadowActivity.getNextStartedActivity();
        return pushIntent;
    }

    private ReplyToPushResult pushReplyToSender(Intent sendIntent)
            throws IOException, RemoteException, InterruptedException {
        Bundle bundleFromSender = sendIntent.getExtras().getBundle("payload");
        String messageTransferId = bundleFromSender.getString("messageTransferId");
        String streamId = bundleFromSender.getString("streamId");

        Messenger messengerFromIntent = new Messenger(bundleFromSender.getBinder("messenger"));

        Bundle bundleToSender = new Bundle();
        bundleToSender.putString("messageTransferId", messageTransferId);
        bundleToSender.putString("streamId", streamId);

        ParcelFileDescriptor[] pipe = ParcelFileDescriptor.createPipe();
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];
        bundleToSender.putParcelable("writeTo", writePipe);

        Message msg = Message.obtain();
        msg.setData(bundleToSender);

        messengerFromIntent.send(msg);
        mShadowLooperMain.runToEndOfTasks(); // will execute 1 iteration of the message loop.

        // Reading from streams in a robolectric context works differently from Android devices.
        // On Android, the read will not return -1 until the EOF is hit.  The EOF is not inserted
        // into the stream until the writePipe has been closed.
        // In Robolectric, the EOF is hit upon the first read, regardless what the writer has
        // inserted.  We sleep for this duration to ensure that the writing stream has had
        // enough time to complete the write.
        Thread.sleep(500);

        byte[] byteArray = new byte[1000000];
        ParcelFileDescriptor.AutoCloseInputStream stream = new ParcelFileDescriptor.AutoCloseInputStream(readPipe);
        int bytesRead = stream.read(byteArray);

        String messageFromSender = new String(Arrays.copyOfRange(byteArray, 0, bytesRead), StandardCharsets.UTF_8);
        stream.close();

        return new ReplyToPushResult(messageFromSender, streamId, bytesRead, messengerFromIntent);
    }

    class ReplyToPushResult {
        public String messageFromSender;
        public String streamId;
        public int bytesRead;
        public Messenger messengerFromIntent;

        public ReplyToPushResult(
                String messageFromSender, String streamId, int bytesRead, Messenger messengerFromIntent) {
            this.messageFromSender = messageFromSender;
            this.streamId = streamId;
            this.bytesRead = bytesRead;
            this.messengerFromIntent = messengerFromIntent;
        }
    }
}