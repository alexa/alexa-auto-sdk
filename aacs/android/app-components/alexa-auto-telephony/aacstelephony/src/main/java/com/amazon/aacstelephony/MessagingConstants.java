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

package com.amazon.aacstelephony;

public class MessagingConstants {
    public static final int MAP_CLIENT = 18;
    public static final String PERMISSION_OFF = "OFF";
    public static final String PERMISSION_ON = "ON";
    public static final String MESSAGING_ACTION_SEND_MESSAGE = "SendMessage";
    public static final String MESSAGING_ACTION_UPDATE_MESSAGE_STATUS = "UpdateMessagesStatus";
    public static final String MESSAGING_ACTION_UPLOAD_CONVERSATION = "UploadConversations";

    public static final String ACTION_MESSAGE_RECEIVED = "android.bluetooth.mapmce.profile.action.MESSAGE_RECEIVED";
    public static final String ACTION_MESSAGE_SENT = "com.amazon.aacstelephony.MESSAGE_SENT_SUCCESSFULLY";
    public static final String ACTION_MESSAGE_READ_STATUS_CHANGED =
            "android.bluetooth.mapmce.profile.action.MESSAGE_READ_STATUS_CHANGED";
    public static final String CONNECTION_STATE_CHANGED =
            "android.bluetooth.mapmce.profile.action.CONNECTION_STATE_CHANGED";

    public static final String SEND_MESSAGE_FAIL_ERROR_GENERIC = "GENERIC_FAILURE";
    public static final String SEND_MESSAGE_FAIL_ERROR_CONNECTION = "NO_CONNECTIVITY";
    public static final String SEND_MESSAGE_FAIL_ERROR_PERMISSION = "NO_PERMISSION";

    public static final String CONNECTION_STATE = "connectionState";
    public static final String SEND_PERMISSION = "sendPermission";
    public static final String READ_PERMISSION = "readPermission";
    public static final String PREFERENCES_FILE_NAME = "com.amazon.aacstelephony.messaging_consent";
    public static final String CONSENT_PROPERTY = "consent";
    public static final String STATE_PROPERTY = "state";
    public static final String DEVICE_ADDRESS_PROPERTY = "device";
}
