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

import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Base64;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.PlayerPlaybackInfo;
import com.amazon.maccandroid.model.errors.CapabilityAgentError;
import com.amazon.maccandroid.model.state.MediaAppPlaybackState;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class MediaApp extends MediaBrowserCompat.ConnectionCallback {
    private static final long MEDIA_SESSION_CONNECTION_TIMEOUT = 3000;
    private static final String VALIDATION_METHOD_SIGNING = "SIGNING_CERTIFICATE";
    private static final String TAG = MediaApp.class.getSimpleName();

    private final Context mContext;

    private final String mLocalPlayerId;
    private final String mClassName;
    private String mSpiVersion;
    private String mPlayerCookie;
    private List<String> validationData;
    private String validationMethod = null;

    private PlayerPlaybackInfo mPlayerPlaybackInfo;
    private UUID mPlaybackSessionId;
    private String mSkillToken;
    private Handler mHandler = new Handler();

    private MediaBrowserCompat mMediaBrowser;
    private MediaControllerCallback mMediaControllerCallback;
    private MediaControllerCompat mMediaController;
    private MediaAppsConnectionListener mMediaAppsConnectionListener;
    private boolean mSessionReady = false;

    public static MediaApp create(
            Context context, String packageName, String className, String spiVersion, String playerCookie) {
        return new MediaApp(context, packageName, className, spiVersion, playerCookie);
    }

    // package private for testing
    /*package*/ MediaApp(
            Context context, String packageName, String className, String spiVersion, String playerCookie) {
        mContext = context;
        mLocalPlayerId = packageName;
        mClassName = className;
        mSpiVersion = spiVersion;
        mPlayerCookie = playerCookie;
        initValidationData(context);
    }

    // Package private for testing
    /*package*/ void initValidationData(Context context) {
        try {
            validationData = new ArrayList<>();
            PackageInfo packageInfo =
                    context.getPackageManager().getPackageInfo(mLocalPlayerId, PackageManager.GET_SIGNATURES);
            for (Signature signature : packageInfo.signatures) {
                validationData.add(Base64.encodeToString(signature.toByteArray(), Base64.NO_WRAP));
            }
            validationMethod = VALIDATION_METHOD_SIGNING;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
    }

    /**
     * Connect to the MediaBrowserService of the MediaApp. We don't want to do this immediately as
     * it means maintaining unnecessary connections. We should only connect when there is an alexa
     * initiated request, or the media app requests a connection from us.
     * @param listener
     */
    public void connect(final MediaAppsConnectionListener listener) {
        Log.i(TAG, "connect | " + mLocalPlayerId);
        if (mMediaBrowser != null) {
            Log.i(TAG, "already connected no need to connect again");
            return;
        }
        ComponentName componentName = new ComponentName(mLocalPlayerId, mClassName);
        this.mMediaAppsConnectionListener = listener;

        mMediaBrowser = new MediaBrowserCompat(mContext, componentName, this, null);
        mMediaBrowser.connect();
        handleConnectionTimeout();
    }

    @Override
    public void onConnected() {
        super.onConnected();
        if (mMediaBrowser == null) {
            Log.e(TAG, "onConnected mMediaBrowser is null");
            return;
        }

        mHandler.removeCallbacksAndMessages(null);

        MediaSessionCompat.Token token = mMediaBrowser.getSessionToken();
        try {
            mMediaController = new MediaControllerCompat(mContext, token);
            Bundle extras = mMediaController.getExtras();
            if (extras != null) {
                if (extras.containsKey(APIConstants.ExtrasKeys.SPI_VERSION_KEY)) {
                    mSpiVersion = extras.getString(APIConstants.ExtrasKeys.SPI_VERSION_KEY);
                }

                if (extras.containsKey(APIConstants.ExtrasKeys.PLAYER_COOKIE_KEY)) {
                    mPlayerCookie = extras.getString(APIConstants.ExtrasKeys.PLAYER_COOKIE_KEY);
                }
            } else
                Log.e(TAG, "MediaControllerCompat extras is null");

            mSessionReady = true; // mMediaController.isSessionReady();

            if (mSessionReady && !doesControllerSupportsRequiredActions(mMediaController)) {
                Log.i(TAG, "Controller for App " + mLocalPlayerId + " does not support required actions");
                MediaAppsRepository.getInstance().removeMediaApp(mLocalPlayerId);
            }

            if (mMediaAppsConnectionListener != null) {
                mMediaAppsConnectionListener.onConnectionSuccessful();
            }

            registerCallback();
            refreshPlaybackState();

        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public void refreshPlaybackState() {
        if (mMediaController == null) {
            Log.e(TAG, "refreshPlaybackState mMediaController is null");
            return;
        }
        mMediaControllerCallback.onPlaybackStateChanged(mMediaController.getPlaybackState());
        mMediaControllerCallback.onMetadataChanged(mMediaController.getMetadata());
        mMediaControllerCallback.onRepeatModeChanged(mMediaController.getRepeatMode());
        mMediaControllerCallback.onShuffleModeChanged(mMediaController.getShuffleMode());
    }

    @Override
    public void onConnectionFailed() {
        super.onConnectionFailed();
        mHandler.removeCallbacksAndMessages(null);
        Log.i(TAG, "onConnectionFailed for App " + mLocalPlayerId);
        if (mMediaAppsConnectionListener != null) {
            mMediaAppsConnectionListener.onConnectionFailure(CapabilityAgentError.PLAYER_CONNECTION_REJECTED);
        }
        mMediaBrowser = null;
    }

    @Override
    public void onConnectionSuspended() {
        super.onConnectionSuspended();
        Log.i(TAG, "onConnectionSuspended for App " + mLocalPlayerId);
    }

    private void handleConnectionTimeout() {
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mMediaAppsConnectionListener != null) {
                    mMediaAppsConnectionListener.onConnectionFailure(CapabilityAgentError.PLAYER_CONNECTION_TIMEOUT);
                }
                mMediaController = null;
            }
        }, MEDIA_SESSION_CONNECTION_TIMEOUT);
    }

    /**
     * Check if the controller supports both required actions ACTION_PLAY_FROM_URI and ACTION_PREPARE_FROM_URI
     *
     * @param controller
     * @return
     */
    private boolean doesControllerSupportsRequiredActions(MediaControllerCompat controller) {
        PlaybackStateCompat playbackState = controller.getPlaybackState();
        if (playbackState == null) {
            Log.e(TAG, "doesControllerSupportsRequiredActions playbackState is null");
            return false;
        }
        long actions = playbackState.getActions();
        return ((actions & PlaybackStateCompat.ACTION_PLAY_FROM_URI) > 0)
                && ((actions & PlaybackStateCompat.ACTION_PREPARE_FROM_URI) > 0);
    }

    public void registerCallback() {
        mMediaControllerCallback = new MediaControllerCallback(getLocalPlayerId(), this);
        mMediaController.registerCallback(mMediaControllerCallback);
    }

    public void onDestroy() {
        try {
            if (mMediaController != null && mMediaControllerCallback != null) {
                mMediaController.unregisterCallback(mMediaControllerCallback);
            }
            if (mMediaBrowser != null) {
                mMediaBrowser.disconnect();
            }
        } catch (Exception e) {
            Log.e(TAG, "Error onDestroy: " + e.getMessage());
        }
    }

    public String getLocalPlayerId() {
        return mLocalPlayerId;
    }

    public List<String> getValidationData() {
        return validationData;
    }

    public String getValidationMethod() {
        return validationMethod;
    }

    public static String getValidationMethodSigning() {
        return VALIDATION_METHOD_SIGNING;
    }

    public String getClassName() {
        return mClassName;
    }

    public UUID getPlaybackSessionId() {
        return mPlaybackSessionId;
    }

    public MediaControllerCompat getMediaController() {
        return mMediaController;
    }

    public void setPlaybackSessionId(UUID playbackSessionId) {
        this.mPlaybackSessionId = playbackSessionId;
    }

    public void setSpiVersion(String mSpiVersion) {
        this.mSpiVersion = mSpiVersion;
    }

    public void setPlayerCookie(String mPlayerCookie) {
        this.mPlayerCookie = mPlayerCookie;
    }

    public PlayerPlaybackInfo getPlayerPlaybackInfo() {
        return mPlayerPlaybackInfo;
    }

    public void setPlayerPlaybackInfo(PlayerPlaybackInfo playerPlaybackInfo) {
        mPlayerPlaybackInfo = playerPlaybackInfo;
    }

    public void setSkillToken(String skillToken) {
        mSkillToken = skillToken;
    }

    public String getSkillToken() {
        return mSkillToken;
    }

    public void setSessionReady(boolean initalized) {
        mSessionReady = initalized;
        if (initalized) {
            refreshPlaybackState();
        }
    }

    public boolean isSessionReady() {
        return mSessionReady;
    }

    public String getSpiVersion() {
        return mSpiVersion;
    }

    public String getPlayerCookie() {
        return mPlayerCookie;
    }

    public void resetUnauthorizedReported() {
        if (mMediaControllerCallback != null)
            mMediaControllerCallback.resetUnauthorizedReported();
    }
}
