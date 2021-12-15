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

import android.app.PendingIntent;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.support.v4.media.RatingCompat;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.Directive.AdjustSeekDirective;
import com.amazon.maccandroid.model.Directive.Directive;
import com.amazon.maccandroid.model.Directive.LoginDirective;
import com.amazon.maccandroid.model.Directive.LogoutDirective;
import com.amazon.maccandroid.model.Directive.PlayControlDirective;
import com.amazon.maccandroid.model.Directive.PlayDirective;
import com.amazon.maccandroid.model.Directive.SeekDirective;
import com.amazon.maccandroid.model.SupportedOperations;
import com.amazon.maccandroid.model.errors.CapabilityAgentError;
import com.amazon.maccandroid.model.players.AuthorizedPlayer;

import java.util.List;
import java.util.Map;

public class MediaAppsDirectivesHandler implements MediaAppsConnectionListener {
    private static final String TAG = MediaAppsDirectivesHandler.class.getSimpleName();
    private static final String ALEXA_SCHEMA = "alexa";
    private static final String INDEX_KEY = "index";
    private static final String OFFSET_MILLISECONDS_KEY = "offsetInMilliseconds";
    private static final String NAVIGATION_KEY = "navigation";

    private static final String SCHEMA_SEPARATOR = ":";
    private static final String QUERY_INITIATOR = "?";
    private static final String QUERY_ASSIGNER = "=";
    private static final String QUERY_SEPARATOR = "&";

    private static final String SPOTIFY_SPECIFIC_URI = "spotify";

    private final Context mContext;
    private final Handler mMainThreadDirectivesHandler;
    private final SupportedOperations mSupportedOperations;

    private Directive mCachedDirective = null;

    public MediaAppsDirectivesHandler(Context context, SupportedOperations supportedOperations) {
        mContext = context;
        mMainThreadDirectivesHandler = new Handler(Looper.getMainLooper());
        mSupportedOperations = supportedOperations;
    }

    public void handleDirective(final Directive directive) {
        Log.i(TAG,
                "handleDirective | directive type: " + directive.getClass().getSimpleName()
                        + " | for player id: " + directive.getPlayerId());

        Map<String, MediaApp> mediaApps = MediaAppsRepository.getInstance().getAuthorizedMediaApps();
        for (String app : mediaApps.keySet()) {
            Log.i(TAG, app);
        }

        if (mediaApps.containsKey(directive.getPlayerId())) {
            MediaApp mediaApp = mediaApps.get(directive.getPlayerId());
            MediaControllerCompat mediaController = mediaApp.getMediaController();

            if (mediaController != null) { // && mediaController.isSessionReady() ) {
                Log.i(TAG, "handleDirective| mediacontroller is already connected");
                handleDirectiveForSession(directive, mediaController);
            } else {
                if (isAppInstalled(mContext, mediaApp.getLocalPlayerId())) {
                    Log.i(TAG, "handleDirective| app is already installed trying to connect");
                    mediaApp.connect(this);
                    mCachedDirective = directive;
                } else {
                    Log.i(TAG, "error");
                    MediaAppsStateReporter.getInstance().reportError(
                            directive.getPlayerId(), CapabilityAgentError.PLAYER_NOT_FOUND);
                }
            }
        } else {
            Log.i(TAG, "handleDirective | player id not found");
            MediaAppsStateReporter.getInstance().reportError(
                    directive.getPlayerId(), CapabilityAgentError.PLAYER_UNKNOWN);
        }
    }

