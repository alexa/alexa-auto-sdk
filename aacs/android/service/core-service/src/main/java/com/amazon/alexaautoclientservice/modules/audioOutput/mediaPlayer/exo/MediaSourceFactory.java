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

package com.amazon.alexaautoclientservice.modules.audioOutput.mediaPlayer.exo;

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.MediaItem;
import com.google.android.exoplayer2.source.LoadEventInfo;
import com.google.android.exoplayer2.source.MediaLoadData;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.MediaSourceEventListener;
import com.google.android.exoplayer2.source.ProgressiveMediaSource;
import com.google.android.exoplayer2.source.dash.DashMediaSource;
import com.google.android.exoplayer2.source.dash.DefaultDashChunkSource;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.source.smoothstreaming.DefaultSsChunkSource;
import com.google.android.exoplayer2.source.smoothstreaming.SsMediaSource;
import com.google.android.exoplayer2.upstream.*;
import com.google.android.exoplayer2.util.Util;

import java.io.IOException;

class MediaSourceFactory {
    private static final String TAG = AACSConstants.AACS + "-" + MediaSourceFactory.class.getSimpleName();
    private static final String USER_AGENT_NAME = "com.amazon.alexaautoclientservice";
    private static final int CONNECTION_TIMEOUT_MS = 8000;
    private static final int READ_TIMEOUT_MS = 20000;

    private final Context mContext;
    private final String mName;
    private final Handler mMainHandler;
    private final PlaylistParser mPlaylistParser = new PlaylistParser();
    private final MediaSourceListener mMediaSourceListener = new MediaSourceListener();
    private final DataSource.Factory mFileDataSourceFactory = new FileDataSource.Factory();
    private final DataSource.Factory mHttpDataSourceFactory;

    MediaSourceFactory(@NonNull Context context, @NonNull String name, @NonNull Handler handler) {
        mContext = context;
        mName = name;
        mMainHandler = handler;
        mHttpDataSourceFactory = buildHttpDataSourceFactory(mContext);
    }

    private HttpDataSource.Factory buildHttpDataSourceFactory(Context context) {
        String userAgent = Util.getUserAgent(context, USER_AGENT_NAME);
        DefaultHttpDataSource.Factory factory = new DefaultHttpDataSource.Factory();
        factory.setUserAgent(userAgent);
        factory.setConnectTimeoutMs(CONNECTION_TIMEOUT_MS);
        factory.setReadTimeoutMs(READ_TIMEOUT_MS);
        factory.setAllowCrossProtocolRedirects(true);
        return factory;
    }

    MediaSource createFileMediaSource(@NonNull final Uri uri) throws Exception {
        Log.v(TAG, String.format("Creating file media source. URI=%s", uri));
        return createMediaSource(uri, mFileDataSourceFactory, mMediaSourceListener, mMainHandler, mPlaylistParser);
    }

    MediaSource createHttpMediaSource(@NonNull final Uri uri) throws Exception {
        Log.v(TAG, String.format("Creating http media source. URI=%s", uri));
        return createMediaSource(uri, mHttpDataSourceFactory, mMediaSourceListener, mMainHandler, mPlaylistParser);
    }

    private static MediaSource createMediaSource(final Uri uri, final DataSource.Factory dataSourceFactory,
            final MediaSourceEventListener mediaSourceListener, final Handler handler,
            final PlaylistParser playlistParser) throws Exception {
        MediaType type = MediaType.inferContentType(uri.getLastPathSegment());
        MediaItem mediaItem = MediaItem.fromUri(uri);
        switch (type) {
            case DASH:
                DashMediaSource dashMediaSource =
                        new DashMediaSource
                                .Factory(new DefaultDashChunkSource.Factory(dataSourceFactory), dataSourceFactory)
                                .createMediaSource(mediaItem);
                dashMediaSource.addEventListener(handler, mediaSourceListener);
                return dashMediaSource;
            case SMOOTH_STREAMING:
                SsMediaSource ssMediaSource =
                        new SsMediaSource
                                .Factory(new DefaultSsChunkSource.Factory(dataSourceFactory), dataSourceFactory)
                                .createMediaSource(mediaItem);
                ssMediaSource.addEventListener(handler, mediaSourceListener);
                return ssMediaSource;
            case HLS:
                HlsMediaSource hlsMediaSource =
                        new HlsMediaSource.Factory(dataSourceFactory).createMediaSource(mediaItem);
                hlsMediaSource.addEventListener(handler, mediaSourceListener);
                return hlsMediaSource;
            case M3U:
            case PLS:
                Uri parsedUri = playlistParser.parseUri(uri);
                return createMediaSource(parsedUri, dataSourceFactory, mediaSourceListener, handler, playlistParser);
            case OTHER:
                ProgressiveMediaSource progressiveMediaSource =
                        new ProgressiveMediaSource.Factory(dataSourceFactory).createMediaSource(mediaItem);
                progressiveMediaSource.addEventListener(handler, mediaSourceListener);
                return progressiveMediaSource;
            default:
                throw new IllegalStateException("Unsupported type");
        }
    }

    //
    // Media types for creating an ExoPlayer MediaSource
    //
    enum MediaType {
        DASH(C.TYPE_DASH),
        SMOOTH_STREAMING(C.TYPE_SS),
        HLS(C.TYPE_HLS),
        OTHER(C.TYPE_OTHER),
        M3U(4),
        PLS(5);

        private final int mType;

        MediaType(int type) {
            mType = type;
        }

        public int getType() {
            return mType;
        }

        public static MediaType inferContentType(@Nullable final String fileExtension) {
            if (fileExtension == null) {
                return OTHER;
            } else if (fileExtension.endsWith(".ashx") || fileExtension.endsWith(".m3u")) {
                return M3U;
            } else if (fileExtension.endsWith(".pls")) {
                return PLS;
            } else {
                int type = Util.inferContentType(fileExtension);
                for (MediaType mediaType : MediaType.values()) {
                    if (mediaType.getType() == type)
                        return mediaType;
                }
                return OTHER;
            }
        }
    }

    //
    // Media Source event listener
    //
    private class MediaSourceListener implements MediaSourceEventListener {
        private int mRetryCount = 0;

        @Override
        public void onLoadStarted(int windowIndex, MediaSource.MediaPeriodId mediaPeriodId, LoadEventInfo loadEventInfo,
                MediaLoadData mediaLoadData) {
            mRetryCount = 1;
            Log.v(TAG, String.format("(%s) Load media started.", mName));
        }

        @Override
        public void onLoadCompleted(int windowIndex, MediaSource.MediaPeriodId mediaPeriodId,
                LoadEventInfo loadEventInfo, MediaLoadData mediaLoadData) {
            mRetryCount = 0;
            Log.v(TAG, String.format("(%s) Load media completed.", mName));
        }

        @Override
        public void onLoadCanceled(int windowIndex, MediaSource.MediaPeriodId mediaPeriodId,
                LoadEventInfo loadEventInfo, MediaLoadData mediaLoadData) {
            Log.v(TAG, String.format("(%s) Load media cancelled.", mName));
            mRetryCount = 0;
        }

        @Override
        public void onLoadError(int windowIndex, MediaSource.MediaPeriodId mediaPeriodId, LoadEventInfo loadEventInfo,
                MediaLoadData mediaLoadData, IOException error, boolean wasCanceled) {
            Log.v(TAG, String.format("(%s) Error loading media. Attempts: %s", mName, mRetryCount));
            mRetryCount++;
        }
    }
}
