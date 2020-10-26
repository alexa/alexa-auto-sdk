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

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.ByteArrayOutputStream;
import java.io.FileDescriptor;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AACSReceiver {
    public static class Builder {
        private MessageReceivedCallback mAASBReceivedCallback = null;
        private MessageReceivedCallback mConfigReceivedCallback = null;
        private FetchStreamCallback mFetchStreamCallback = null;
        private StreamPushedFromSenderCallback mStreamPushedFromSenderCallback = null;
        private Looper mLooper = Looper.getMainLooper();

        public Builder() {}

        public Builder withAASBCallback(MessageReceivedCallback aasbCallback) {
            mAASBReceivedCallback = aasbCallback;
            return this;
        }

        public Builder withConfigCallback(MessageReceivedCallback configCallback) {
            mConfigReceivedCallback = configCallback;
            return this;
        }

        public Builder withFetchCallback(FetchStreamCallback fetchCallback) {
            mFetchStreamCallback = fetchCallback;
            return this;
        }

        public Builder withPushCallback(StreamPushedFromSenderCallback pushCallback) {
            mStreamPushedFromSenderCallback = pushCallback;
            return this;
        }

        public Builder withLooper(Looper looper) {
            mLooper = looper;
            return this;
        }

        public AACSReceiver build() {
            AACSReceiver receiver = new AACSReceiver(mAASBReceivedCallback, mConfigReceivedCallback,
                    mFetchStreamCallback, mStreamPushedFromSenderCallback, mLooper);
            return receiver;
        }
    }

    private static final String TAG = "AACS-" + AACSReceiver.class.getSimpleName();
    private static final int READ_BUFFER_CHUNK_SIZE = 32;
    private MessageReceivedCallback mAASBReceivedCallback;
    private MessageReceivedCallback mConfigReceivedCallback;
    private FetchStreamCallback mFetchStreamCallback;
    private StreamPushedFromSenderCallback mStreamPushedFromSenderCallback;
    private Looper mLooper;
    private Handler mHandler;
    private boolean mIsShutdown = false; // this member is only ever read/written on the same thread as mLooper.
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(5);

    // Constructor
    private AACSReceiver(MessageReceivedCallback messageReceivedCallbacks,
            MessageReceivedCallback configReceivedCallback, FetchStreamCallback fetchStreamCallback,
            StreamPushedFromSenderCallback streamPushedFromSenderCallback, Looper looper) {
        mAASBReceivedCallback = messageReceivedCallbacks;
        mConfigReceivedCallback = configReceivedCallback;
        mFetchStreamCallback = fetchStreamCallback;
        mStreamPushedFromSenderCallback = streamPushedFromSenderCallback;
        mLooper = looper;
        mHandler = new Handler(mLooper);
    }

    public void shutdown() {
        if (!mLooper.isCurrentThread()) {
            Log.e(TAG, "IPC: AACSReceiver shutdown must be called from the looper that was specified in construction");
            return;
        }

        mIsShutdown = true;

        if (mExecutor != null) {
            mExecutor.shutdown();
        }
    }

    public void receive(@NonNull Intent intent, @Nullable BroadcastReceiver.PendingResult result) {
        if (!mLooper.isCurrentThread()) {
            Log.e(TAG, "IPC: receive must be called from the looper that was specified in construction");
            return;
        }

        if (intent == null) {
            Log.e(TAG, "IPC: Intent received is NULL.");
            return;
        }
        if (intent.getExtras() == null) {
            Log.e(TAG, "IPC: Intent received has no extras.");
            return;
        }

        Bundle bundle = intent.getExtras().getBundle(IPCConstants.AACS_IPC_MESSAGE_PAYLOAD);
        if (IPCConstants.ACTION_FETCH.equals(intent.getAction())) {
            handleFetch(bundle);
        } else if (IPCConstants.ACTION_CANCEL_FETCH.equals(intent.getAction())) {
            handleCancelFetch(bundle);
        } else if (IPCConstants.ACTION_PUSH.equals(intent.getAction())) {
            handlePush(bundle);
        } else if (IPCConstants.ACTION_CONFIG.equals(intent.getAction())) {
            if (mConfigReceivedCallback != null) {
                handleSend(bundle, mConfigReceivedCallback, result);
            } else {
                Log.e(TAG, "IPC: received called on config message but mConfigReceivedCallback is null");
            }
        } else {
            if (mAASBReceivedCallback != null) {
                handleSend(bundle, mAASBReceivedCallback, result);
            } else {
                Log.e(TAG, "IPC: received called on config message but mAASBReceivedCallback is null");
            }
        }
    }

    private void handleSend(Bundle bundle, MessageReceivedCallback callback, BroadcastReceiver.PendingResult result) {
        if (bundle == null) {
            return;
        }

        final String type = bundle.getString(IPCConstants.AACS_IPC_MESSAGE_TYPE);
        if (type.equals(IPCConstants.AacsIpcMessageType.EMBEDDED.getTypeAsString())) {
            handleSendEmbedded(bundle, callback, result);
        } else {
            handleSendStreamed(bundle, callback, result);
        }
    }

    private void handleSendEmbedded(
            Bundle bundle, MessageReceivedCallback callback, BroadcastReceiver.PendingResult result) {
        String message = bundle.getString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE);
        callback.onMessageReceived(message);
        if (result != null) {
            Log.i(TAG,
                    "Calling finish() on the PendingResult to inform the system the broadcast receiver can be recycled.");
            result.finish();
        }
    }

    private void handleSendStreamed(
            Bundle bundle, MessageReceivedCallback callback, BroadcastReceiver.PendingResult result) {
        String messageTransferId = bundle.getString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID);
        final String resourceId = bundle.getString(IPCConstants.AACS_IPC_RESOURCE_ID);
        final IBinder msgBinder = bundle.getBinder(IPCConstants.AACS_IPC_MESSENGER);

        if (msgBinder == null) {
            Log.e(TAG, "IPC: msgBinder is null. Aborting AACSReceiver::handleSendStreamed");
            return;
        }

        Messenger messenger = new Messenger(msgBinder);

        ParcelFileDescriptor[] pipe;
        try {
            pipe = ParcelFileDescriptor.createPipe();
        } catch (IOException e) {
            Log.e(TAG, "IPC: IOException encountered on ParcelFileDescriptor.createPipe() in handleSend. " + e);
            return;
        }
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];

        Message msg = Message.obtain();

        Bundle bundleReply = new Bundle();
        bundleReply.putString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID, messageTransferId);
        bundleReply.putString(IPCConstants.AACS_IPC_RESOURCE_ID, resourceId);
        bundleReply.putParcelable(IPCConstants.AACS_IPC_WRITE_TO, writePipe);

        msg.setData(bundleReply);

        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.e(TAG, "IPC: RemoteException encountered on messenger.send() in handleSend. " + e);
            return;
        }

        Thread readerThread = new Thread() {
            @Override
            public void run() {
                Log.i(TAG, "IPC: STARTING read stream.");
                boolean isReading = true;
                byte[] byteArray = new byte[READ_BUFFER_CHUNK_SIZE];
                StringBuilder stringBuilder = new StringBuilder();
                try (ParcelFileDescriptor.AutoCloseInputStream stream =
                                new ParcelFileDescriptor.AutoCloseInputStream(readPipe)) {
                    while (isReading) {
                        int bytesRead = stream.read(byteArray);
                        if (bytesRead > 0) {
                            String subString =
                                    new String(Arrays.copyOfRange(byteArray, 0, bytesRead), StandardCharsets.UTF_8);
                            Log.i(TAG,
                                    "IPC: Substring received.  Bytes read: " + bytesRead + ", Substring: " + subString);
                            stringBuilder.append(subString);
                            if (bytesRead < byteArray.length || stream.available() == 0) {
                                isReading = false;
                            }
                        } else {
                            isReading = false;
                        }
                    }

                    String completeString = stringBuilder.toString();
                    Log.i(TAG, "IPC: Received complete message: " + completeString);

                    Message msg = Message.obtain();
                    Bundle bundleAck = new Bundle();
                    bundleAck.putString(IPCConstants.AACS_IPC_ACK_STATE, IPCConstants.AACS_IPC_ACK_SUCCESS);
                    bundleAck.putString(IPCConstants.AACS_IPC_RESOURCE_ID, resourceId);
                    msg.setData(bundleAck);

                    Messenger messenger = new Messenger(msgBinder);
                    messenger.send(msg);
                    Log.i(TAG, "IPC: Confirmation message sent.");

                    readPipe.close();
                    writePipe.close();

                    mHandler.post(() -> {
                        if (!mIsShutdown) {
                            callback.onMessageReceived(completeString);
                            if (result != null) {
                                Log.i(TAG,
                                        "Calling finish() on the PendingResult to inform the system the broadcast receiver can be recycled.");
                                result.finish();
                            }
                        }
                    });
                } catch (IOException | RemoteException e) {
                    Log.i(TAG, "IPC: FAILED to read from stream." + e);
                }
            }
        };
        if (!mExecutor.isShutdown()) {
            mExecutor.submit(readerThread);
        }
    }

    private void handleFetch(Bundle bundle) {
        if (mFetchStreamCallback == null) {
            Log.e(TAG, "IPC: mFetchStreamCallback is null.  Unable to call fetch handler");
            return;
        }

        String messageTransferId = bundle.getString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID);
        final String streamId = bundle.getString(IPCConstants.AACS_IPC_STREAM_ID);

        Messenger messenger = new Messenger(bundle.getBinder(IPCConstants.AACS_IPC_MESSENGER));

        ParcelFileDescriptor[] pipe;
        try {
            pipe = ParcelFileDescriptor.createPipe();
        } catch (IOException e) {
            Log.e(TAG, "IPC: IOException encountered on ParcelFileDescriptor.createPipe() in handleFetch. " + e);
            return;
        }
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];

        Message msg = Message.obtain();

        Bundle bundleReply = new Bundle();
        Log.d(TAG, "IPC: receiver responding to fetch request with streamId: " + streamId);
        bundleReply.putString(IPCConstants.AACS_IPC_STREAM_ID, streamId);
        bundleReply.putString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID, messageTransferId);
        bundleReply.putParcelable(IPCConstants.AACS_IPC_READ_FROM, readPipe);

        msg.setData(bundleReply);

        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.e(TAG, "IPC: RemoteException encountered on messenger.send() in handleFetch. " + e);
            return;
        }

        Log.d(TAG, "IPC: receiver handleFetch initiating callback to provide data to stream");
        mHandler.post(() -> {
            if (!mIsShutdown) {
                try {
                    readPipe.close();
                } catch (Exception e) {
                    Log.i(TAG, "IPC: Error while closing read pipe " + e.toString());
                }
                mFetchStreamCallback.onStreamRequested(streamId, writePipe);
            }
        });
    }

    private void handleCancelFetch(Bundle bundle) {
        final String streamId = bundle.getString(IPCConstants.AACS_IPC_STREAM_ID);
        if (mFetchStreamCallback == null) {
            Log.e(TAG, "IPC: mFetchStreamCallback is null.  Unable to call fetch handler");
            return;
        }
        Log.d(TAG, "IPC: receiver handleCancelFetch. Calling onStreamFetchCancelled()");
        mFetchStreamCallback.onStreamFetchCancelled(streamId);
    }

    private void handlePush(Bundle bundle) {
        if (mStreamPushedFromSenderCallback == null) {
            Log.e(TAG, "IPC: mStreamPushedFromSenderCallback is null.  Unable to call push handler");
            return;
        }

        String messageTransferId = bundle.getString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID);
        final String streamId = bundle.getString(IPCConstants.AACS_IPC_STREAM_ID);

        Messenger messenger = new Messenger(bundle.getBinder(IPCConstants.AACS_IPC_MESSENGER));

        ParcelFileDescriptor[] pipe;
        try {
            pipe = ParcelFileDescriptor.createPipe();
        } catch (IOException e) {
            Log.e(TAG, "IPC: IOException encountered on ParcelFileDescriptor.createPipe() in handlePush. " + e);
            return;
        }
        final ParcelFileDescriptor readPipe = pipe[0];
        final ParcelFileDescriptor writePipe = pipe[1];

        Message msg = Message.obtain();

        Bundle bundleReply = new Bundle();
        Log.d(TAG, "IPC: receiver responding to push request with streamId: " + streamId);
        bundleReply.putString(IPCConstants.AACS_IPC_STREAM_ID, streamId);
        bundleReply.putString(IPCConstants.AACS_IPC_MESSAGE_TRANSFER_ID, messageTransferId);
        bundleReply.putParcelable(IPCConstants.AACS_IPC_WRITE_TO, writePipe);

        msg.setData(bundleReply);

        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.e(TAG, "IPC: RemoteException encountered on messenger.send() in handlePush. " + e);
            return;
        }

        Log.d(TAG, "IPC: receiver handlePush initiating callback to provide data to stream");

        mHandler.post(() -> {
            if (!mIsShutdown) {
                try {
                    writePipe.close();
                } catch (Exception e) {
                    Log.i(TAG, "IPC: Error while closing write pipe " + e.toString());
                }
                mStreamPushedFromSenderCallback.onStreamPushedFromSenderCallback(streamId, readPipe);
            }
        });
    }

    public interface MessageReceivedCallback { void onMessageReceived(String message); }

    public interface FetchStreamCallback {
        void onStreamRequested(String streamId, ParcelFileDescriptor writePipe);
        void onStreamFetchCancelled(String streamId);
    }

    public interface StreamPushedFromSenderCallback {
        void onStreamPushedFromSenderCallback(String streamId, ParcelFileDescriptor readPipe);
    }
}
