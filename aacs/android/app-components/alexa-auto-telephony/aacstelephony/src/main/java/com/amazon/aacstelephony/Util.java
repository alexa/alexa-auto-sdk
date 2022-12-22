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

import static com.amazon.aacstelephony.Constants.HEADSET_CLIENT_PROFILE_ID;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.telecom.Call;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.util.Preconditions;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TelephonyConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.IPCUtils;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.aacs.common.AACSComponentRegistryUtil;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONStringer;

import java.util.HashMap;
import java.util.List;

public class Util {
    private static final String TAG = AACSConstants.AACS + "-" + Util.class.getSimpleName();

    @SuppressLint("RestrictedApi")
    public static void saveMessagingConsent(
            Context context, boolean value, String deviceAddress, String primaryAddress) {
        Log.d(TAG,
                "SMS Consent set to " + value + " deviceAddress: " + deviceAddress
                        + " primaryAddress: " + primaryAddress);
        if (primaryAddress.isEmpty() || deviceAddress.equals(primaryAddress)) {
            Preconditions.checkNotNull(context);
            SharedPreferences messagingConsentSharedPreferences =
                    context.getSharedPreferences(Constants.MESSAGING_CONSENT_FILE_NAME, context.MODE_PRIVATE);

            // Commit device address first
            messagingConsentSharedPreferences.edit().putString("device", deviceAddress).commit();
            messagingConsentSharedPreferences.edit().putBoolean("consent", value).commit();
        } else {
            Log.v(TAG, "Not primary device. Skipping notifying consent notification");
        }
    }

    public static String getPrimaryDevice(Context context) {
        String deviceAddress = "";
        TelecomManager telecomManager = (TelecomManager) context.getSystemService(Context.TELECOM_SERVICE);
        if (telecomManager != null) {
            PhoneAccountHandle handle = telecomManager.getDefaultOutgoingPhoneAccount(PhoneAccount.SCHEME_TEL);
            if (handle != null) {
                deviceAddress = handle.getId();
            }
        }
        if (deviceAddress.isEmpty() || !BluetoothAdapter.checkBluetoothAddress(deviceAddress)) {
            Log.e(TAG, "cannot find any valid primary device.");
            return null;
        }
        return deviceAddress;
    }

    static void publishCallStateToAACS(
            int state, @NonNull AACSMessageSender aacsMessageSender, @NonNull String callId, @NonNull String callerId) {
        Log.d(TAG, "Publish call state to AACS " + state);
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(TelephonyConstants.STATE)
                                     .value(getAACSCallState(state))
                                     .key(TelephonyConstants.CALLID)
                                     .value(callId)
                                     .key(TelephonyConstants.CALLERID)
                                     .value(callerId)
                                     .endObject()
                                     .toString();
            aacsMessageSender.sendMessage(
                    Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.CALL_STATE_CHANGED, payload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create callStateChanged JSON payload.");
        }
    }

