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

package com.amazon.apl.android.render.dagger.module;

import android.content.Context;

import com.amazon.apl.android.providers.IImageLoaderProvider;
import com.amazon.apl.android.providers.impl.GlideImageLoaderProvider;
import com.amazon.apl.android.render.dagger.ActivityContext;
import com.amazon.apl.android.render.dagger.ActivityScope;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;
import lombok.NonNull;

/**
 * Provides the activity context.
 */
@Module
public class ActivityModule {
    /**
     * Activity context.
     */
    private final WeakReference<Context> mContext;

    /**
     * Instantiates this module with the activity context.
     *
     * @param context The context.
     */
    public ActivityModule(@ActivityContext final Context context) {
        // Ensure application context
        mContext = new WeakReference<>(context);
    }

    /**
     * @return The activity {@link Context}
     */
    @Provides
    @ActivityScope
    @ActivityContext
    Context provideContext() {
        return mContext.get();
    }

    @Provides
    @ActivityScope
    IImageLoaderProvider provideImageLoader() {
        return new GlideImageLoaderProvider();
    }
}
