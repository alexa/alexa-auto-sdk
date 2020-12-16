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

package com.amazon.aacstts.models;

import com.amazon.aacstts.JSONUtil;

public class PrepareSpeechMessagePayload {
    private String mSpeechId;
    private String mText;
    private String mProvider;
    private PrepareSpeechMessageOptions options;

    public PrepareSpeechMessagePayload(
            String mSpeechId, String mText, String mProvider, PrepareSpeechMessageOptions options) {
        this.mSpeechId = mSpeechId;
        this.mText = mText;
        this.mProvider = mProvider;
        this.options = options;
    }

    public String getSpeechId() {
        return mSpeechId;
    }

    @Override
    public String toString() {
        return "PrepareSpeechMessagePayload{"
                + "speechId='" + mSpeechId + '\'' + ", text='" + mText + '\'' + ", provider='" + mProvider + '\''
                + ", options=" + options + '}';
    }

    public String toJsonString() {
        return "{"
                + "\"speechId\":\"" + mSpeechId + "\","
                + "\"text\":\"" + mText + "\","
                + "\"provider\":\"" + mProvider + "\","
                + "\"options\":\"" + JSONUtil.escapeString((options.toJsonString())) + "\""
                + "}";
    }
}
