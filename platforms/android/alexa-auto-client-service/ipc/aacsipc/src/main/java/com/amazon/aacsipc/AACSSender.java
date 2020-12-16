/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.aacsipc;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class AACSSender {
    private static final String TAG = "AACS-" + AACSSender.class.getSimpleName();
    private static final int MAX_NUM_BYTES_IN_EMBEDDED_MESSAGE_INTENT = 400000;

    private Messenger mSendMessenger = null;
    private Messenger mFetchMessenger = null;
    private Messenger mPushMessenger = null;

    private Looper mLooper = null;

    private SenderMap mMap;
    private int mCacheCapacity;
    private ConcurrentHashMap<String, StreamFetchedFromReceiverCallback> mStreamFetchCallbackMap;
    private ConcurrentHashMap<String, PushToStreamIdCallback> mStreamPushCallbackMap;
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(5);

    public interface StreamFetchedFromReceiverCallback {
        void onStreamFetchedFromServer(ParcelFileDescriptor readPipe);
    }

    public interface PushToStreamIdCallback { void onPushToStreamId(String streamId, ParcelFileDescriptor writePipe); }

    public int getMaxEmbeddedMessageSize() {
        return MAX_NUM_BYTES_IN_EMBEDDED_MESSAGE_INTENT;
    }

    public boolean willMessageFitAsEmbedded(String message) {
        int messageSizeInBytes = message.getBytes(StandardCharsets.UTF_8).length;
        return (messageSizeInBytes <= MAX_NUM_BYTES_IN_EMBEDDED_MESSAGE_INTENT);
    }

    public AACSSender() {
        this(Looper.getMainLooper(), IPCConstants.DEFAULT_CACHE_CAPACITY);
    }

    public AACSSender(int capacity) {
        this(Looper.getMainLooper(), capacity);
    }

    // Constructor
    public AACSSender(Looper looper, int capacity) {
        if (looper == null) {
            Log.e(TAG, "IPC: looper is null in constructor.  defaulting to main looper");
            looper = Looper.getMainLooper();
        }

        mLooper = looper;

        SendHandler sendHandler = new SendHandler(looper);
        mSendMessenger = new Messenger(sendHandler);

        FetchHandler fetchHandler = new FetchHandler(looper);
        mFetchMessenger = new Messenger(fetchHandler);

        PushHandler pushHandler = new PushHandler(looper);
        mPushMessenger = new Messenger(pushHandler);

        mMap = new SenderMap(capacity);
        mCacheCapacity = capacity;
        mStreamFetchCallbackMap = new ConcurrentHashMap<String, StreamFetchedFromReceiverCallback>();
        mStreamPushCallbackMap = new ConcurrentHashMap<String, PushToStreamIdCallback>();
    }

    public void shutDown() {
        if (!mLooper.isCurrentThread()) {
            Log.e(TAG, "IPC: AACSSender shutdown must be called from the looper that was specified in construction");
            return;
        }

        if (mExecutor != null) {
            mExecutor.shutdown();
        }

        if (mStreamFetchCallbackMap != null) {
            mStreamFetchCallbackMap.clear();
        }

        if (mStreamPushCallbackMap != null) {
            mStreamPushCallbackMap.clear();
        }
    }

    // Requirement: message string must be less than 400KB.
    public void sendAASBMessageEmbedded(
            String message, String action, String topic, List<TargetComponent> targets, Context context) {
        checkLogStringValid(message, "message", "sendAASBMessageSmall");
        checkLogStringValid(action, "action", "sendAASBMessageSmall");
        checkLogStringValid(topic, "topic", "sendAASBMessageSmall");
        checkLogArgNonNull(targets, "targets", "sendAASBMessageSmall");
        checkLogArgNonNull(context, "context", "sendAASBMessageSmall");
        checkMessageFitsEmbeddedSizeRequirements(message);

        String intentAction = IPCConstants.AASB_INTENT_PREFIX + action;
        String intentCategory = IPCConstants.AASB_INTENT_PREFIX + topic;

        sendMessage(message, intentAction, intentCategory, targets, context);
    }

    public void sendAASBMessageEmbedded(
            String message, String action, String topic, TargetComponent target, Context context) {
        checkLogStringValid(message, "message", "sendAASBMessageSmall");
        checkLogStringValid(action, "action", "sendAASBMessageSmall");
        checkLogStringValid(topic, "topic", "sendAASBMessageSmall");
        checkLogArgNonNull(target, "target", "sendAASBMessageSmall");
        checkLogArgNonNull(context, "context", "sendAASBMessageSmall");
        checkMessageFitsEmbeddedSizeRequirements(message);

        String intentAction = IPCConstants.AASB_INTENT_PREFIX + action;
        String intentCategory = IPCConstants.AASB_INTENT_PREFIX + topic;

        sendMessage(message, intentAction, intentCategory, target, context);
    }

    public Future<Boolean> sendAASBMessageAnySize(
            String message, String action, String topic, List<TargetComponent> targets, Context context) {
        checkLogStringValid(message, "message", "sendAASBMessageSmall");
        checkLogStringValid(action, "action", "sendAASBMessageSmall");
        checkLogStringValid(topic, "topic", "sendAASBMessageSmall");
        checkLogArgNonNull(targets, "targets", "sendAASBMessageSmall");
        checkLogArgNonNull(context, "context", "sendAASBMessageSmall");

        String intentAction = IPCConstants.AASB_INTENT_PREFIX + action;
        String intentCategory = IPCConstants.AASB_INTENT_PREFIX + topic;

        return sendMessage(message, intentAction, intentCategory, targets, context);
    }

    public Future<Boolean> sendAASBMessageAnySize(
            String message, String action, String topic, TargetComponent target, Context context) {
        checkLogStringValid(message, "message", "sendAASBMessageSmall");
        checkLogStringValid(action, "action", "sendAASBMessageSmall");
        checkLogStringValid(topic, "topic", "sendAASBMessageSmall");
        checkLogArgNonNull(target, "target", "sendAASBMessageSmall");
        checkLogArgNonNull(context, "context", "sendAASBMessageSmall");

        String intentAction = IPCConstants.AASB_INTENT_PREFIX + action;
        String intentCategory = IPCConstants.AASB_INTENT_PREFIX + topic;

        return sendMessage(message, intentAction, intentCategory, target, context);
    }

    public void sendConfigMessageEmbedded(String message, TargetComponent target, Context context) {
        checkLogStringValid(message, "message", "sendConfigMessage");
        checkLogArgNonNull(target, "target", "sendConfigMessage");
        checkLogArgNonNull(context, "context", "sendConfigMessage");
        checkMessageFitsEmbeddedSizeRequirements(message);

        String intentAction = IPCConstants.ACTION_CONFIG;
        String intentCategory = IPCConstants.CATEGORY_SERVICE;

        sendMessage(message, intentAction, intentCategory, target, context);
    }

    public Future<Boolean> sendConfigMessageAnySize(String message, TargetComponent target, Context context) {
        checkLogStringValid(message, "message", "sendConfigMessage");
        checkLogArgNonNull(target, "target", "sendConfigMessage");
        checkLogArgNonNull(context, "context", "sendConfigMessage");

        String intentAction = IPCConstants.ACTION_CONFIG;
        String intentCategory = IPCConstants.CATEGORY_SERVICE;

        return sendMessage(message, intentAction, intentCategory, target, context);
    }

    private Future<Boolean> sendMessage(
            String message, String action, String category, List<TargetComponent> targets, Context context) {
        if (willMessageFitAsEmbedded(message)) {
            Bundle bundle = constructEmbeddedMessageBundle(message);
            for (TargetComponent target : targets) {
                Intent intent = constructIntent(action, category, target, bundle);
                sendIntent(intent, target, context);
            }
            return null;
        } else {
            return sendStreamedMessage(message, action, category, targets, context);
        }
    }

    private Future<Boolean> sendMessage(
            String message, String action, String category, TargetComponent target, Context context) {
        if (willMessageFitAsEmbedded(message)) {
            Bundle bundle = constructEmbeddedMessageBundle(message);
            Intent intent = constructIntent(action, category, target, bundle);
            sendIntent(intent, target, context);
            return null;
        } else {
            return sendStreamedMessage(message, action, category, target, context);
        }
    }

    public void fetch(
            String streamId, StreamFetchedFromReceiverCallback fetchCallback, TargetComponent target, Context context) {
        checkLogStringValid(streamId, "message", "fetch");
        checkLogArgNonNull(fetchCallback, "fetchCallback", "fetch");
        checkLogArgNonNull(target, "target", "fetch");
        checkLogArgNonNull(context, "context", "fetch");

        Log.d(TAG, "IPC: fetching streamId=" + streamId);
        mStreamFetchCallbackMap.put(streamId, fetchCallback);
        String intentAction = IPCConstants.ACTION_FETCH;
        String intentCategory = IPCConstants.CATEGORY_SERVICE;
        Bundle bundle = constructFetchOrPushBundle(streamId, mFetchMessenger);
        Intent intent = constructIntent(intentAction, intentCategory, target, bundle);
        sendIntent(intent, target, context);
    }

    public void cancelFetch(String streamId, TargetComponent target, Context context) {
        checkLogStringValid(streamId, "message", "fetch");
        checkLogArgNonNull(target, "target", "fetch");
        checkLogArgNonNull(context, "context", "fetch");

        Log.d(TAG, "IPC: cancel fetching streamId=" + streamId);
        String intentAction = IPCConstants.ACTION_CANCEL_FETCH;
        String intentCategory = IPCConstants.CATEGORY_SERVICE;
        Bundle bundle = constructFetchOrPushBundle(streamId, mFetchMessenger);
        Intent intent = constructIntent(intentAction, intentCategory, target, bundle);
        sendIntent(intent, target, context);
    }

    public void push(String streamId, PushToStreamIdCallback pushCallback, TargetComponent target, Context context) {
        checkLogStringValid(streamId, "message", "push");
        checkLogArgNonNull(pushCallback, "pushCallback", "push");
        checkLogArgNonNull(target, "target", "push");
        checkLogArgNonNull(context, "context", "push");

        Log.d(TAG, "IPC: pushing to streamId" + streamId);
        mStreamPushCallbackMap.put(streamId, pushCallback);
        String intentAction = IPCConstants.ACTION_PUSH;
        String intentCategory = IPCConstants.CATEGORY_SERVICE;
        Bundle bundle = constructFetchOrPushBundle(streamId, mPushMessenger);
        Intent intent = constructIntent(intentAction, intentCategory, target, bundle);
        sendIntent(intent, target, context);
    }

    private Bundle constructEmbeddedMessageBundle(String message) {
        Bundle bundle = new Bundle();
        bundle.putString(
                IPCConstants.AACS_IPC_MESSAGE_TYPE, IPCConstants.AacsIpcMessageType.EMBEDDED.getTypeAsString());
        bundle.putString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE, message);
        return bundle;
    }

    private Bundle constructStreamingMessageBundle(int resourceId) {
        Bundle bundle = new Bundle();
        bundle.putString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID, UUID.randomUUID().toString());
        bundle.putString(
                IPCConstants.AACS_IPC_MESSAGE_TYPE, IPCConstants.AacsIpcMessageType.STREAMED.getTypeAsString());
        bundle.putString(IPCConstants.AACS_IPC_RESOURCE_ID, Integer.toString(resourceId));
        bundle.putBinder(IPCConstants.AACS_IPC_MESSENGER, mSendMessenger.getBinder());
        return bundle;
    }

    private Bundle constructFetchOrPushBundle(String streamId, Messenger messenger) {
        Bundle bundle = new Bundle();
        bundle.putString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID, UUID.randomUUID().toString());
        bundle.putString(IPCConstants.AACS_IPC_STREAM_ID, streamId);
        bundle.putBinder(IPCConstants.AACS_IPC_MESSENGER, messenger.getBinder());
        return bundle;
    }

    private Intent constructIntent(String action, String category, TargetComponent target, Bundle payload) {
        Intent intent = new Intent();

        if (target != null && target.component != null) {
            intent.setComponent(target.component);
        }

        if (target != null && target.packageName != null) {
            intent.setPackage(target.packageName);
        }

        intent.addCategory(category);
        intent.setAction(action);
        intent.putExtra(IPCConstants.AACS_IPC_MESSAGE_PAYLOAD, payload);
        return intent;
    }

    private Future<Boolean> sendStreamedMessage(
            String message, String action, String category, List<TargetComponent> targets, Context context) {
        CompletableFuture<Boolean> future = new CompletableFuture<>();
        int targetCount = targets.size();
        int resourceId = mMap.put(message, future, targetCount);
        Bundle bundle = constructStreamingMessageBundle(resourceId);
        for (TargetComponent target : targets) {
            Intent intent = constructIntent(action, category, target, bundle);
            sendIntent(intent, target, context);
        }
        return future;
    }

    private Future<Boolean> sendStreamedMessage(
            String message, String action, String category, TargetComponent target, Context context) {
        CompletableFuture<Boolean> future = new CompletableFuture<>();
        int resourceId = mMap.put(message, future, 1);
        Bundle bundle = constructStreamingMessageBundle(resourceId);
        Intent intent = constructIntent(action, category, target, bundle);
        sendIntent(intent, target, context);
        return future;
    }

    private void sendIntent(Intent intent, TargetComponent target, Context context) {
        switch (target.type) {
            case ACTIVITY:
                // startActivity will call the activity to the foreground.
                // Specify a broadcast receiver as the target if don't want the app to be switched to the foreground.
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent, null);
                break;
            case SERVICE:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    context.startForegroundService(intent);
                } else {
                    context.startService(intent);
                }
                break;
            case RECEIVER:
                context.sendBroadcast(intent);
                break;
        }
    }

    private class SendHandler extends Handler {
        SendHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            final int resourceId = Integer.parseInt((String) msg.getData().get(IPCConstants.AACS_IPC_RESOURCE_ID));
            final SenderMap.ResourceBundle resource = mMap.get(resourceId);

            final ParcelFileDescriptor writePipe =
                    (ParcelFileDescriptor) msg.getData().get(IPCConstants.AACS_IPC_WRITE_TO);

            if (writePipe != null) {
                Log.i(TAG, "IPC: Received Message with write pipe from Messenger.");

                Thread writerThread = new Thread() {
                    @Override
                    public void run() {
                        Log.i(TAG, "IPC: STARTING write stream");
                        String message = resource.getMessage();

                        try (ParcelFileDescriptor.AutoCloseOutputStream stream =
                                        new ParcelFileDescriptor.AutoCloseOutputStream(writePipe)) {
                            stream.write(message.getBytes(StandardCharsets.UTF_8));
                            stream.flush();
                        } catch (IOException e) {
                            Log.e(TAG, "IPC: FAILED to write to stream: IOException." + e);
                        }
                    }
                };
                if (!mExecutor.isShutdown()) {
                    mExecutor.submit(writerThread);
                }
            } else {
                boolean result =
                        (msg.getData().get(IPCConstants.AACS_IPC_ACK_STATE)).equals(IPCConstants.AACS_IPC_ACK_SUCCESS);
                if (result) {
                    Log.i(TAG, "IPC: Received Confirmation from Receiver.");
                    resource.decrementCount();
                    if (resource.getTargetCount() == 0) {
                        resource.getFuture().complete(true);
                        mMap.remove(resourceId);
                    }
                }
            }
        }
    }

    private class FetchHandler extends Handler {
        FetchHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(final Message msg) {
            Log.i(TAG, "IPC: Processing Messenger fetch request.");
            final String streamId = msg.getData().getString(IPCConstants.AACS_IPC_STREAM_ID);
            final ParcelFileDescriptor readPipe =
                    (ParcelFileDescriptor) msg.getData().get(IPCConstants.AACS_IPC_READ_FROM);

            Log.i(TAG, "IPC: STARTING fetched read stream with streamId=" + streamId);
            Log.d(TAG, "mStreamFetchCallbackMap = " + mStreamFetchCallbackMap);

            StreamFetchedFromReceiverCallback fetchCallback = mStreamFetchCallbackMap.get(streamId);

            if (fetchCallback != null) {
                fetchCallback.onStreamFetchedFromServer(readPipe);
            } else {
                Log.e(TAG, "IPC: FAILED to find fetch callback associated with streamId: " + streamId);
            }
            if (mStreamFetchCallbackMap.containsKey(streamId)) {
                mStreamFetchCallbackMap.remove(streamId);
            }
        }
    }

    private class PushHandler extends Handler {
        PushHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(final Message msg) {
            Log.i(TAG, "IPC: Processing Messenger push request.");
            final String streamId = msg.getData().getString(IPCConstants.AACS_IPC_STREAM_ID);
            final ParcelFileDescriptor writePipe =
                    (ParcelFileDescriptor) msg.getData().get(IPCConstants.AACS_IPC_WRITE_TO);

            Log.i(TAG, "IPC: STARTING pushing write stream with streamId=" + streamId);
            Log.d(TAG, "mStreamPushCallbackMap = " + mStreamPushCallbackMap);

            PushToStreamIdCallback pushCallback = mStreamPushCallbackMap.get(streamId);

            if (pushCallback != null) {
                pushCallback.onPushToStreamId(streamId, writePipe);
            } else {
                Log.e(TAG, "IPC: FAILED to find push callback associated with streamId: " + streamId);
            }
            if (mStreamPushCallbackMap.containsKey(streamId)) {
                mStreamPushCallbackMap.remove(streamId);
            }
        }
    }

    private void checkMessageFitsEmbeddedSizeRequirements(String message) {
        if (!willMessageFitAsEmbedded(message)) {
            int messageSizeInBytes = message.getBytes(StandardCharsets.UTF_8).length;
            String errorMessage = "IPC: AACSSender message size is too long: " + messageSizeInBytes;
            Log.e(TAG, errorMessage);
            throw new IllegalArgumentException(errorMessage);
        }
    }

    private void checkLogArgNonNull(Object check, String argName, String methodName) {
        if (check == null) {
            String errorMessage = "IPC: " + argName + " is null. " + methodName + " failed";
            Log.e(TAG, errorMessage);
            throw new IllegalArgumentException(errorMessage);
        }
    }

    private void checkLogStringValid(String check, String argName, String methodName) {
        if (check == null) {
            String errorMessage = "IPC: " + argName + " is null. " + methodName + " failed";
            Log.e(TAG, errorMessage);
            throw new IllegalArgumentException(errorMessage);
        } else if (check.isEmpty()) {
            String errorMessage = "IPC: " + argName + " is empty. " + methodName + " failed";
            Log.e(TAG, errorMessage);
            throw new IllegalArgumentException(errorMessage);
        }
    }

    private String getTargetDebugMessage(List<TargetComponent> targets) {
        StringBuilder sb = new StringBuilder();
        for (TargetComponent component : targets) {
            sb.append(", target type: ")
                    .append(component.type)
                    .append(", target: ")
                    .append(component.component.flattenToString());
        }
        return sb.toString();
    }

    private String getTargetDebugMessage(TargetComponent target) {
        List<TargetComponent> targets = new ArrayList<>();
        targets.add(target);
        return getTargetDebugMessage(targets);
    }

    public int getCacheCapacity() {
        return mCacheCapacity;
    }
}
