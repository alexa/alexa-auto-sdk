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
package com.amazon.maccandroid;

import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.errors.CapabilityAgentError;
import com.amazon.maccandroid.model.errors.MediaAppPlayerError;
import com.amazon.maccandroid.model.players.DiscoveredPlayer;

import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MediaAppsStateReporter {
    private static final String TAG = MediaAppsStateReporter.class.getSimpleName();

    private static MediaAppsStateReporter sInstance;
    private MACCAndroidClient mMACCAndroidClient;

    private MediaAppsStateReporter() {}

    public static MediaAppsStateReporter getInstance() {
        if (sInstance == null) {
            sInstance = new MediaAppsStateReporter();
        }
        return sInstance;
    }

    void initalize(MACCAndroidClient maccAndroidClient) {
        mMACCAndroidClient = maccAndroidClient;
    }

    void reportError(String playerId, CapabilityAgentError error) {
        if (!isInitalized()) {
            Log.e(TAG, "not initialized: " + playerId);
            return;
        }
        MediaApp app = MediaAppsRepository.getInstance().getAuthorizedMediaApp(playerId);
        String skillToken = null;
        UUID playbackSessionId = null;
        if (app != null) {
            playbackSessionId = app.getPlaybackSessionId();
        } else
            Log.e(TAG, "reportError app is null");
        mMACCAndroidClient.reportError(error.getName(), error.getErrorCode(), false, playerId, playbackSessionId);
    }

    void reportError(String playerId, MediaAppPlayerError error) {
        if (!isInitalized() || !isAppAuthorized(playerId))
            return;
        MediaApp app = MediaAppsRepository.getInstance().getAuthorizedMediaApp(playerId);
        mMACCAndroidClient.reportError(
                error.getName(), error.getErrorCode(), true, playerId, app.getPlaybackSessionId());
    }

    void reportPlayerEvent(String playerId, Set<PlayerEvents> playerEvents) {
        if (!isInitalized() || !isAppAuthorized(playerId))
            return;

        Log.i(TAG, "reportPlayerEvent | " + playerEvents);

        MediaApp app = MediaAppsRepository.getInstance().getAuthorizedMediaApp(playerId);
        mMACCAndroidClient.reportPlayerEvents(playerId, playerEvents, app.getSkillToken(), app.getPlaybackSessionId());
    }

    void reportDiscoveredPlayers(List<DiscoveredPlayer> discoveredPlayers) {
        if (!isInitalized())
            return;
        if (discoveredPlayers.size() == 0) {
            Log.i(TAG, "reportDiscoveredPlayers | No players to report");
            return;
        }
        mMACCAndroidClient.reportDiscoveredPlayers(discoveredPlayers);
    }

    private boolean isInitalized() {
        if (mMACCAndroidClient == null) {
            Log.e(TAG, "MediaAppsStateReporter not initalized");
            return false;
        }
        return true;
    }

    static void setTestStateReporter(MediaAppsStateReporter testInstance) {
        if (testInstance == null) {
            Log.e(TAG, "setTestStateReporter testInstance is null");
            return;
        }
        sInstance = testInstance;
    }

    private boolean isAppAuthorized(String playerId) {
        return MediaAppsRepository.getInstance().isAuthorizedApp(playerId);
    }

    public void requestToken(String localPlayerId) {
        mMACCAndroidClient.requestToken(localPlayerId);
    }

    public void reportRemovedApp(String localPlayerId) {
        mMACCAndroidClient.reportRemovedApp(localPlayerId);
    }
}
