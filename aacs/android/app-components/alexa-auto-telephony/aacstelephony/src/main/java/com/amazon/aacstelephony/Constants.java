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

public class Constants {
    public static final String TELEPHONY_SERVICE_CLASS = "com.amazon.aacstelephony.AACSTelephonyService";

    public static final String ACTION_CANCEL_IDLE_TIMER = "com.amazon.aacs.telephony.serviceLifecycle.cancel";
    public static final String ACTION_RESET_IDLE_TIMER = "com.amazon.aacs.telephony.serviceLifecycle.reset";
    public static final String ACTION_BLUETOOTH_PBAP_CLIENT_STATE_CHANGED =
            "android.bluetooth.pbapclient.profile.action.CONNECTION_STATE_CHANGED";
    public static final String ACTION_BLUETOOTH_HFP_CLIENT_STATE_CHANGED =
            "android.bluetooth.headsetclient.profile.action.CONNECTION_STATE_CHANGED";
    public static final String AACS_TELEPHONY_PERMISSION = "com.amazon.aacstelephony";
    public static final String AACS_BOND_STATE_CHANGED = "com.amazon.aacstelephony.bluetooth.bondStateChanged";

    public static final String CALLEE = "callee";
    public static final String CODE = "code";
    public static final String MESSAGE = "message";

    public static final String HEADSET_CLIENT = "HEADSET_CLIENT";
    public static final String PBAP_CLIENT = "PBAP_CLIENT";

    // Bluetooth Profile IDs that are marked as @SystemApi in Android 30
    // ID source:
    // https://cs.android.com/android/platform/superproject/+/android-11.0.0_r2:frameworks/base/core/java/android/bluetooth/BluetoothProfile.java
    public static final int HEADSET_CLIENT_PROFILE_ID = 16;
    public static final int PBAP_CLIENT_PROFILE_ID = 17;

    public static class CallState {
        public static final String IDLE = "IDLE";
        public static final String DIALING = "DIALING";
        public static final String OUTBOUND_RINGING = "OUTBOUND_RINGING";
        public static final String ACTIVE = "ACTIVE";
        public static final String CALL_RECEIVED = "CALL_RECEIVED";
        public static final String INBOUND_RINGING = "INBOUND_RINGING";
    }

    public static class SendDTMFError {
        public static final String CALL_NOT_IN_PROGRESS = "CALL_NOT_IN_PROGRESS";
        public static final String DTMF_FAILED = "DTMF_FAILED";
    }

    public static class ConnectionState {
        public static final String CONNECTED = "CONNECTED";
        public static final String DISCONNECTED = "DISCONNECTED";
    }
}
