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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.os.Build;
import android.telecom.Call;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TelephonyConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.aacsipc.IPCUtils;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.aacs.common.AACSComponentRegistryUtil;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONStringer;

import java.lang.annotation.Target;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Util {
    private static final String TAG = AACSConstants.AACS + "-" + Util.class.getSimpleName();

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
}
