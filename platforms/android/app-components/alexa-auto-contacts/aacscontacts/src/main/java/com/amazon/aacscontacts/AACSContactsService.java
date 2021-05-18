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

package com.amazon.aacscontacts;

import static com.amazon.aacsconstants.AACSConstants.PAYLOAD;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.ContactsConstants;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSReplyMessage;

import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AACSContactsService extends Service {
    private final String TAG = AACSConstants.AACS + "-" + AACSContactsService.class.getSimpleName();
    private static final String CONTACTS_CHANNEL_ID = "com.amazon.aacscontacts";
    private static final String CONTACTS_CHANNEL_NAME = "AACSContacts";
    private PhoneBookController mPhoneBookController;
    private static ExecutorService mExecutor;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "AACS Contacts Service created");
        startAACSContactsService();
        mPhoneBookController = new PhoneBookController(this);
        mExecutor = Executors.newSingleThreadExecutor();
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        if (intent != null) {
            String.format("Receiving intent: action=%s, category=%s", intent.getAction(), intent.getCategories());
            handleIntent(intent);
        }
        return Service.START_NOT_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "AACS Contacts Service destroyed");
    }

    private void handleIntent(Intent intent) {
        if (ContactsConstants.CONTACTS_ACTION_UPLOAD.equals(intent.getAction())) {
            upload(intent.getStringExtra(ContactsConstants.ADDRESS_BOOK_SOURCE_ID),
                    intent.getStringExtra(ContactsConstants.ADDRESS_BOOK_NAME));
            return;
        } else if (ContactsConstants.CONTACTS_ACTION_REMOVE.equals(intent.getAction())) {
            remove(intent.getStringExtra(ContactsConstants.ADDRESS_BOOK_SOURCE_ID));
            return;
        }

        Bundle payloadBundle = intent.getBundleExtra(PAYLOAD);
        String replyMessage =
                payloadBundle != null ? payloadBundle.getString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE) : null;
        Optional<AACSReplyMessage> messageOptional = AACSMessageBuilder.parseReplyMessage(replyMessage);
        if (messageOptional.isPresent()) {
            AACSReplyMessage message = messageOptional.get();

            switch (message.action) {
                case Action.AddressBook.ADD_ADDRESS_BOOK:
                    Log.i(TAG,
                            String.format(
                                    "AddressBook Adding %s with ReplyToId: %s", message.payload, message.replyToId));
                    if (message.payload.contains("true")) {
                        mPhoneBookController.completeAddressBookUpdate(message.replyToId, true);
                    } else {
                        mPhoneBookController.completeAddressBookUpdate(message.replyToId, false);
                    }
                    stopAACSContactsService();
                    break;
                case Action.AddressBook.REMOVE_ADDRESS_BOOK:
                    Log.i(TAG,
                            String.format(
                                    "AddressBook Removing %s with ReplyToId: %s", message.payload, message.replyToId));
                    if (message.payload.contains("true")) {
                        mPhoneBookController.completeAddressBookUpdate(message.replyToId, true);
                    } else {
                        mPhoneBookController.completeAddressBookUpdate(message.replyToId, false);
                    }
                    stopAACSContactsService();
                    break;
            }
        }
    }

    public void upload(String addressBookSourceId, String addressBookName) {
        if (!addressBookSourceId.isEmpty()) {
            synchronized (AACSContactsService.class) {
                if (mExecutor.isShutdown()) {
                    Log.w(TAG, " Executor has shutdown, recreate executor");
                    mExecutor = Executors.newSingleThreadExecutor();
                }
                mExecutor.submit(() -> {
                    Log.i(TAG,
                            String.format(
                                    "Adding %s AddressBook expecting confirmation from engine.", addressBookSourceId));
                    mPhoneBookController.addAddressBook(addressBookSourceId, addressBookName);
                    mExecutor.shutdown();
                });
            }
        } else {
            Log.e(TAG, "addressBookSourceId is empty");
        }
    }

    public void remove(String addressBookSourceId) {
        if (!addressBookSourceId.isEmpty()) {
            synchronized (AACSContactsService.class) {
                if (mExecutor.isShutdown()) {
                    Log.w(TAG, " Executor has shutdown, recreate executor");
                    mExecutor = Executors.newSingleThreadExecutor();
                }
                mExecutor.submit(() -> {
                    Log.i(TAG,
                            String.format("Removing %s AddressBook expecting confirmation from engine.",
                                    addressBookSourceId));
                    mPhoneBookController.removeAddressBook(addressBookSourceId);
                    mExecutor.shutdown();
                });
            }
        } else {
            Log.e(TAG, "addressBookSourceId is empty");
        }
    }

    private void startAACSContactsService() {
        if (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel notificationChannel = new NotificationChannel(
                    CONTACTS_CHANNEL_ID, CONTACTS_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);

            if (notificationManager != null) {
                notificationManager.createNotificationChannel(notificationChannel);
            }
        }

        // Create notification. Notification channel id is ignored in Android versions below O.
        Log.d(TAG, "Build foreground notification");
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CONTACTS_CHANNEL_ID)
                                                     .setContentTitle(this.getString(R.string.aacs_contacts_title))
                                                     .setContentText(this.getString(R.string.aacs_contacts_description))
                                                     .setSmallIcon(R.drawable.alexa_notification_icon)
                                                     .setStyle(new NotificationCompat.BigTextStyle());

        Notification notification = builder.build();

        startForeground(Constants.CONTACTS_SERVICE_NOTIFICATION, notification);
    }

    private void stopAACSContactsService() {
        Log.i(TAG, "Stopping AACS Contacts Service");
        stopSelf();
    }
}