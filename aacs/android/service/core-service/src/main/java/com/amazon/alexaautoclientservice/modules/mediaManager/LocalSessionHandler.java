/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexaautoclientservice.modules.mediaManager;

import android.content.ComponentName;
import android.content.Context;
import android.media.session.MediaController;
import android.media.session.MediaSessionManager;
import android.util.Log;

import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.NotificationListener;
import com.amazon.alexaautoclientservice.modules.externalMediaPlayer.IDiscoveredPlayerProvider;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LocalSessionHandler {
    private static final String TAG = AACSConstants.AACS + "-" + LocalSessionHandler.class.getSimpleName();

    private MediaSessionManager mMediaSessionManager;
    private Context mContext;
    private final Map<String, LocalMediaSourceHandler> mSources;
    private IDiscoveredPlayerProvider mDiscoveredPlayerProvider;

    public LocalSessionHandler() {
        mSources = new HashMap<>();
    }

    public void onCreate(Context context) {
        if (!NotificationListener.isEnabled(context)) {
            Log.w(TAG,
                    "Notification Listener permission is required for this feature to work, please provide the permission and restart the app to use Local Media Source controlling through Alexa");
            return;
        }
        mMediaSessionManager = (MediaSessionManager) context.getSystemService(Context.MEDIA_SESSION_SERVICE);
        if (mMediaSessionManager == null) {
            Log.w(TAG, "MediaSessionManager is null, something went wrong");
            return;
        }
        this.mContext = context;
    }

    public void onInitialize() {
        ComponentName listenerComponent = new ComponentName(mContext, NotificationListener.class);
        mMediaSessionManager.addOnActiveSessionsChangedListener(mSessionsChangedListener, listenerComponent);
        process(mMediaSessionManager, listenerComponent, null);
    }

    public void onDestroy() {
        mMediaSessionManager.removeOnActiveSessionsChangedListener(mSessionsChangedListener);
        this.mContext = null;
        Collection<LocalMediaSourceHandler> collection = mSources.values();
        for (LocalMediaSourceHandler localMediaSource : collection) {
            localMediaSource.cleanup();
        }
        mSources.clear();
        mDiscoveredPlayerProvider = null;
    }

    private final MediaSessionManager.OnActiveSessionsChangedListener mSessionsChangedListener =
            new MediaSessionManager.OnActiveSessionsChangedListener() {
                @Override
                public void onActiveSessionsChanged(
                        @Nullable @org.jetbrains.annotations.Nullable List<MediaController> controllers) {
                    process(null, null, controllers);
                }
            };

    public void add(LocalMediaSourceHandler localMediaSource) {
        if (localMediaSource != null) {
            if (localMediaSource.getMediaSource().getSourceType().equals(MediaSource.DEFAULT)) {
                localMediaSource.getMediaSource().putPackageName("");
                localMediaSource.getMediaSource().putClassName("");
                mSources.put("", localMediaSource);
            } else {
                mSources.put(localMediaSource.getMediaSource().getPackageName(), localMediaSource);
            }
        }
    }

    public void process(MediaSessionManager mediaSessionManager, ComponentName listenerComponent,
            List<MediaController> controllerList) {
        Log.v(TAG, "Processing the session list");
        if (mediaSessionManager != null && controllerList == null) {
            controllerList = mediaSessionManager.getActiveSessions(listenerComponent);
        }
        if (controllerList == null || controllerList.isEmpty()) {
            Log.v(TAG, "No active sessions right now");
        } else {
            for (int pointer = 0; pointer < controllerList.size(); pointer++) {
                String packageName = controllerList.get(pointer).getPackageName();
                Log.v(TAG, String.format("OnActiveSessionsChanged %d %s", pointer, packageName));
                LocalMediaSourceHandler mediaSourceHandler = mSources.get(packageName);
                if (mediaSourceHandler == null) {
                    // This is DEFAULT media
                    if (!isExceptionalApp(packageName)) {
                        mediaSourceHandler = mSources.get("");
                        if (mediaSourceHandler == null)
                            continue;
                        mediaSourceHandler.mediaSessionStarted(controllerList.get(pointer));
                    } else {
                        Log.w(TAG, "Default player can not be " + packageName);
                    }
                } else {
                    mediaSourceHandler.mediaSessionStarted(controllerList.get(0));
                }
                break;
            }
        }
    }

    /**
     * Alexa media and MACC discovered media are exceptions and should not be treated a DEFAULT
     * @param packageName of identified media
     * @return true if package name belongs to Alexa media or MACC discovered players
     */
    private boolean isExceptionalApp(String packageName) {
        return (packageName.equals(mContext.getPackageName()) || packageName.contains("com.amazon.alexa.auto.app")
                || (mDiscoveredPlayerProvider != null
                        && mDiscoveredPlayerProvider.containsDiscoveredPlayer(packageName)));
    }

    public void setDiscoveredPlayerProvider(IDiscoveredPlayerProvider discoveredPlayerObserver) {
        mDiscoveredPlayerProvider = discoveredPlayerObserver;
    }
}
