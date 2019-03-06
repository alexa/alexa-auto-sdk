/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
    private final Map< String, MediaApp > discoveredMediaApps = new HashMap<>();
    private final Map< String, MediaApp > authorizedMediaApps = new HashMap<>();

    private MediaAppsRepository() {
    }

    public static MediaAppsRepository getInstance() {
        if ( sInstance == null ) {
            sInstance = new MediaAppsRepository();
        }
        return sInstance;
    }

    void addDiscoveredMediaApp(MediaApp mediaApp) {
        Log.i(TAG, "addDiscoveredMediaApp | " + mediaApp);
        discoveredMediaApps.put( mediaApp.getLocalPlayerId(), mediaApp );
    }

    void addAuthorizedMediaApp(MediaApp mediaApp) {
        Log.i(TAG, "addAuthorizedMediaApp | " + mediaApp);
        authorizedMediaApps.put( mediaApp.getLocalPlayerId(), mediaApp );
    }

    MediaApp getDiscoveredMediaApp(String playerId) {
        return discoveredMediaApps.get(playerId);
    }

    MediaApp getAuthorizedMediaApp(String playerId) {
        return authorizedMediaApps.get(playerId);
    }

    Map< String, MediaApp > getDiscoveredMediaApps() {
        return discoveredMediaApps;
    }

    Map <String, MediaApp> getAuthorizedMediaApps() {
        return authorizedMediaApps;
    }

    boolean isAuthorizedApp(String app) {
        return authorizedMediaApps.containsKey(app);
    }

    boolean isDiscoveredApp(String app) {
        return discoveredMediaApps.containsKey(app);
    }

    void removeMediaApp(String packageName) {
        Log.i(TAG, "removeMediaApp | " + packageName, new Exception());
        MediaApp app = discoveredMediaApps.get(packageName);
        if (app != null) {
            app.onDestroy();
            MediaAppsStateReporter.getInstance().reportRemovedApp(packageName);
        }
        discoveredMediaApps.remove(packageName);
        authorizedMediaApps.remove(packageName);
    }

    void clearDiscoveredApps() {
        discoveredMediaApps.clear();
    }
}
