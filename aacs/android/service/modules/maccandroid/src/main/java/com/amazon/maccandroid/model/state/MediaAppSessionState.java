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
package com.amazon.maccandroid.model.state;

import com.amazon.maccandroid.MediaApp;

public class MediaAppSessionState {
    private final String mPlayerid;
    private final boolean mLaunched;
    private final boolean mActive;
    private final String mSpiVersion;
    private final String mPlayerCookie;

    public MediaAppSessionState(MediaApp app) {
        mPlayerid = app.getLocalPlayerId();
        mLaunched = app.getMediaController() != null;
        mActive = app.isSessionReady();
        mSpiVersion = app.getSpiVersion();
        mPlayerCookie = app.getPlayerCookie();
    }

    public String getPlayerid() {
        return mPlayerid;
    }

    public boolean isLaunched() {
        return mLaunched;
    }

    public boolean isActive() {
        return mActive;
    }

    public String getSpiVersion() {
        return mSpiVersion;
    }

    public String getPlayerCookie() {
        return mPlayerCookie;
    }
}
