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

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.CallLog;
import android.provider.ContactsContract;
import android.telecom.Call;
import android.telecom.InCallService;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TelephonyConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONStringer;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class PhoneCallController extends InCallService {
    private static final String TAG = AACSConstants.AACS + "-"
            + "AACSTelephonyService"
            + "-" + PhoneCallController.class.getSimpleName();
    private Context mTelephonyServiceContext;
    private TelecomManager mTelecomManager;
    private AACSMessageSender mAACSMessageSender;
    private static CallMap mCallMap;

    /**
     * Constructor for the system when binding InCallService
     */
    public PhoneCallController() {
        Log.i(TAG, "InCallService created");
        AACSSender aacsSender = new AACSSender();
        mAACSMessageSender = new AACSMessageSender(new WeakReference<>(this), aacsSender);
    }

    /**
     * Constructor for any non-system components
     * @param context Context of AACSTelephonyService
     */
    public PhoneCallController(@NonNull Context context, @NonNull AACSMessageSender aacsMessageSender) {
        mTelephonyServiceContext = context;
        mAACSMessageSender = aacsMessageSender;
        if (mCallMap == null)
            mCallMap = new CallMap();
        if (mTelecomManager == null)
            mTelecomManager = (TelecomManager) context.getSystemService(Context.TELECOM_SERVICE);
    }

    PhoneCallController(
            @NonNull Context context, @NonNull AACSMessageSender aacsMessageSender, TelecomManager telecomManager) {
        this(context, aacsMessageSender);
        if (telecomManager != null)
            mTelecomManager = telecomManager;
    }

    /**
     * Platform implementation for Alexa initiating a call
     * Suppressing the "MissingPermission" error as AACS currently doesn't request any permissions from the user
     * @param callId Call Id in the PhoneCallController.Dial directive
     * @param callNumber Number to be dialed
     * @param handle Phone account handle to be used to make the call
     */
    @SuppressLint("MissingPermission")
    void dial(String callId, String callNumber, PhoneAccountHandle handle) {
        Log.d(TAG, "dial");
        if (Util.bluetoothNotConnected()) {
            Log.e(TAG, "Bluetooth disconnected, the call will not be initiated.");
            Util.reportCallFailure(
                    mAACSMessageSender, callId, TelephonyConstants.ErrorCode.OTHER, "Bluetooth disconnected");
            return;
        }

        if (handle == null) {
            Log.e(TAG, "Invalid phoneAccountHandle, the call will not be initiated.");
            Util.reportCallFailure(mAACSMessageSender, callId, TelephonyConstants.ErrorCode.NO_CARRIER, "");
            return;
        }

        mCallMap.addCall(null, callId, callNumber);
        Uri uri = Uri.fromParts(PhoneAccount.SCHEME_TEL, callNumber, null);
        Bundle extras = new Bundle();
        extras.putParcelable(TelecomManager.EXTRA_PHONE_ACCOUNT_HANDLE, handle);

        mTelecomManager.placeCall(uri, extras);
    }

    /**
     * Platform implementation for redialing
     * Suppressing the "MissingPermission" error as AACS currently doesn't request any permissions from the user
     * @param callId Call Id in the PhoneCallController.Redial directive
     * @param handle Phone account handle to be used to make the call
     */
    @SuppressLint("MissingPermission")
    void redial(String callId, PhoneAccountHandle handle) {
        Log.d(TAG, "redial");
        String callNumber = CallLog.Calls.getLastOutgoingCall(mTelephonyServiceContext);
        if (callNumber.isEmpty()) {
            Log.e(TAG, "Last dialed number is not available");
            Util.reportCallFailure(mAACSMessageSender, callId, TelephonyConstants.ErrorCode.NO_NUMBER_FOR_REDIAL, "");
            return;
        }

        dial(callId, callNumber, handle);
    }

    /**
     * Platform implementation for answering the current incoming call
     * @param callId Call Id of the call to be answered
     */
    void answer(String callId) {
        Log.d(TAG, "answer");
        if (Util.bluetoothNotConnected()) {
            Log.e(TAG, "Bluetooth disconnected");
            Util.reportCallFailure(
                    mAACSMessageSender, callId, TelephonyConstants.ErrorCode.OTHER, "Bluetooth disconnected");
            return;
        }

        Call currentCall = mCallMap.getCall(callId);
        if (currentCall == null) {
            Log.e(TAG, String.format("Call object with callId=%s not found", callId));
            Util.reportCallFailure(mAACSMessageSender, callId, TelephonyConstants.ErrorCode.OTHER,
                    "No matching call Id in current calls");
            return;
        }

        Log.v(TAG, "Answering call with callId " + callId);
        currentCall.answer(0);
    }

    /**
     * Platform implementation for Alexa stopping the current call
     * @param callId Call with this Call Id to be stopped
     */
    void stop(String callId) {
        Log.d(TAG, "stop");
        if (Util.bluetoothNotConnected()) {
            Log.e(TAG, "Bluetooth disconnected");
            Util.reportCallFailure(
                    mAACSMessageSender, callId, TelephonyConstants.ErrorCode.OTHER, "Bluetooth disconnected");
            return;
        }

        Call currentCall = mCallMap.getCall(callId);
        if (currentCall == null) {
            Log.e(TAG, String.format("Call object with callId=%s not found", callId));
            Util.reportCallFailure(mAACSMessageSender, callId, TelephonyConstants.ErrorCode.OTHER,
                    "No matching call Id in current calls");
            return;
        }

        if (currentCall.getState() == Call.STATE_RINGING) {
            Log.v(TAG, "Rejecting the current incoming call..");
            currentCall.reject(false, null);
        } else {
            Log.v(TAG, "Stopping the current active call..");
            currentCall.disconnect();
        }
    }

    /**
     * Platform implementation for sending DTMF signal to the current call
     * @param callId Call with this Call Id to get the signal
     * @param signal The signal to be sent
     */
    void sendDTMF(String callId, String signal) {
        Log.d(TAG, "sendDTMF");
        if (Util.bluetoothNotConnected()) {
            Log.e(TAG, "Bluetooth disconnected");
            Util.reportSendDTMFResult(
                    false, mAACSMessageSender, callId, Constants.SendDTMFError.DTMF_FAILED, "Bluetooth disconnected");
            return;
        }

        Call currentCall = mCallMap.getCall(callId);
        if (currentCall == null) {
            Log.e(TAG, "Call object not found");
            Util.reportSendDTMFResult(
                    false, mAACSMessageSender, callId, Constants.SendDTMFError.CALL_NOT_IN_PROGRESS, "No active call");
            return;
        }

        Log.d(TAG, "Sending DTMF..." + signal);
        for (char digit : signal.toCharArray()) {
            currentCall.playDtmfTone(digit);
            currentCall.stopDtmfTone();
        }
        Util.reportSendDTMFResult(true, mAACSMessageSender, callId, null, null);
    }

    void callIdReceived(String callId) {
        Log.d(TAG, "callIdReceived");

        CallMap.CallInfo currentCallInfo = mCallMap.getCallInfoWithoutCallId();
        if (currentCallInfo == null) {
            Log.e(TAG, "No call that is expecting Call Id found");
            return;
        }

        String currentCallerId = currentCallInfo.getCallerId();
        Call currentCall = currentCallInfo.getCall();

        Util.publishCallStateToAACS(currentCall.getState(), mAACSMessageSender, callId, currentCallerId);
        Call.Callback callStateCallback =
                new CallStateListener(mTelephonyServiceContext, callId, currentCallerId, mAACSMessageSender);
        currentCall.registerCallback(callStateCallback);

        // Add call to the map
        mCallMap.addCallback(currentCall, callStateCallback);
        mCallMap.addCall(currentCall, callId, currentCallerId);
    }

    boolean initialCallStateCheck() {
        Log.d(TAG, "initialCallStateCheck");

        HashMap<String, CallMap.CallInfo> callIdCallInfoMap = mCallMap.getCallIdCallInfoMap();
        Log.i(TAG, "Active calls when AACS starts " + callIdCallInfoMap.size());
        if (callIdCallInfoMap.size() > 0) {
            Util.checkAndReportCurrentCalls(callIdCallInfoMap, mAACSMessageSender);
            return true;
        }

        return false;
    }

    /**
     * Public API which can either be called by the client or by AACSTelephonyService upon receiving
     * related intent from the client to update device configuration.
     * Currently Auto SDK only supports one property: DTMF_SUPPORTED (true/false)
     * @param deviceProperty
     * @param value
     */
    public void updateDeviceConfiguration(String deviceProperty, boolean value) {
        Log.d(TAG, "updateDeviceConfiguration");
        try {
            String configurationMap =
                    new JSONStringer().object().key(deviceProperty).value(value).endObject().toString();
            String payload =
                    new JSONStringer().object().key("configurationMap").value(configurationMap).endObject().toString();

            mAACSMessageSender.sendMessage(
                    Topic.PHONE_CALL_CONTROLLER, Action.PhoneCallController.DEVICE_CONFIGURATION_UPDATED, payload);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create createCallId JSON payload." + e.getMessage());
        }
    }

    public PhoneAccountHandle getPhoneAccount() {
        if (mTelephonyServiceContext.checkSelfPermission(android.Manifest.permission.READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(TAG, "Failed to get the outgoing phone account due to missing READ_PHONE_STATE permission");
            return null;
        }
        PhoneAccountHandle selectedHandle = mTelecomManager.getDefaultOutgoingPhoneAccount(PhoneAccount.SCHEME_TEL);

        // if getDefaultOutgoingPhoneAccount returns null, call getCallCapablePhoneAccounts to get all the valid phone
        // accounts. if there is only one valid account, then return the handle for that account. Otherwise, return
        // null.
        if (selectedHandle == null) {
            Log.i(TAG, "getDefaultOutgoingAccount returns null, calling getCallCapablePhoneAccounts");
            List<PhoneAccountHandle> validHandles = new ArrayList<>();
            for (PhoneAccountHandle handle : mTelecomManager.getCallCapablePhoneAccounts()) {
                PhoneAccount phoneAccount = mTelecomManager.getPhoneAccount(handle);
                if (phoneAccount == null)
                    continue;
                // verify the phone account is not an emergency account
                if (phoneAccount.getShortDescription() == null
                        || !phoneAccount.getShortDescription().toString().contains("Emergency")) {
                    validHandles.add(handle);
                }
            }
            if (validHandles.size() == 1)
                selectedHandle = validHandles.get(0);
        }
        return selectedHandle;
    }

    @Override
    public void onCallAdded(Call call) {
        Log.d(TAG, "onCallAdded");
        int callState = call.getState();

        if (mCallMap == null) {
            mCallMap = new CallMap();
        }

        // Incoming call
        if (callState == Call.STATE_RINGING) {
            Util.sendQueryCallIdRequest(mAACSMessageSender);
            String currentCallerId = call.getDetails().getHandle().getSchemeSpecificPart();
            mCallMap.addCall(call, Constants.CallState.INBOUND_RINGING, currentCallerId);

            Log.v(TAG, "New incoming call...");
            return;
        }

        // Already in an active call
        if (callState == Call.STATE_ACTIVE) {
            Util.sendQueryCallIdRequest(mAACSMessageSender);
            String currentCallerId = call.getDetails().getHandle().getSchemeSpecificPart();
            mCallMap.addCall(call, Constants.CallState.ACTIVE, currentCallerId);

            Log.v(TAG, "New active call...");
            return;
        }

        // Outgoing calls
        if (callState == Call.STATE_CONNECTING || callState == Call.STATE_DIALING) {
            CallMap.CallInfo outgoingCallInfo = mCallMap.getOutgoingCallInfo();
            Log.v(TAG, "New outgoing call...");

            if (outgoingCallInfo != null) {
                // Alexa initiates the call
                String callerId = outgoingCallInfo.getCallerId();
                String callId = outgoingCallInfo.getCallId();

                Call.Callback callStateCallback = new CallStateListener(this, callId, callerId, mAACSMessageSender);
                call.registerCallback(callStateCallback);

                mCallMap.setCall(callId, call);
                mCallMap.addCallback(call, callStateCallback);

            } else {
                // User initiates the call from head unit or phone
                Util.sendQueryCallIdRequest(mAACSMessageSender);
                String currentCallerId = call.getDetails().getHandle().getSchemeSpecificPart();
                mCallMap.addCall(call, Constants.CallState.DIALING, currentCallerId);
            }
        }
    }

    @Override
    public void onCallRemoved(Call call) {
        Log.d(TAG, "onCallRemoved");

        Call.Callback callStateCallback = mCallMap.removeCallback(call);
        mCallMap.removeCall(call);

        if (callStateCallback != null) {
            call.unregisterCallback(callStateCallback);
        } else {
            Log.w(TAG, "Callback is not found for the call");
        }

        HashMap<String, CallMap.CallInfo> callIdCallInfoMap = mCallMap.getCallIdCallInfoMap();
        if (callIdCallInfoMap.size() > 0) {
            Util.checkAndReportCurrentCalls(callIdCallInfoMap, mAACSMessageSender);
        }
    }

    CallMap getCallMap() {
        Log.d(TAG, "getCallMap");
        return mCallMap;
    }

    void cleanUp() {
        Log.d(TAG, "cleanUp");
        mCallMap.clear();
        mCallMap = null;
    }
}
