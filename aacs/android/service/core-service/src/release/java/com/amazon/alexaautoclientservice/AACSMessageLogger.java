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

package com.amazon.alexaautoclientservice;

import androidx.annotation.Nullable;

public class AACSMessageLogger {
    public static final String TO_ENGINE = "TO_ENGINE";
    public static final String FROM_ENGINE = "FROM_ENGINE";

    public void setLogFileLocation(String filePath) {
        return;
    }
    public void buffer(String direction, String topic, String action, String payload, @Nullable String messageId,
            @Nullable String replyToId) {
        return;
    }
    // This class is for debug usage, full definition is available in debug build.
}
