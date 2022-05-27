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
package com.amazon.alexa.auto.settings.dependencies;

import android.content.Context;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;

/**
 * Module for providing Android Objects.
 */
@Module
public class AndroidModule {
    private WeakReference<Context> mContext;

    /**
     * Constructs the @c AndroidModule.
     *
     * @param context Android Context.
     */
    public AndroidModule(@NonNull Context context) {
        this.mContext = new WeakReference<>(context);
    }

    /**
     * Provides the Android Context.
     *
     * @return Android Context.
     */
    @Provides
    public WeakReference<Context> provideContext() {
        return this.mContext;
    }
}
