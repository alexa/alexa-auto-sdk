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
package com.amazon.alexa.auto.media.session;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;
import com.amazon.alexa.auto.apis.media.MediaSourceInfo;
import com.amazon.alexa.auto.apis.media.PlayerInfoCache;

import java.lang.ref.WeakReference;
import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link com.amazon.alexa.auto.apis.media.PlayerInfoCache}.
 */
public class PlayerInfoCacheImpl implements PlayerInfoCache {
    private static final String TAG = AACSConstants.AACS + "-" + PlayerInfoCacheImpl.class.getSimpleName();
    private final WeakReference<Context> mContextWk;
    private final BehaviorSubject<Optional<RenderPlayerInfo>> mPlayerInfoSubject;
    private final BehaviorSubject<Optional<MediaSourceInfo>> mMediaSourceInfoSubject;
    private Optional<RenderPlayerInfo> mPlayerInfo;
    private Optional<MediaSourceInfo> mMediaSourceInfo;
    private long mPlaybackPosition;

    /**
     * Constructs an instance of PlayerInfoCacheImpl.
     *
     * @param contextWk Android Context.
     */
    public PlayerInfoCacheImpl(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
        mPlayerInfo = Optional.empty();
        mPlayerInfoSubject = BehaviorSubject.createDefault(mPlayerInfo);
        mMediaSourceInfo = Optional.empty();
        mMediaSourceInfoSubject = BehaviorSubject.createDefault(mMediaSourceInfo);
        mPlaybackPosition = 0;
    }

    @Override
    public void setPlayerInfo(@NonNull Optional<RenderPlayerInfo> playerInfo) {
        Log.d(TAG, "setPlayerInfo. Has value:" + playerInfo.isPresent());
        mPlayerInfo = playerInfo;
        mPlayerInfoSubject.onNext(mPlayerInfo);
    }

    @Override
    public Optional<RenderPlayerInfo> getPlayerInfo() {
        return mPlayerInfo;
    }

    @Override
    public Observable<Optional<RenderPlayerInfo>> observePlayerInfo() {
        return mPlayerInfoSubject;
    }

    @Override
    public void setMediaSourceInfo(Optional<MediaSourceInfo> info) {
        if (info.isPresent()) {
            Log.d(TAG, "setMediaSourceInfo. URL: " + info.get().getUrl());
        } else {
            Log.d(TAG, "setMediaSourceInfo empty");
        }

        mMediaSourceInfo = info;
        mMediaSourceInfoSubject.onNext(mMediaSourceInfo);
    }

    @Override
    public Optional<MediaSourceInfo> getMediaSourceInfo() {
        return mMediaSourceInfo;
    }

    @Override
    public Observable<Optional<MediaSourceInfo>> observeMediaSourceInfo() {
        return mMediaSourceInfoSubject;
    }

    @Override
    public void setPlaybackPosition(long position) {
        Log.d(TAG, "setPlaybackPosition. position: " + position);
        mPlaybackPosition = position;
    }

    @Override
    public long getPlaybackPosition() {
        return mPlaybackPosition;
    }

    @Override
    public void clearAllData() {
        setMediaSourceInfo(Optional.empty());
        setPlaybackPosition(0);
        setPlayerInfo(Optional.empty());
    }
}
