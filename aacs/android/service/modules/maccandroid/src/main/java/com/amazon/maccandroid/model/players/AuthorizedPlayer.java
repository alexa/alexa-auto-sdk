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
package com.amazon.maccandroid.model.players;

public class AuthorizedPlayer {
    private final String mLocalPlayerId;
    private final boolean mAuthorized;

    public AuthorizedPlayer(String localPlayerId, boolean authorized) {
        mLocalPlayerId = localPlayerId;
        mAuthorized = authorized;
    }

    public String getLocalPlayerId() {
        return mLocalPlayerId;
    }

    public boolean isAuthorized() {
        return mAuthorized;
    }

    @Override
    public String toString() {
        return "mLocalPlayerId: " + mLocalPlayerId + " | mAuthorized: " + mAuthorized;
    }
}