    static void publishConnectionStateToAACS(@NonNull String state, @NonNull AACSMessageSender aacsMessageSender) {
        try {
            String payload =
                    new JSONStringer().object().key(TelephonyConstants.STATE).value(state).endObject().toString();
            aacsMessageSender.sendMessage(
                    Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.CONNECTION_STATE_CHANGED, payload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create connectionStateChanged JSON payload.");
        }
    }

    static void publishMessagingEndpointStateToAACS(
            @NonNull Context context, @NonNull AACSMessageSender aacsMessageSender) {
        SharedPreferences messagingPreferences =
                context.getSharedPreferences(MessagingConstants.PREFERENCES_FILE_NAME, context.MODE_PRIVATE);
        boolean consent = messagingPreferences.getBoolean(MessagingConstants.CONSENT_PROPERTY, false);
        String state = messagingPreferences.getString(
                MessagingConstants.STATE_PROPERTY, Constants.ConnectionState.DISCONNECTED);
        publishMessagingEndpointStateToAACS(context, aacsMessageSender, state, consent);
    }

    static void publishMessagingEndpointStateToAACS(
            @NonNull Context context, @NonNull AACSMessageSender aacsMessageSender, String state, boolean consent) {
        try {
            Log.d(TAG, "Publish messaging endpoint state to aacs connection state: " + state + " consent: " + consent);
            String permissionValue = consent ? MessagingConstants.PERMISSION_ON : MessagingConstants.PERMISSION_OFF;
            String payload = new JSONStringer()
                                     .object()
                                     .key(MessagingConstants.CONNECTION_STATE)
                                     .value(state)
                                     .key(MessagingConstants.SEND_PERMISSION)
                                     .value(permissionValue)
                                     .key(MessagingConstants.READ_PERMISSION)
                                     .value(permissionValue)
                                     .endObject()
                                     .toString();

            aacsMessageSender.sendMessage(Topic.MESSAGING, Action.Messaging.UPDATE_MESSAGING_ENDPOINT_STATE, payload);

        } catch (Exception e) {
            Log.e(TAG, "failed to create connectionStateChanged JSON payload.");
        }
    }

    static void publishMessagingEndpointStateToAACS(
            @NonNull String state, @NonNull Context context, @NonNull AACSMessageSender aacsMessageSender) {
        Log.d(TAG, "Update messaging connection state: " + state);
        SharedPreferences.Editor messagingPreferences =
                context.getSharedPreferences(MessagingConstants.PREFERENCES_FILE_NAME, context.MODE_PRIVATE).edit();
        messagingPreferences.putString(MessagingConstants.STATE_PROPERTY, state);
        messagingPreferences.commit();
        publishMessagingEndpointStateToAACS(context, aacsMessageSender);
    }

    static String getAACSCallState(int state) {
        switch (state) {
            case Call.STATE_DIALING:
                return Constants.CallState.DIALING;
            case Call.STATE_ACTIVE:
                return Constants.CallState.ACTIVE;
            case Call.STATE_RINGING:
                return Constants.CallState.INBOUND_RINGING;
            case Call.STATE_DISCONNECTED:
                return Constants.CallState.IDLE;
            default:
                return null;
        }
    }

    static void reportSendDTMFResult(boolean isSuccessful, @NonNull AACSMessageSender aacsMessageSender,
            @NonNull String callId, String errorCode, String errorMessage) {
        String payload;
        try {
            if (isSuccessful) {
                payload =
                        new JSONStringer().object().key(TelephonyConstants.CALLID).value(callId).endObject().toString();
                aacsMessageSender.sendMessage(
                        Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.SEND_DTMF_SUCCEEDED, payload);
            } else {
                payload = new JSONStringer()
                                  .object()
                                  .key(TelephonyConstants.CALLID)
                                  .value(callId)
                                  .key(Constants.CODE)
                                  .value(errorCode)
                                  .key(Constants.MESSAGE)
                                  .value(errorMessage)
                                  .endObject()
                                  .toString();
                aacsMessageSender.sendMessage(
                        Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.SEND_DTMF_FAILED, payload);
            }
        } catch (Exception e) {
            Log.e(TAG, "failed to create reporting sendDTMF result JSON payload." + e.toString());
        }
    }

    static void reportCallFailure(@NonNull AACSMessageSender aacsMessageSender, @NonNull String callId,
            @NonNull String errorCode, String errorMessage) {
        String payload;
        try {
            payload = new JSONStringer()
                              .object()
                              .key(TelephonyConstants.CALLID)
                              .value(callId)
                              .key(Constants.CODE)
                              .value(errorCode)
                              .key(Constants.MESSAGE)
                              .value(errorMessage)
                              .endObject()
                              .toString();
            aacsMessageSender.sendMessage(Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.CALL_FAILED, payload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create reporting call failure JSON payload." + e.toString());
        }
    }

    static boolean bluetoothNotConnected() {
        return BluetoothAdapter.getDefaultAdapter().getProfileConnectionState(HEADSET_CLIENT_PROFILE_ID)
                != BluetoothAdapter.STATE_CONNECTED;
    }

    @SuppressLint("NewApi")
    public static String getBluetoothDeviceName(@NonNull Context context, String deviceAddress) {
        String deviceName = "";
        if (deviceAddress != null && !deviceAddress.isEmpty()) {
            BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
            BluetoothAdapter adapter = manager.getAdapter();
            BluetoothDevice device = adapter.getRemoteDevice(deviceAddress);
            if (device != null) {
                deviceName = device.getName();
            }
        }

        return deviceName;
    }

    static void broadcastBluetoothState(
            @NonNull Context context, @NonNull boolean isConnected, String deviceName, String deviceAddress) {
        String action = TelephonyConstants.ACTION_BLUETOOTH_STATE_DISCONNECTED;
        if (isConnected)
            action = TelephonyConstants.ACTION_BLUETOOTH_STATE_CONNECTED;

        Intent intent = new Intent(action);
        intent.addCategory(TelephonyConstants.CATEGORY_AACS_TELEPHONY);
        intent.putExtra("deviceName", deviceName);
        intent.putExtra("deviceAddress", deviceAddress);

        List<TargetComponent> listeners =
                AACSComponentRegistryUtil.queryPackageManager(context, intent, Constants.AACS_TELEPHONY_PERMISSION);

        if (listeners == null) {
            Log.e(TAG, "No listeners to the bluetooth state change events");
            return;
        }

        sendNonAASBIntent(context, listeners, intent);
    }

    static void broadcastConnectionCheckCompleted(@NonNull Context context) {
        String action = TelephonyConstants.ACTION_BLUETOOTH_STATE_CONNECTION_CHECK_COMPLETED;
        Intent intent = new Intent(action);
        intent.addCategory(TelephonyConstants.CATEGORY_AACS_TELEPHONY);

        List<TargetComponent> listeners =
                AACSComponentRegistryUtil.queryPackageManager(context, intent, Constants.AACS_TELEPHONY_PERMISSION);

        if (listeners == null) {
            Log.e(TAG, "No listeners to the bluetooth connection check completed events");
            return;
        }

        sendNonAASBIntent(context, listeners, intent);
    }

    static void broadcastPairedDevices(@NonNull Context context, String deviceName, String deviceAddress) {
        String action = TelephonyConstants.ACTION_PAIRED_DEVICE;
        Intent intent = new Intent(action);
        intent.putExtra("deviceName", deviceName);
        intent.putExtra("deviceAddress", deviceAddress);
        intent.addCategory(TelephonyConstants.CATEGORY_AACS_TELEPHONY);

        List<TargetComponent> listeners =
                AACSComponentRegistryUtil.queryPackageManager(context, intent, Constants.AACS_TELEPHONY_PERMISSION);

        if (listeners == null) {
            Log.e(TAG, "No listeners to the broadcast paired devices events");
            return;
        }

        sendNonAASBIntent(context, listeners, intent);
    }

    static void broadcastBondState(
            @NonNull Context context, String deviceName, String deviceAddress, Integer bondState) {
        Intent intent = new Intent(Constants.AACS_BOND_STATE_CHANGED);
        intent.putExtra("deviceName", deviceName);
        intent.putExtra("deviceAddress", deviceAddress);
        intent.putExtra("bondState", bondState);
        intent.addCategory(TelephonyConstants.CATEGORY_AACS_TELEPHONY);

        List<TargetComponent> listeners =
                AACSComponentRegistryUtil.queryPackageManager(context, intent, Constants.AACS_TELEPHONY_PERMISSION);

        if (listeners == null) {
            Log.w(TAG, "No listeners to the bond state bluetooth connection check completed events");
            return;
        }

        sendNonAASBIntent(context, listeners, intent);
    }

    static void checkAndReportCurrentCalls(@NonNull HashMap<String, CallMap.CallInfo> callIdCallInfoMap,
            @NonNull AACSMessageSender aacsMessageSender) {
        for (String callId : callIdCallInfoMap.keySet()) {
            if (callId.equals(Constants.CallState.INBOUND_RINGING) || callId.equals(Constants.CallState.DIALING)
                    || callId.equals(Constants.CallState.ACTIVE)) {
                sendQueryCallIdRequest(aacsMessageSender);
            } else {
                CallMap.CallInfo callInfo = callIdCallInfoMap.get(callId);
                if (callInfo == null) {
                    Log.e(TAG, "No call info found with call Id " + callId);
                    continue;
                }

                String callerId = callInfo.getCallerId();
                Call currentCall = callInfo.getCall();
                if (currentCall == null) {
                    Log.e(TAG, "No call found with call Id " + callId);
                    continue;
                }

                Util.publishCallStateToAACS(currentCall.getState(), aacsMessageSender, callId, callerId);
            }
        }
    }

    static void sendQueryCallIdRequest(@NonNull AACSMessageSender aacsMessageSender) {
        Log.d(TAG, "queryCallId");
        try {
            String payload = new JSONStringer().object().endObject().toString();
            aacsMessageSender.sendMessage(
                    Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.CREATE_CALL_ID, payload);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create createCallId JSON payload." + e.getMessage());
        }
    }

    static void sendNonAASBIntent(
            @NonNull Context context, @NonNull List<TargetComponent> targets, @NonNull Intent intent) {
        for (TargetComponent target : targets) {
            intent.setComponent(target.component);
            intent.setPackage(target.packageName);
            switch (target.type) {
                case ACTIVITY:
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    context.startActivity(intent, null);
                    break;
                case SERVICE:
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                            && !IPCUtils.getInstance(context).isSystemApp()) {
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
    }

    public static void toast(@NonNull Context context, String message) {
        Handler handler = new Handler(Looper.getMainLooper());

        handler.post(() -> { Toast.makeText(context, message, Toast.LENGTH_SHORT).show(); });
    }
}