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
package com.amazon.apl.android.render.media;

import android.content.Context;
import android.media.AudioManager;
import android.util.Log;
import android.view.TextureView;

import androidx.annotation.NonNull;

import com.amazon.apl.android.dependencies.impl.MediaPlayer;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;

/**
 * Provides a wrapper of the MediaPlayer player that can report activity events.
 * The Media player is used for playing video content. Android audio focus
 * management is built into the base MediaPlayer class.
 */
public class APLMediaPlayer extends MediaPlayer {
    private static final String TAG = APLMediaPlayer.class.getSimpleName();

    private final IAPLEventSender mAplEventSender;
    private final IAPLTokenProvider mAplTokenProvider;

    public APLMediaPlayer(@NonNull Context context, @NonNull TextureView view, @NonNull IAPLEventSender aplEventSender,
            @NonNull IAPLTokenProvider aplTokenProvider) {
        super(context, view);
        Log.v(TAG, "Created");
        mAplEventSender = aplEventSender;
        mAplTokenProvider = aplTokenProvider;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void play() {
        Log.v(TAG, "play:");
        super.play();
        mAplEventSender.sendActivityEventRequest(mAplTokenProvider.getToken(), IAPLEventSender.ActivityEvent.ACTIVATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void stop() {
        Log.v(TAG, "stop:");
        super.stop();
        mAplEventSender.sendActivityEventRequest(
                mAplTokenProvider.getToken(), IAPLEventSender.ActivityEvent.DEACTIVATED);
    }
}
