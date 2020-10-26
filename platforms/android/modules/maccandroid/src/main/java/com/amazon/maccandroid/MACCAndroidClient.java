/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Looper;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.Directive.Directive;
import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.SupportedOperations;
import com.amazon.maccandroid.model.errors.CapabilityAgentError;
import com.amazon.maccandroid.model.players.AuthorizedPlayer;
import com.amazon.maccandroid.model.players.DiscoveredPlayer;
import com.amazon.maccandroid.model.state.ExternalMediaPlayerState;

import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * MACCAndroidClient implementation that can be instantiated to provide a client to connect to
 * MACC compatible apps, and provides callbacks to assist with the initialization process
 * and reporting of events and states
 */
public class MACCAndroidClient {
    private static final String TAG = MACCAndroidClient.class.getSimpleName();

    private final Context mContext;
    private final MediaAppsDirectivesHandler mDirectivesHandler;
    private final DiscoverAndReportMediaAppsHandler mDiscoverAndReportMediaAppshandler;
    private MACCAndroidClientCallback mMACCAndroidClientCallback;
    private final SupportedOperations mSupportedOperations;

    /**
     * App initiated connection to  MACCAndroidClient
     * e.g. The user opens up the media app using touch and presses play
     */
    private BroadcastReceiver mAppInitiatedBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Checking to make sure intent has the information we need
            Log.i(TAG, "onReceive intent : " + intent);
            if (intent != null && intent.getExtras() != null
                    && intent.getExtras().containsKey(Intent.EXTRA_COMPONENT_NAME)) {
                final ComponentName componentName = (ComponentName) intent.getExtras().get(Intent.EXTRA_COMPONENT_NAME);
                Log.i(TAG, "Component : " + componentName + " is trying to connect to MACC Android Client");

                if (MediaAppsRepository.getInstance().isAuthorizedApp(componentName.getPackageName())) {
                    MediaAppsRepository.getInstance()
                            .getAuthorizedMediaApps()
                            .get(componentName.getPackageName())
                            .connect(new MediaAppsConnectionListener() {
                                @Override
                                public void onConnectionSuccessful() {
                                    Log.i(TAG, "Component : " + componentName + " successfully connected");
                                }

                                @Override
                                public void onConnectionFailure(CapabilityAgentError error) {
                                    Log.i(TAG, "onConnectionFailure");
                                    MediaAppsStateReporter.getInstance().reportError(
                                            componentName.getPackageName(), error);
                                    MediaAppsRepository.getInstance().removeMediaApp(componentName.getPackageName());
                                    runDiscovery();
                                }
                            });
                }
            }
        }
    };

    /**
     * Any time a package gets changed or added we want to rescan for MACC compatible apps
     */
    BroadcastReceiver mPackageChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "mPackageChangedReceiver | running discovery");
            runDiscovery();
        }
    };

    public MACCAndroidClient(Context context) {
        this(context, new SupportedOperations());
    }

    public MACCAndroidClient(Context context, SupportedOperations supportedOperations) {
        super();
        mContext = context;
        mSupportedOperations = supportedOperations;
        MediaAppsStateReporter.getInstance().initalize(this);
        mDiscoverAndReportMediaAppshandler = new DiscoverAndReportMediaAppsHandler(Looper.getMainLooper(), mContext);
        mDirectivesHandler = new MediaAppsDirectivesHandler(context, mSupportedOperations);
        mContext.registerReceiver(mAppInitiatedBroadcastReceiver, new IntentFilter(APIConstants.Actions.CONNECT_ACTION),
                APIConstants.Permissions.EMP_CONNECT_PERMISSION, null);
    }

    public void cleanup() {
        Log.i(TAG, "unregistering mPackageChangedReceiver & mAppInitiatedBroadcastReceiver");
        if (mAppInitiatedBroadcastReceiver != null)
            mContext.unregisterReceiver(mAppInitiatedBroadcastReceiver);
        if (mPackageChangedReceiver != null)
            mContext.unregisterReceiver(mPackageChangedReceiver);
    }

    /**
     * runs a single instance of the discovery scan, usually good to run at startup
     */
    public void initAndRunDiscovery() {
        runDiscovery();
        registerPackageChangedReceiver();
    }

    public void runDiscovery() {
        mDiscoverAndReportMediaAppshandler.sendEmptyMessage(DiscoverAndReportMediaAppsHandler.START_DISCOVERY);
        mDiscoverAndReportMediaAppshandler.sendEmptyMessage(
                DiscoverAndReportMediaAppsHandler.REPORT_DISCOVERED_MEDIA_APPS);
    }

    /**
     * Call that is made to give a list of players that were authorized from the cloud
     * @param players list of authorized players
     */
    public void onAuthorizedPlayers(List<AuthorizedPlayer> players) {
        Log.i(TAG, "onAuthorizedPlayers");
        mDirectivesHandler.handleAuthorizedPlayers(players);
    }

    /**
     * handles a directive coming from AVS
     * @param directive directive coming from AVS
     */
    public void handleDirective(Directive directive) {
        Log.i(TAG, "handleDirective | directive: " + directive);
        mDirectivesHandler.handleDirective(directive);
    }

    /**
     * Gets the current state of a specific player. This state is ExternalMediaPlayerState
     * @param playerId
     */
    public ExternalMediaPlayerState getState(String playerId) {
        if (playerId == null) {
            Log.e(TAG, "getState | player id should never be null");
            return null;
        }

        if (!MediaAppsRepository.getInstance().isAuthorizedApp(playerId)) {
            Log.e(TAG, "getState | player id requested is either not present or not authorized");
            return null;
        }
        Log.i(TAG, "getState | playerId:" + playerId);
        ExternalMediaPlayerState state = new ExternalMediaPlayerState(
                MediaAppsRepository.getInstance().getAuthorizedMediaApp(playerId), mSupportedOperations);
        return state;
    }

    /**
     * Package protected method that is responsible for reporting the discovered MediaApps in the
     * system to the client who registered the callback
     * @param mediaApps list of media apps that were discovered
     */
    void reportDiscoveredPlayers(List<DiscoveredPlayer> mediaApps) {
        Log.i(TAG, "reportDiscoveredPlayers");
        mMACCAndroidClientCallback.onPlayerDiscovered(mediaApps);
    }

    /**
     * Reports a list of player events associated with a single player id
     * @param playerId player id
     * @param playerEvents set of player events
     * @param skillToken skill token of player
     * @param playbackSessionId session id of player
     */
    void reportPlayerEvents(
            String playerId, Set<PlayerEvents> playerEvents, String skillToken, UUID playbackSessionId) {
        mMACCAndroidClientCallback.onPlayerEvent(playerId, playerEvents, skillToken, playbackSessionId);
    }

    /**
     * Reports an error via callback up
     * @param errorName name of the error
     * @param errorCode code number
     * @param fatal whether it was fatal to the media session or not
     * @param playerId player id the error was derived from
     * @param playbackSessionId playback session id of media app
     */
    void reportError(String errorName, int errorCode, boolean fatal, String playerId, UUID playbackSessionId) {
        mMACCAndroidClientCallback.onError(errorName, errorCode, fatal, playerId, playbackSessionId);
    }

    public void registerCallback(MACCAndroidClientCallback callback) {
        if (callback == null)
            throw new IllegalArgumentException("callback cannot be null");
        mMACCAndroidClientCallback = callback;
    }

    private void registerPackageChangedReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_CHANGED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_REPLACED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        intentFilter.addDataScheme("package");
        mContext.registerReceiver(mPackageChangedReceiver, intentFilter);
    }

    public void requestToken(String localPlayerId) {
        mMACCAndroidClientCallback.requestTokenForPlayerId(localPlayerId);
    }

    public void reportRemovedApp(String localPlayerId) {
        mMACCAndroidClientCallback.onRemovedPlayer(localPlayerId);
    }
}