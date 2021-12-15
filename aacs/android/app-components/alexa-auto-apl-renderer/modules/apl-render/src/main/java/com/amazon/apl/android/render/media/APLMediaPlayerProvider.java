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
import android.view.TextureView;

import androidx.annotation.NonNull;

import com.amazon.apl.android.dependencies.IMediaPlayer;
import com.amazon.apl.android.providers.impl.MediaPlayerProvider;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;

/**
 * This class extends the Media provider that is used by the
 * Android view host to create a Media player instance.
 */
public class APLMediaPlayerProvider extends MediaPlayerProvider {
    private final IAPLEventSender mAplEventSender;
    private final IAPLTokenProvider mAplTokenProvider;

    public APLMediaPlayerProvider(
            @NonNull IAPLEventSender aplEventSender, @NonNull IAPLTokenProvider aplTokenProvider) {
        super();
        mAplEventSender = aplEventSender;
        mAplTokenProvider = aplTokenProvider;
    }

    @NonNull
    public IMediaPlayer createPlayer(@NonNull Context context, @NonNull TextureView view) {
        return new APLMediaPlayer(context, view, mAplEventSender, mAplTokenProvider);
    }
}
