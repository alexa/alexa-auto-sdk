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

import java.util.List;

public class ProviderVoiceItem {
    private String mVoiceId;
    private List<String> mSupportedLocales;

    public ProviderVoiceItem(String mVoiceId, List<String> mSupportedLocales) {
        this.mVoiceId = mVoiceId;
        this.mSupportedLocales = mSupportedLocales;
    }

    public String getVoiceId() {
        return mVoiceId;
    }

    public List<String> getSupportedLocales() {
        return mSupportedLocales;
    }

    @Override
    public String toString() {
        return "ProviderVoiceItem{"
                + "mVoiceId='" + mVoiceId + '\'' + ", mSupportedLocales=" + mSupportedLocales + '}';
    }
}
