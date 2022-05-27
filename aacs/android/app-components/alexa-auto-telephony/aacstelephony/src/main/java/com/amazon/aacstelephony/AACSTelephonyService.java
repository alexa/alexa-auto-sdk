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

package com.amazon.aacstelephony;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.telecom.PhoneAccountHandle;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TelephonyConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.aacs.common.AACSComponentRegistryUtil;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

public class AACSTelephonyService extends Service {
    private static final String TAG = AACSConstants.AACS + "-" + AACSTelephonyService.class.getSimpleName();
    private static final String TELEPHONY_CHANNEL_ID = "com.amazon.aacstelephony";
    private static final String TELEPHONY_CHANNEL_NAME = "AACSTelephony";
    private final ServiceLifecycle mServiceLifecycle;
    private AACSMessageSender mAACSMessageSender;
    private PhoneCallController mPhoneCallController;
    private BluetoothStateListener mBluetoothStateListener;

    public AACSTelephonyService() {
        mServiceLifecycle = new ServiceLifecycle();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "AACS Telephony Service created");
        mAACSMessageSender = new AACSMessageSender(new WeakReference<>(this), new AACSSender());
        mPhoneCallController = new PhoneCallController(this, mAACSMessageSender);
        initializeBluetoothStateListener();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null) {
            Log.d(TAG,
                    String.format(
                            "Receiving intent: action=%s, category=%s", intent.getAction(), intent.getCategories()));
            mServiceLifecycle.startServiceAndCheckBusyState();
            handleIntent(intent);
        }
        return Service.START_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void onDestroy() {
        super.onDestroy();

        // clean up call map
        mPhoneCallController.cleanUp();

        // unregister bluetooth state listener
        unregisterReceiver(mBluetoothStateListener);
        Log.i(TAG, "AACS Telephony Service destroyed");
    }

    private void handleIntent(Intent intent) {
        if (TelephonyConstants.ACTION_UPDATE_DEVICE_COFIGURATION.equals(intent.getAction())) {
            String property = intent.getStringExtra(TelephonyConstants.DEVICE_PROPERTY);
            boolean enable = intent.getBooleanExtra(TelephonyConstants.ENABLE, false);
            mPhoneCallController.updateDeviceConfiguration(property, enable);
            return;
        }

        if (Constants.ACTION_CANCEL_IDLE_TIMER.equals(intent.getAction())) {
            mServiceLifecycle.markServiceBusy();
            return;
        }

        if (Constants.ACTION_RESET_IDLE_TIMER.equals(intent.getAction())) {
            mServiceLifecycle.markServiceIdle();
            return;
        }

        Optional<AACSMessage> messageOptional = AACSMessageBuilder.parseEmbeddedIntent(intent);

        if (messageOptional.isPresent()) {
            AACSMessage message = messageOptional.get();
            if (Topic.PHONE_CALL_CONTROLLER.equals(message.topic)) {
                switch (message.action) {
                    case Action.PhoneCallController.DIAL:
                        queryPhoneAccount(message.payload, false);
                        break;
                    case Action.PhoneCallController.REDIAL:
                        queryPhoneAccount(message.payload, true);
                        break;
                    case Action.PhoneCallController.ANSWER:
                        answer(message.payload);
                        break;
                    case Action.PhoneCallController.STOP:
                        stop(message.payload);
                        break;
                    case Action.PhoneCallController.SEND_DTMF:
                        sendDTMF(message.payload);
                        break;
                    case Action.PhoneCallController.CREATE_CALL_ID:
                        handleCreateCallIdReply(message.payload);
                        break;
                }
            } else if (Topic.AASB.equals(message.topic)) {
                if (Action.AASB.START_SERVICE.equals(message.action)) {
                    Log.d(TAG, "AACS started. ");
                    mBluetoothStateListener.initialConnectionCheck(this);
                    mPhoneCallController.initialCallStateCheck();
                } else if (Action.AASB.STOP_SERVICE.equals(message.action)) {
                    Log.d(TAG, "AACS stopped, stopping AACS Telephony Service if no active call");
                    mServiceLifecycle.shutdownServiceIfNoCall();
                }
            }
        }
    }

    public void dial(PhoneAccountHandle handle, String callId, String callNumber) {
        mPhoneCallController.dial(callId, callNumber, handle);
    }

    public void queryPhoneAccount(String payload, Boolean isRedial) {
        PhoneAccountHandle selectedPhoneAccount = mPhoneCallController.getPhoneAccount();
        try {
            String callNumber = "";
            String payloadString = new JSONObject(payload).getString(AACSConstants.PAYLOAD);
            JSONObject obj = new JSONObject(payloadString);
            String callId = obj.getString(TelephonyConstants.CALLID);
            if (obj.has("callee")) {
                JSONObject callee = obj.getJSONObject("callee");
                JSONObject defaultAddress = callee.getJSONObject("defaultContactAddress");
                if (defaultAddress != null) {
                    callNumber = defaultAddress.getString("value");
                }
            }
            if (!isRedial) {
                dial(selectedPhoneAccount, callId, callNumber);
            } else {
                redial(selectedPhoneAccount, callId);
            }
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing Dial directive payload: " + e.getMessage());
        }
    }

    public void redial(PhoneAccountHandle handle, String callId) {
        mPhoneCallController.redial(callId, handle);
    }

    public void answer(String payload) {
        try {
            String payloadString = new JSONObject(payload).getString(AACSConstants.PAYLOAD);
            JSONObject obj = new JSONObject(payloadString);
            String callId = obj.getString(TelephonyConstants.CALLID);
            mPhoneCallController.answer(callId);
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing Answer directive payload: " + e.getMessage());
        }
    }

    public void stop(String payload) {
        try {
            String payloadString = new JSONObject(payload).getString(AACSConstants.PAYLOAD);
            JSONObject obj = new JSONObject(payloadString);
            String callId = obj.getString(TelephonyConstants.CALLID);
            mPhoneCallController.stop(callId);
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing Stop directive payload: " + e.getMessage());
        }
    }

    public void sendDTMF(String payload) {
        try {
            String payloadString = new JSONObject(payload).getString(AACSConstants.PAYLOAD);
            JSONObject obj = new JSONObject(payloadString);
            String callId = obj.getString(TelephonyConstants.CALLID);
            String signal = obj.getString("signal");
            mPhoneCallController.sendDTMF(callId, signal);
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing SendDTMF directive payload: " + e.getMessage());
        }
    }

    public void handleCreateCallIdReply(String payload) {
        Log.i(TAG, "Received create call id reply from engine " + payload);
        try {
            String callId = new JSONObject(payload).getString(TelephonyConstants.CALLID);
            mPhoneCallController.callIdReceived(callId);
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing CreateCallId reply payload: " + e.getMessage());
        }
    }

    private void initializeBluetoothStateListener() {
        mBluetoothStateListener = new BluetoothStateListener(mAACSMessageSender);
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(Constants.ACTION_BLUETOOTH_PBAP_CLIENT_STATE_CHANGED);
        filter.addAction(Constants.ACTION_BLUETOOTH_HFP_CLIENT_STATE_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);

        // BluetoothStateListener run on another thread
        HandlerThread handlerThread = new HandlerThread("BluetoothStateListener");
        handlerThread.start();
        this.registerReceiver(mBluetoothStateListener, filter, null, new Handler(handlerThread.getLooper()));
    }

    private void startAACSTelephonyService() {
        if (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel notificationChannel = new NotificationChannel(
                    TELEPHONY_CHANNEL_ID, TELEPHONY_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);

            if (notificationManager != null) {
                notificationManager.createNotificationChannel(notificationChannel);
            }
        }

        // Create notification. Notification channel id is ignored in Android versions below O.
        Log.d(TAG, "Build foreground notification");
        NotificationCompat.Builder builder =
                new NotificationCompat.Builder(this, TELEPHONY_CHANNEL_ID)
                        .setContentTitle(this.getString(R.string.aacs_telephony_title))
                        .setContentText(this.getString(R.string.aacs_telephony_description))
                        .setSmallIcon(R.drawable.alexa_notification_icon)
                        .setStyle(new NotificationCompat.BigTextStyle());

        Notification notification = builder.build();

        startForeground(1, notification);
    }

    private void stopAACSTelephonyService() {
        Log.i(TAG, "Stopping AACS Telephony Service");
        stopSelf();
    }

    public class ServiceLifecycle {
        private boolean mServiceStarted = false;
        private boolean mServiceIsBusy = false;

        void markServiceBusy() {
            Log.v(TAG, "Marking service busy. Service would not stop until marked idle");
            mServiceIsBusy = true;
        }

        void markServiceIdle() {
            Log.v(TAG, "Marking service idle.");
            mServiceIsBusy = false;
        }

        void shutdownServiceIfNoCall() {
            if (mServiceIsBusy) {
                return;
            }

            HashMap<String, CallMap.CallInfo> callIdCallInfoMap =
                    mPhoneCallController.getCallMap().getCallIdCallInfoMap();
            if (callIdCallInfoMap.size() == 0) {
                Log.v(TAG, "No ongoing call. Marking service idle. Service would stop immediately");
                mServiceIsBusy = false;
                stopAACSTelephonyService();
                mServiceStarted = false;
            } else {
                mServiceIsBusy = true;
            }
        }

        void startServiceAndCheckBusyState() {
            if (!mServiceStarted) {
                startAACSTelephonyService();
                mServiceStarted = true;
            }
        }
    }
}
