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

import androidx.annotation.NonNull;

import com.amazon.apl.android.render.dagger.ApplicationContext;
import com.amazon.apl.android.render.dagger.ApplicationScope;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.android.render.tts.APLTtsPlayerProvider;

import dagger.Module;
import dagger.Provides;

/**
 * Provides an instance of {@link APLTtsPlayerProvider}.
 */
@Module(includes = {ApplicationModule.class})
public class TtsModule {
    /**
     * @return the {@link APLTtsPlayerProvider}.
     */
    @Provides
    @ApplicationScope
    public APLTtsPlayerProvider provideTtsPlayerProvider(@ApplicationContext final Context context,
            @NonNull final IAPLEventSender aplEventSender, @NonNull final IAPLTokenProvider aplTokenProvider) {
        return new APLTtsPlayerProvider(context, aplEventSender, aplTokenProvider);
    }
}