    private void handleDirectiveForSession(Directive directive, MediaControllerCompat mediaController) {
        //         if (!actionSupported(directive, mediaController)) {
        //             Log.i(TAG, "Action is not supported");
        //             MediaAppsStateReporter.getInstance().reportError(directive.getPlayerId(),
        //                     CapabilityAgentError.OPERATION_UNSUPPORTED);
        //             return;
        //         }

        final MediaControllerCompat finalMediaController = mediaController;

        final MediaApp finalMediaApp =
                MediaAppsRepository.getInstance().getAuthorizedMediaApps().get(directive.getPlayerId());

        if (directive instanceof LoginDirective) {
            LoginDirective loginDirective = (LoginDirective) directive;
            Bundle extras = new Bundle();
            extras.putString("accessToken", (loginDirective.getAccessToken()));
            extras.putString("username", (loginDirective.getUserName()));
            extras.putInt("tokenRefreshIntervalInMilliseconds", (int) loginDirective.getRefreshInterval());
            extras.putBoolean("forceLogin", loginDirective.isForce());
            mediaController.getTransportControls().sendCustomAction(
                    APIConstants.Directives.ExternalMediaPlayerDirectives.CUSTOM_ACTION_LOGIN, extras);
        } else if (directive instanceof LogoutDirective) {
            // IGNORE
        } else if (directive instanceof PlayDirective) {
            PlayDirective playDirective = (PlayDirective) directive;
            Log.i(TAG, "handleDirectiveForSession| in play directive : " + playDirective);
            final Uri playUri = constructPlayUri(playDirective);

            // if user tries to play with unauthorized Spotify, allow error to be sent again
            finalMediaApp.resetUnauthorizedReported();

            if (playDirective.isPreload()) {
                Log.i(TAG, "handleDirectiveForSession | is preload |  media controller | here is the URI: " + playUri);
                mMainThreadDirectivesHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        finalMediaController.getTransportControls().prepareFromUri(playUri, null);
                    }
                });
            } else {
                Log.i(TAG, "handleDirectiveForSession | not preload |  media controller | here is the URI: " + playUri);
                //                finalMediaController.getTransportControls().playFromUri(playUri, null);
                mMainThreadDirectivesHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        finalMediaController.getTransportControls().playFromUri(playUri, null);
                    }
                });
            }

        } else if (directive instanceof PlayControlDirective) {
            PlayControlDirective playControlDirective = (PlayControlDirective) directive;
            // TODO | once we get finalized PlayControlDirective types from the engine we can declare
            // TODO | actual final Strings for all the actions
            switch (playControlDirective.getType()) {
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_PLAY:
                    Log.i(TAG, "PlayControlDirective: play");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().play();
                            ;
                        }
                    });

                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_STOP:
                    Log.i(TAG, "PlayControlDirective: stop");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().stop();
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_PAUSE:
                    Log.i(TAG, "PlayControlDirective: pause");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().pause();
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_STARTOVER:
                    Log.i(TAG, "PlayControlDirective: start over");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().seekTo(0);
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_PREVIOUS:
                    Log.i(TAG, "PlayControlDirective: previous");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().skipToPrevious();
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_NEXT:
                    Log.i(TAG, "PlayControlDirective: next");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().skipToNext();
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_REWIND:
                    Log.i(TAG, "PlayControlDirective: rewind");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().rewind();
                            ;
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_FASTFORWARD:
                    Log.i(TAG, "PlayControlDirective: fastforward");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().fastForward();
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT:
                    Log.i(TAG, "PlayControlDirective: enable repeat");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setRepeatMode(
                                    PlaybackStateCompat.REPEAT_MODE_ALL);
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT_ONE:
                    Log.i(TAG, "PlayControlDirective: enable repeat one");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setRepeatMode(
                                    PlaybackStateCompat.REPEAT_MODE_ONE);
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_REPEAT:
                    Log.i(TAG, "PlayControlDirective: disable repeat");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setRepeatMode(
                                    PlaybackStateCompat.REPEAT_MODE_NONE);
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_SHUFFLE:
                    Log.i(TAG, "PlayControlDirective: enable shuffle");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setShuffleMode(
                                    PlaybackStateCompat.SHUFFLE_MODE_ALL);
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_SHUFFLE:
                    Log.i(TAG, "PlayControlDirective: disable shuffle");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setShuffleMode(
                                    PlaybackStateCompat.SHUFFLE_MODE_NONE);
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_FAVORITE:
                    Log.i(TAG, "PlayControlDirective: favorite");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setRating(RatingCompat.newThumbRating(true));
                        }
                    });
                    break;
                case APIConstants.Directives.PlayControl.PLAY_CONTROL_UNFAVORITE:
                    Log.i(TAG, "PlayControlDirective: unfavorite");
                    mMainThreadDirectivesHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            finalMediaController.getTransportControls().setRating(RatingCompat.newThumbRating(false));
                        }
                    });
                    break;
            }
        } else if (directive instanceof SeekDirective) {
            mediaController.getTransportControls().seekTo(((SeekDirective) directive).getOffset());
        } else if (directive instanceof AdjustSeekDirective) {
            PlaybackStateCompat playbackState = mediaController.getPlaybackState();
            long position = (long) (playbackState.getPosition()
                    + playbackState.getPlaybackSpeed()
                            * (SystemClock.elapsedRealtime() - playbackState.getLastPositionUpdateTime())
                    + ((AdjustSeekDirective) directive).getDelta());
            mediaController.getTransportControls().seekTo(position);
        } else {
            Log.e(TAG, "Unsupported directive " + directive);
            MediaAppsStateReporter.getInstance().reportError(
                    directive.getPlayerId(), CapabilityAgentError.OPERATION_UNSUPPORTED);
        }
    }

    private Uri constructPlayUri(PlayDirective playDirective) {
        StringBuilder uriBuilder = new StringBuilder();
        // Spotify has a different way of building the URI. The others are as per the MACC requirements.
        if (playDirective.getToken().startsWith(SPOTIFY_SPECIFIC_URI)) {
            uriBuilder.append(ALEXA_SCHEMA)
                    .append(SCHEMA_SEPARATOR)
                    .append(playDirective.getToken())
                    .append(QUERY_INITIATOR);

            constructQuery(INDEX_KEY, String.valueOf(playDirective.getIndex()), uriBuilder).append(QUERY_SEPARATOR);

            constructQuery(OFFSET_MILLISECONDS_KEY, String.valueOf(playDirective.getOffest()), uriBuilder)
                    .append(QUERY_SEPARATOR);

            constructQuery(NAVIGATION_KEY, playDirective.getNavigation(), uriBuilder);

            return Uri.parse(uriBuilder.toString());
        } else {
            Uri.Builder builder = new Uri.Builder();
            builder.scheme(ALEXA_SCHEMA)
                    .path(playDirective.getToken())
                    .appendQueryParameter(INDEX_KEY, String.valueOf(playDirective.getIndex()))
                    .appendQueryParameter(OFFSET_MILLISECONDS_KEY, String.valueOf(playDirective.getOffest()))
                    .appendQueryParameter(NAVIGATION_KEY, playDirective.getNavigation());
            return builder.build();
        }
    }

    private StringBuilder constructQuery(final String key, final String value, final StringBuilder uriBuilder) {
        return uriBuilder.append(key).append(QUERY_ASSIGNER).append(value);
    }

    private boolean isAppInstalled(Context context, String packageName) {
        Log.i(TAG, "isAppInstalled:" + packageName);
        boolean installed = true;
        try {
            context.getPackageManager().getApplicationInfo(packageName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            // This should not happen. If we get a media session for a package, then the
            // package must be installed on the device.
            Log.e(TAG, "Unable to load package details", e);
            installed = false;
        }
        return installed;
    }

    private void startSessionActivity(MediaControllerCompat mediaController) {
        PendingIntent intent = mediaController.getSessionActivity();
        if (intent != null) {
            try {
                intent.send();
                return;
            } catch (PendingIntent.CanceledException e) {
                e.printStackTrace();
                Log.e(TAG, e.toString());
            }
        }
        Log.w(TAG, "Failed to open app by session activity.");
    }

    private boolean actionSupported(Directive directive, MediaControllerCompat mediaController) {
        String action = null;
        if (directive instanceof PlayControlDirective) {
            action = ((PlayControlDirective) directive).getType();
        } else if (directive instanceof SeekDirective) {
            //            action = SupportedOperations.SET_SEEK_POSITION;
            // Spotify is not reporting this TODO
            return true;
        } else {
            return true;
        }
        return mSupportedOperations
                .getSupportedOperations(
                        MediaAppsRepository.getInstance().getAuthorizedMediaApp(directive.getPlayerId()))
                .contains(action);
    }

    @Override
    public void onConnectionSuccessful() {
        Log.i(TAG, "onConnectionSuccessful to : " + mCachedDirective.getPlayerId());
        Log.i(TAG,
                "Authorized app: "
                        + MediaAppsRepository.getInstance().getAuthorizedMediaApp(mCachedDirective.getPlayerId()));
        MediaControllerCompat controller = MediaAppsRepository.getInstance()
                                                   .getAuthorizedMediaApp(mCachedDirective.getPlayerId())
                                                   .getMediaController();
        //        if (controller.isSessionReady()) {
        Log.i(TAG, "Session ready");
        handleDirectiveForSession(mCachedDirective, controller);
        //        } else {
        //            Log.i(TAG, "session not ready");
        //        }
        mCachedDirective = null;
    }

    @Override
    public void onConnectionFailure(CapabilityAgentError error) {
        Log.i(TAG, "onConnectionFailure");
        MediaAppsRepository.getInstance().removeMediaApp(mCachedDirective.getPlayerId());
        MediaAppsStateReporter.getInstance().reportError(mCachedDirective.getPlayerId(), error);
    }

    public void handleAuthorizedPlayers(List<AuthorizedPlayer> players) {
        Log.i(TAG, "handleAuthorizedPlayers");
        for (AuthorizedPlayer player : players) {
            if (MediaAppsRepository.getInstance().isDiscoveredApp(player.getLocalPlayerId())) {
                Log.i(TAG, "handleAuthorizedPlayers | found local player id");
                MediaApp app = MediaAppsRepository.getInstance().getDiscoveredMediaApp(player.getLocalPlayerId());
                if (player.isAuthorized()) {
                    // App is new and has not been added before
                    if ((MediaAppsRepository.getInstance().getAuthorizedMediaApp(player.getLocalPlayerId()) == null)) {
                        MediaAppsRepository.getInstance().addAuthorizedMediaApp(app);
                        // Now that app is authorized, send up any playbackstate events necessary
                        // for the cloud
                        app.refreshPlaybackState();
                    }
                } else {
                    Log.i(TAG, "Player " + player.getLocalPlayerId() + " was not authorized");
                }
            }
        }
    }
}
