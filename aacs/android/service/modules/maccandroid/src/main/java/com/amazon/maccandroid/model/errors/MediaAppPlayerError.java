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

public enum MediaAppPlayerError {
    UNPLAYABLE_BY_ACCOUNT("UNPLAYABLE_BY_ACCOUNT", 2100),
    INTERNAL_ERROR("INTERNAL_ERROR", 3000),
    UNPLAYABLE_BY_AUTHORIZATION("UNPLAYABLE_BY_AUTHORIZATION", 2000),
    UNPLAYABLE_BY_STREAM_CONCURRENCY("UNPLAYABLE_BY_STREAM_CONCURRENCY", 2400),
    OPERATION_REJECTED_UNINTERRUPTIBLE("OPERATION_REJECTED_UNINTERRUPTIBLE", 1300),
    OPERATION_REJECTED_END_OF_QUEUE("OPERATION_REJECTED_END_OF_QUEUE", 1100),
    UNPLAYABLE_BY_REGION("UNPLAYABLE_BY_REGION", 2500),
    OPERATION_UNSUPPORTED("OPERATION_UNSUPPORTED", 1000),
    UNPLAYABLE_BY_PARENTAL_CONTROL("UNPLAYABLE_BY_PARENTAL_CONTROL", 2600),
    UNPLAYABLE_BY_SUBSCRIPTION("UNPLAYABLE_BY_SUBSCRIPTION", 2300),
    OPERATION_REJECTED_SKIP_LIMIT("OPERATION_REJECTED_SKIP_LIMIT", 1200),
    UNKNOWN_ERROR("UNKNOWN_ERROR", 3100),
    ;

    private final int mErrorCode;
    private final String mName;

    MediaAppPlayerError(String name, int errorCode) {
        mErrorCode = errorCode;
        mName = name;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    public String getName() {
        return mName;
    }
}
