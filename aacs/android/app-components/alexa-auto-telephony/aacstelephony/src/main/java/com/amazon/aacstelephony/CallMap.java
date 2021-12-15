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

import android.telecom.Call;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;

import java.util.HashMap;

class CallMap {
    private static final String TAG = AACSConstants.AACS + "-"
            + "AACSTelephonyService"
            + "-" + CallMap.class.getSimpleName();

    private static HashMap<Call, Call.Callback> mCallStateCallbackMap;
    private static HashMap<String, CallInfo> mCallIdCallInfoMap;
    private static HashMap<Call, CallInfo> mCallInfoMap;

    CallMap() {
        mCallStateCallbackMap = new HashMap<>();
        mCallIdCallInfoMap = new HashMap<>();
        mCallInfoMap = new HashMap<>();
    }

    void addCall(Call call, String callId, String callerId) {
        Log.i(TAG, "Call added to CallMap...");
        CallInfo callInfo = new CallInfo(call, callerId, callId);
        if (call != null)
            mCallInfoMap.put(call, callInfo);
        if (callId != null)
            mCallIdCallInfoMap.put(callId, callInfo);
    }

    void removeCall(Call call) {
        CallInfo callInfo = mCallInfoMap.remove(call);

        if (callInfo == null) {
            Log.e(TAG, "CallInfo to be removed not found");
            return;
        }

        Log.i(TAG, "Call removed from CallMap...");
        String callId = callInfo.getCallId();
        mCallIdCallInfoMap.remove(callId);
    }

    Call getCall(String callId) {
        CallInfo callInfo = mCallIdCallInfoMap.get(callId);
        if (callInfo == null) {
            Log.e(TAG, "No call with matching callId available");
            return null;
        }

        return callInfo.getCall();
    }

    HashMap<String, CallInfo> getCallIdCallInfoMap() {
        return mCallIdCallInfoMap;
    }

    CallInfo getCallInfoWithoutCallId() {
        for (String callId : mCallIdCallInfoMap.keySet()) {
            if (callId.equals(Constants.CallState.INBOUND_RINGING) || callId.equals(Constants.CallState.DIALING)
                    || callId.equals(Constants.CallState.ACTIVE)) {
                CallInfo callInfo = mCallIdCallInfoMap.get(callId);
                Call call = callInfo.getCall();

                // remove the entries with outdated call id
                mCallIdCallInfoMap.remove(callId);
                mCallInfoMap.remove(call);

                return callInfo;
            }
        }
        return null;
    }

    CallInfo getOutgoingCallInfo() {
        for (String callId : mCallIdCallInfoMap.keySet()) {
            CallInfo callInfo = mCallIdCallInfoMap.get(callId);
            // For outgoing calls placed by Alexa, the map always has the call id before getting the Call object
            if (callInfo != null && callInfo.getCall() == null) {
                return callInfo;
            }
        }
        return null;
    }

    void addCallback(Call call, Call.Callback callback) {
        mCallStateCallbackMap.put(call, callback);
    }

    Call.Callback removeCallback(Call call) {
        return mCallStateCallbackMap.remove(call);
    }

    void setCall(String callId, Call call) {
        Log.i(TAG, "Call object updated for outgoing call...");
        CallInfo newCallInfo = mCallIdCallInfoMap.get(callId);
        newCallInfo.setCall(call);
        mCallIdCallInfoMap.put(callId, newCallInfo);
        mCallInfoMap.put(call, newCallInfo);
    }

    void clear() {
        mCallIdCallInfoMap.clear();
        mCallIdCallInfoMap = null;

        mCallInfoMap.clear();
        mCallInfoMap = null;

        mCallStateCallbackMap.clear();
        mCallStateCallbackMap = null;
    }

    // CallInfo Struct
    static class CallInfo {
        private Call mCall;
        private String mCallerId;
        private String mCallId;

        CallInfo(Call call, String callerId, String callId) {
            mCall = call;
            mCallerId = callerId;
            mCallId = callId;
        }

        Call getCall() {
            return mCall;
        }

        String getCallerId() {
            return mCallerId;
        }

        String getCallId() {
            return mCallId;
        }

        void setCall(Call call) {
            mCall = call;
        }
    }
}
