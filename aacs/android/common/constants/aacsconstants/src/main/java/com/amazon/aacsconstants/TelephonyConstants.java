/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.aacsconstants;

public class TelephonyConstants {
    public static final String ACTION_QUERY_PHONE_ACCOUNT_HANDLER = "com.amazon.aacstelephony.phoneAccount.query";
    public static final String ACTION_REPLY_PHONE_ACCOUNT_HANDLER = "com.amazon.aacstelephony.phoneAccount.reply";
    public static final String ACTION_UPDATE_DEVICE_COFIGURATION = "com.amazon.aacstelephony.updateDeviceConfiguration";
    public static final String ACTION_BLUETOOTH_STATE_CONNECTED = "com.amazon.aacstelephony.bluetooth.connected";
    public static final String ACTION_BLUETOOTH_STATE_DISCONNECTED = "com.amazon.aacstelephony.bluetooth.disconnected";
    public static final String ACTION_BLUETOOTH_STATE_CONNECTION_CHECK_COMPLETED =
            "com.amazon.aacstelephony.bluetooth.connectionCheckCompleted";
    public static final String ACTION_PAIRED_DEVICE = "com.amazon.aacstelephony.bluetooth.pairedDevice";
    public static final String CATEGORY_AACS_TELEPHONY = "com.amazon.aacstelephony";

    public static final String STATE = "state";
    public static final String CALLID = "callId";
    public static final String CALLERID = "callerId";
    public static final String ISREDIAL = "isRedial";
    public static final String PHONE_ACCOUNT_BUNDLE = "phoneAccountBundle";
    public static final String PHONE_ACCOUNT_HANDLE = "phoneAccountHandle";

    public static class ErrorCode {
        public static final String NO_CARRIER = "NO_CARRIER";
        public static final String BUSY = "BUSY";
        public static final String NO_ANSWER = "NO_ANSWER";
        public static final String NO_NUMBER_FOR_REDIAL = "NO_NUMBER_FOR_REDIAL";
        public static final String OTHER = "OTHER";
    }

    public static final String DEVICE_PROPERTY = "deviceProperty";
    public static final String ENABLE = "enable";
}
