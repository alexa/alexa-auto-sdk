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

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.provider.ContactsContract;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.ContactsConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class PhoneBookController {
    private static final String TAG = AACSConstants.AACS + "-" + PhoneBookController.class.getSimpleName();
    private Context mContext;
    private AACSSender mAACSSender;
    private final TargetComponent mTarget;
    private AACSMessageSender mAACSMessageSender;
    public static HashMap<String, CompletableFuture<Boolean>> mAddressBookUpdateToFutureMap = new HashMap<>();
    private final int REPLY_WAIT_DURATION = 5000;
    private ExecutorService mExecutor;

    public PhoneBookController(@NonNull Context context) {
        mAACSSender = new AACSSender();
        mContext = context;
        mExecutor = Executors.newSingleThreadExecutor();
        mTarget = TargetComponent.withComponent(new ComponentName("com.amazon.alexaautoclientservice",
                                                        "com.amazon.alexaautoclientservice.AlexaAutoClientService"),
                TargetComponent.Type.SERVICE);
        mAACSMessageSender = new AACSMessageSender(new WeakReference<>(context), new AACSSender());
    }

    PhoneBookController(@NonNull Context context, AACSMessageSender messageSender) {
        mAACSMessageSender = messageSender;
        mContext = context;
        mExecutor = Executors.newSingleThreadExecutor();
        mTarget = TargetComponent.withComponent(new ComponentName("com.amazon.alexaautoclientservice",
                                                        "com.amazon.alexaautoclientservice.AlexaAutoClientService"),
                TargetComponent.Type.SERVICE);
    }

    /**
     * API called by OEM's app if AACS Contacts Library is included in their app.
     *
     * @param addressBookSourceId A unique identifier for an address book.
     * @param addressBookName Friendly name of the address book, or an empty string if not available.
     */
    public boolean uploadContacts(String addressBookSourceId, String addressBookName) {
        Log.d(TAG, "uploadContacts");
        // Retrieving a list of contacts stored in Contacts Provider through PBAP connection
        // Parsing the contacts data and wrap it into AASB intent payload
        // Sending the message to AACS with contacts.
        CompletableFuture<Boolean> waitForAddressBookAdd = new CompletableFuture<>();
        String messageId = addAddressBook(addressBookSourceId, addressBookName);
        mAddressBookUpdateToFutureMap.put(messageId, waitForAddressBookAdd);
        try {
            if (mAddressBookUpdateToFutureMap.get(messageId).get(REPLY_WAIT_DURATION, TimeUnit.MILLISECONDS)) {
                mAddressBookUpdateToFutureMap.remove(messageId);
                Log.i(TAG, Topic.ADDRESS_BOOK + addressBookSourceId + " added successful");
                return true;
            }
        } catch (ExecutionException | InterruptedException e) {
            Log.e(TAG, "Error occurred during wait task execution: " + e.getMessage());
        } catch (CancellationException | TimeoutException e) {
            Log.e(TAG, "Stopping wait for " + addressBookSourceId + " addressBook adding");
        }
        return false;
    }

    /**
     * API called by OEM's app if AACS Contacts Library is included in their app.
     *
     * @param addressBookSourceId The corresponding address book to be removed from cloud.
     */
    public boolean removeContacts(String addressBookSourceId) {
        Log.d(TAG, "removeContacts");
        CompletableFuture<Boolean> waitForAddressBookAdd = new CompletableFuture<>();
        String messageId = removeAddressBook(addressBookSourceId);
        mAddressBookUpdateToFutureMap.put(messageId, waitForAddressBookAdd);
        try {
            if (mAddressBookUpdateToFutureMap.get(messageId).get(REPLY_WAIT_DURATION, TimeUnit.MILLISECONDS)) {
                mAddressBookUpdateToFutureMap.remove(messageId);
                Log.i(TAG, Topic.ADDRESS_BOOK + addressBookSourceId + " removed successful");
                return true;
            }
        } catch (ExecutionException | InterruptedException e) {
            Log.e(TAG, "Error occurred during wait task execution: " + e.getMessage());
        } catch (CancellationException | TimeoutException e) {
            Log.e(TAG, "Stopping wait for " + addressBookSourceId + " addressBook removing");
        }
        return false;
    }

    /**
     * Called by Contacts service when it receives intent from OEM app to start uploading contacts.
     *
     * @param addressBookSourceId A unique identifier for an address book.
     * @param addressBookName Friendly name of the address book, or an empty string if not available.
     */
    String addAddressBook(String addressBookSourceId, String addressBookName) {
        Log.d(TAG, "addAddressBook");

        Boolean hasContacts = false;
        JSONObject addrBookData = new JSONObject();
        JSONArray contactsNames = new JSONArray();
        JSONArray phoneData = new JSONArray();
        JSONObject payload = new JSONObject();
        String messageId = "";
        Log.i(TAG, "Start Retrieving Contacts");

        ContentResolver contentResolver = mContext.getContentResolver();
        Cursor nameCur =
                contentResolver.query(ContactsContract.Data.CONTENT_URI, null, ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE}, null);
        Cursor nickCur =
                contentResolver.query(ContactsContract.Data.CONTENT_URI, null, ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.Nickname.CONTENT_ITEM_TYPE}, null);

        if ((nameCur != null ? nameCur.getCount() : 0) > 0) {
            hasContacts = true;
            while (nameCur != null && nameCur.moveToNext()) {
                String contacts_source =
                        nameCur.getString(nameCur.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_NAME));
                if (!contacts_source.equals(addressBookSourceId)) {
                    // Skip contacts without matching source id
                    continue;
                }
                JSONObject contact = new JSONObject();
                String id = nameCur.getString(nameCur.getColumnIndex(ContactsContract.Data.CONTACT_ID));
                String firstName = nameCur.getString(
                        nameCur.getColumnIndex(ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME));
                String lastName = nameCur.getString(
                        nameCur.getColumnIndex(ContactsContract.CommonDataKinds.StructuredName.FAMILY_NAME));
                Cursor phoneCur = contentResolver.query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null,
                        ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = ?", new String[] {id}, null);

                if (firstName == null) {
                    firstName = "";
                }
                if (lastName == null) {
                    lastName = "";
                }
                try {
                    contact.put(Constants.ENTRY, id);
                    contact.put(Constants.FIRST_NAME, firstName);
                    contact.put(Constants.LAST_NAME, lastName);
                } catch (JSONException e) {
                    Log.e(TAG, "Error while creating contactNames payload.");
                }

                if (nickCur != null && nickCur.moveToNext()) {
                    String nickName =
                            nickCur.getString(nickCur.getColumnIndex(ContactsContract.CommonDataKinds.Nickname.NAME));
                    if (nickName == null) {
                        nickName = "";
                    }
                    try {
                        contact.put(Constants.NICK_NAME, nickName);
                    } catch (JSONException e) {
                        Log.e(TAG, "Error while creating nickName payload.");
                    }
                } else {
                    try {
                        contact.put(Constants.NICK_NAME, "");
                    } catch (JSONException e) {
                        Log.e(TAG, "Error while creating nickName payload.");
                    }
                }

                contactsNames.put(contact);

                while (phoneCur != null && phoneCur.moveToNext()) {
                    JSONObject phone = new JSONObject();
                    String phoneNumber =
                            phoneCur.getString(phoneCur.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                    int phoneType =
                            phoneCur.getInt(phoneCur.getColumnIndex(ContactsContract.CommonDataKinds.Phone.TYPE));
                    String typeLabel =
                            Integer.toString(ContactsContract.CommonDataKinds.Phone.getTypeLabelResource(phoneType));
                    CharSequence phoneLabel = ContactsContract.CommonDataKinds.Phone.getTypeLabel(
                            mContext.getResources(), phoneType, typeLabel);

                    try {
                        phone.put(Constants.ENTRY, id);
                        phone.put(Constants.LABEL, phoneLabel);
                        phone.put(Constants.NUMBER, phoneNumber);
                    } catch (JSONException e) {
                        Log.e(TAG, "Error while creating phoneData payload.");
                    }
                    phoneData.put(phone);
                }
                phoneCur.close();
            }
        }

        if (nameCur != null) {
            nameCur.close();
        }
        if (nickCur != null) {
            nickCur.close();
        }

        if (hasContacts) {
            try {
                addrBookData.put(Constants.NAV_NAMES, new JSONArray());
                addrBookData.put(Constants.CONTACT_NAMES, contactsNames);
                addrBookData.put(Constants.PHONE_DATA, phoneData);
                addrBookData.put(Constants.POSTAL_ADDR, new JSONArray());
                payload.put(ContactsConstants.ADDRESS_BOOK_SOURCE_ID, addressBookSourceId);
                payload.put(Constants.NAME, addressBookName);
                payload.put(Constants.TYPE, Constants.CONTACT_TYPE);
                payload.put(Constants.ADDRESS_BOOK_DATA, addrBookData);
                messageId = mAACSMessageSender.sendMessageReturnID(
                        Topic.ADDRESS_BOOK, Action.AddressBook.ADD_ADDRESS_BOOK, payload.toString());
            } catch (JSONException e) {
                Log.e(TAG, "Error while creating contacts message payload.");
            }
        } else {
            Log.w(TAG, "No contacts to be uploaded");
        }
        return messageId;
    }

    /**
     * Called by Contacts service when it receives intent from OEM app to remove contacts.
     * This method is not designed to be called by client application
     *
     * @param addressBookSourceId The corresponding address book to be removed from cloud.
     */
    String removeAddressBook(String addressBookSourceId) {
        Log.d(TAG, "removeAddressBook");

        JSONObject payload = new JSONObject();
        String messageId = "";
        try {
            payload.put(ContactsConstants.ADDRESS_BOOK_SOURCE_ID, addressBookSourceId);
            messageId = mAACSMessageSender.sendMessageReturnID(
                    Topic.ADDRESS_BOOK, Action.AddressBook.REMOVE_ADDRESS_BOOK, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Error while creating removeAddressBook message payload.");
        }
        return messageId;
    }

    /**
     * Resolve CompletableFuture set by Contacts Service
     * This method is not designed to be called by client application.
     *
     */
    void completeAddressBookUpdate(String id, boolean success) {
        Log.d(TAG, "completeAddressBookUpdate");

        synchronized (PhoneBookController.class) {
            if (mExecutor.isShutdown()) {
                Log.w(TAG, " Executor has shutdown, recreate executor");
                mExecutor = Executors.newSingleThreadExecutor();
            }
            mExecutor.submit(() -> {
                if (mAddressBookUpdateToFutureMap.containsKey(id)) {
                    mAddressBookUpdateToFutureMap.get(id).complete(success);
                } else {
                    Log.e(TAG, String.format("AddressBook Updating with replyToId: %s not found.", id));
                }
            });
        }
    }
}