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

package com.amazon.apl.android.render.tts;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.apl.android.dependencies.ITtsPlayer;
import com.amazon.apl.android.dependencies.ITtsSourceProvider;
import com.amazon.apl.android.providers.ITtsPlayerProvider;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;

import java.net.MalformedURLException;
import java.net.URL;

/**
 * This class implements the TTS provider that is used by the
 * Android view host to create a TTS player instance.
 */
public class APLTtsPlayerProvider implements ITtsPlayerProvider {
    private static final String TAG = APLTtsPlayerProvider.class.getSimpleName();

    private Context mContext;
    private APLTtsPlayer mTtsPlayer;
    private IAPLEventSender mAplEventSender;
    private IAPLTokenProvider mAplTokenProvider;

    public APLTtsPlayerProvider(@NonNull Context context, @NonNull IAPLEventSender aplEventSender,
            @NonNull IAPLTokenProvider aplTokenProvider) {
        mContext = context;
        mAplEventSender = aplEventSender;
        mAplTokenProvider = aplTokenProvider;
    }

    @Override
    public ITtsPlayer getPlayer() {
        if (mTtsPlayer == null) {
            mTtsPlayer = new APLTtsPlayer(mContext, mAplEventSender, mAplTokenProvider);
            Log.v(TAG, "Created TTS player");
        }
        return mTtsPlayer;
    }

    @Override
    public void prepare(@NonNull String source, @NonNull ITtsSourceProvider ttsSourceProvider) {
        try {
            Log.v(TAG, "prepare: " + source);
            ttsSourceProvider.onSource(new URL(source));
        } catch (MalformedURLException e) {
            Log.e(TAG, "Malformed TTS Url " + source);
        }
    }

    @Override
    public void prepare(@NonNull String source) {
        try {
            getPlayer().prepare(source, new URL(source));
        } catch (Exception e) {
            Log.e(TAG, "Could not set the speech source", e);
        }
    }

    @Override
    public void onDocumentFinish() {
        if (mTtsPlayer != null) {
            Log.i(TAG, "Releasing TTS player");
            mTtsPlayer.release();
            mTtsPlayer = null;
        }
    }
}
