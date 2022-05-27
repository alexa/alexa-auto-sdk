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

public class PlayDirective extends Directive {
    private final String mToken;
    private final long mIndex;
    private final long mOffest;
    private final boolean mPreload;
    private final String mNavigation;

    public PlayDirective(String playerId, String token, long index, long offset, boolean preload, String navigation) {
        super(playerId);
        mToken = token;
        mIndex = index;
        mOffest = offset;
        mPreload = preload;
        mNavigation = navigation;
    }

    @Override
    public String toString() {
        return "PlayDirective | mtoken: " + mToken + " | mIndex: " + mIndex + " | mOffset: " + mOffest
                + " | mPreload: " + mPreload + " | mNavigation: " + mNavigation;
    }

    public String getToken() {
        return mToken;
    }

    public long getIndex() {
        return mIndex;
    }

    public long getOffest() {
        return mOffest;
    }

    public boolean isPreload() {
        return mPreload;
    }

    public String getNavigation() {
        return mNavigation;
    }
}
