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

import com.amazon.apl.android.APLOptions;
import com.amazon.apl.android.RootConfig;
import com.amazon.apl.android.audio.RuntimeAudioPlayerFactory;
import com.amazon.apl.android.dependencies.IOpenUrlCallback;
import com.amazon.apl.android.dependencies.impl.OpenUrlCallback;
import com.amazon.apl.android.media.RuntimeMediaPlayerFactory;
import com.amazon.apl.android.render.BuildConfig;
import com.amazon.apl.android.render.dagger.ActivityContext;
import com.amazon.apl.android.render.dagger.ActivityScope;
import com.amazon.apl.android.render.media.APLMediaPlayerProvider;
import com.amazon.apl.android.render.tts.APLTtsPlayerProvider;

import java.util.HashMap;
import java.util.Map;

import dagger.Module;
import dagger.Provides;

/**
 * Provides {@link com.amazon.apl.android.APLOptions.Builder}
 */
@Module(includes = {})
public class APLOptionsModule {
    /**
     * @return an instance of {@link APLOptions}.
     */
    @Provides
    APLOptions.Builder provideAPLOptions(final IOpenUrlCallback openUrlCallback) {
        return APLOptions.builder().openUrlCallback(openUrlCallback);
    }

    /**
     * @return an instance of {@link IOpenUrlCallback}.
     */
    @Provides
    @ActivityScope
    public IOpenUrlCallback provideOpenUrl(@ActivityContext final Context context) {
        return new OpenUrlCallback(context);
    }

    /**
     * @param context the activity context
     * @return an instance of {@link RootConfig}
     */
    @Provides
    public RootConfig provideRootConfig(@ActivityContext final Context context, final APLTtsPlayerProvider ttsProvider,
            final APLMediaPlayerProvider mediaProvider) {
        Map<String, String> autoEnvironmentValues = new HashMap<>();
        autoEnvironmentValues.put("drivingState", "moving");
        return RootConfig.create(context)
                .agent(BuildConfig.VERSION_NAME, "1.9")
                .registerDataSource("dynamicIndexList")
                .registerDataSource("dynamicTokenList")
                .setEnvironmentValue("automobile", autoEnvironmentValues)
                .audioPlayerFactory(new RuntimeAudioPlayerFactory(ttsProvider))
                .mediaPlayerFactory(new RuntimeMediaPlayerFactory(mediaProvider))
                .allowOpenUrl(false);
    }
}
