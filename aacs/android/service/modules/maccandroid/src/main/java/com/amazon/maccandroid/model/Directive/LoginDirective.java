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
package com.amazon.maccandroid.model.Directive;

public class LoginDirective extends Directive {
    private final String mAccessToken;
    private final String mUserName;
    private final boolean mForce;
    private final long mRefreshInterval;

    public LoginDirective(String playerId, String accessToken, String userName, boolean force, long refreshInterval) {
        super(playerId);
        mAccessToken = accessToken;
        mUserName = userName;
        mForce = force;
        mRefreshInterval = refreshInterval;
    }

    public String getAccessToken() {
        return mAccessToken;
    }

    public String getUserName() {
        return mUserName;
    }

    public boolean isForce() {
        return mForce;
    }

    public long getRefreshInterval() {
        return mRefreshInterval;
    }
}
