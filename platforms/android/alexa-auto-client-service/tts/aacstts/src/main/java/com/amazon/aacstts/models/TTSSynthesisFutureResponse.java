/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aacstts.models;

import android.content.Context;
import android.speech.tts.SynthesisCallback;

import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;

import java.util.concurrent.CompletableFuture;

/**
 * Class to hold the Objects required to process the TextToSpeech response.
 */
public class TTSSynthesisFutureResponse {
    private static final String TAG = TTSSynthesisFutureResponse.class.getSimpleName();

    private CompletableFuture<Object> mWaitForReplyfuture;
    private CompletableFuture<Object> mWaitForStreamfuture;

    private SynthesisCallback mSynthesisCallback;
    private AACSSender mAACSSender;
    TargetComponent mTarget;
    Context mContext;

    public TTSSynthesisFutureResponse(CompletableFuture<Object> waitForReplyfuture,
            CompletableFuture<Object> waitForStreamFuture, SynthesisCallback synthesisCallback, AACSSender aacsSender,
            TargetComponent target, Context context) {
        mWaitForReplyfuture = waitForReplyfuture;
        mWaitForStreamfuture = waitForStreamFuture;
        mSynthesisCallback = synthesisCallback;
        mAACSSender = aacsSender;
        mTarget = target;
        mContext = context;
    }

    public enum FutureType { WAIT_FOR_REPLY, WAIT_FOR_STREAM }

    public SynthesisCallback getSynthesisCallback() {
        return mSynthesisCallback;
    }

    public AACSSender getAACSSender() {
        return mAACSSender;
    }

    public TargetComponent getTargetComponent() {
        return mTarget;
    }

    public Context getContext() {
        return mContext;
    }

    public void completeFuture(FutureType type) {
        CompletableFuture<Object> future = getFutureObject(type);
        if (future != null && !future.isDone() && !future.isCancelled()) {
            future.complete(new Object());
        }
    }

    public void cancelFuture(FutureType type) {
        CompletableFuture<Object> future = getFutureObject(type);
        if (future != null && !future.isDone() && !future.isCancelled())
            future.cancel(true);
    }

    private CompletableFuture<Object> getFutureObject(FutureType type) {
        if (FutureType.WAIT_FOR_REPLY.equals(type)) {
            return mWaitForReplyfuture;
        } else if (FutureType.WAIT_FOR_STREAM.equals(type)) {
            return mWaitForStreamfuture;
        }
        return null;
    }
}
