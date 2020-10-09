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

package com.amazon.aacsipc;

public class IPCConstants {
    // Actions
    public static final String ACTION_CONFIG = "com.amazon.aacs.service.config";
    public static final String ACTION_CANCEL_FETCH = "com.amazon.aacs.service.cancelFetch";
    public static final String ACTION_FETCH = "com.amazon.aacs.service.fetch";
    public static final String ACTION_PUSH = "com.amazon.aacs.service.push";

    // Categories
    public static final String CATEGORY_SERVICE = "com.amazon.aacs.service";

    // AACS Package Name
    public static final String AASB_INTENT_PREFIX = "com.amazon.aacs.aasb.";

    // AACS IPC protocol keys
    public static final String AACS_IPC_MESSAGE_PAYLOAD = "payload";
    public static final String AACS_IPC_MESSAGE_TYPE = "type";
    public static final String AACS_IPC_EMBEDDED_MESSAGE = "message";
    public static final String AACS_IPC_RESOURCE_ID = "resourceId";
    public static final String AACS_IPC_MESSAGE_TRANSFER_ID = "messageTransferId";
    public static final String AACS_IPC_STREAM_ID = "streamId";
    public static final String AACS_IPC_MESSENGER = "messenger";
    public static final String AACS_IPC_WRITE_TO = "writeTo";
    public static final String AACS_IPC_READ_FROM = "readFrom";
    public static final String AACS_IPC_ACK_STATE = "state";
    public static final String AACS_IPC_ACK_SUCCESS = "success";

    // AACS IPC protocol enum values
    public enum AacsIpcMessageType {
        EMBEDDED("embedded"),
        STREAMED("streamed");

        private String typeAsString;

        AacsIpcMessageType(String typeAsString) {
            this.typeAsString = typeAsString;
        }

        public String getTypeAsString() {
            return typeAsString;
        }
    }

    // IPC Sender LRU Cache Default Capacity
    public static final int DEFAULT_CACHE_CAPACITY = 20;
}
