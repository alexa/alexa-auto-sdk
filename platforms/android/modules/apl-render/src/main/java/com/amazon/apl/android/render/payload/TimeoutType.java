/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.payload;

/**
 * Supported timeout types in an APL document.
 *
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#renderdocument
 */
public enum TimeoutType {
    SHORT(0),
    LONG(1),
    TRANSIENT(4);
    private int timeoutFlag;
    private static final String TIMEOUT_LONG = "LONG";
    private static final String TIMEOUT_TRANSIENT = "TRANSIENT";

    /**
     * Returns the flag associated with TimeoutType.
     * @return the timeout flag {@link TimeoutPolicy flags}
     */
    public int getTimeoutFlag() {
        return this.timeoutFlag;
    }

    public static TimeoutType getTimeoutType(String timeoutType) {
        switch (timeoutType) {
            case TIMEOUT_LONG:
                return TimeoutType.LONG;
            case TIMEOUT_TRANSIENT:
                return TimeoutType.TRANSIENT;
            default:
                return TimeoutType.SHORT;
        }
    }

    TimeoutType(int timeoutFlag) {
        this.timeoutFlag = timeoutFlag;
    }
}