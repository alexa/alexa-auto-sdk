/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.maccandroid.model.errors;

public enum CapabilityAgentError {
    PLAYER_UNKNOWN("PLAYER_UNKNOWN", 100),
    PLAYER_NOT_FOUND("PLAYER_NOT_FOUND", 200),
    PLAYER_CONNECTION_REJECTED("PLAYER_CONNECTION_REJECTED", 300),
    PLAYER_CONNECTION_TIMEOUT("PLAYER_CONNECTION_TIMEOUT", 400),
    OPERATION_UNSUPPORTED("OPERATION_UNSUPPORTED", 1000),
    ;

    private final int mErrorCode;
    private final String mName;

    CapabilityAgentError(String name, int errorCode) {
        mName = name;
        mErrorCode = errorCode;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    public String getName() {
        return mName;
    }
}
