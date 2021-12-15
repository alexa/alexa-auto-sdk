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

package com.amazon.maccandroid;

import java.util.HashMap;
import java.util.Map;

public class MediaAppsRepository {
    private static MediaAppsRepository sInstance;
    private static final String TAG = MediaAppsRepository.class.getSimpleName();
    private final Map<String, MediaApp> mDiscoveredMediaApps = new HashMap<>();
    private final Map<String, MediaApp> mAuthorizedMediaApps = new HashMap<>();

    private MediaAppsRepository() {}

    public static MediaAppsRepository getInstance() {
        if (sInstance == null) {
            sInstance = new MediaAppsRepository();
        }
        return sInstance;
    }

    void addDiscoveredMediaApp(MediaApp mediaApp) {
        String playerId = mediaApp.getLocalPlayerId();
        Log.i(TAG, "addDiscoveredMediaApp | " + mediaApp);
        mDiscoveredMediaApps.put(mediaApp.getLocalPlayerId(), mediaApp);
    }

    void addAuthorizedMediaApp(MediaApp mediaApp) {
        String playerId = mediaApp.getLocalPlayerId();
        if (mAuthorizedMediaApps.get(playerId) == null) {
            Log.i(TAG, "addAuthorizedMediaApp | " + mediaApp);
            mAuthorizedMediaApps.put(mediaApp.getLocalPlayerId(), mediaApp);
        } else
            Log.i(TAG, "addAuthorizedMediaApp | " + playerId + "app is already authorized, do not reauthorize");
    }

    MediaApp getDiscoveredMediaApp(String playerId) {
        return mDiscoveredMediaApps.get(playerId);
    }

    MediaApp getAuthorizedMediaApp(String playerId) {
        return mAuthorizedMediaApps.get(playerId);
    }

    Map<String, MediaApp> getDiscoveredMediaApps() {
        return mDiscoveredMediaApps;
    }

    Map<String, MediaApp> getAuthorizedMediaApps() {
        return mAuthorizedMediaApps;
    }

    boolean isAuthorizedApp(String app) {
        return mAuthorizedMediaApps.containsKey(app);
    }

    boolean isDiscoveredApp(String app) {
        return mDiscoveredMediaApps.containsKey(app);
    }

    void removeMediaApp(String packageName) {
        Log.i(TAG, "removeMediaApp | " + packageName);
        MediaApp app = mDiscoveredMediaApps.get(packageName);
        if (app != null) {
            app.onDestroy();
            MediaAppsStateReporter.getInstance().reportRemovedApp(packageName);
        }
        mDiscoveredMediaApps.remove(packageName);
        mAuthorizedMediaApps.remove(packageName);
    }

    void clearDiscoveredApps() {
        mDiscoveredMediaApps.clear();
    }
}
