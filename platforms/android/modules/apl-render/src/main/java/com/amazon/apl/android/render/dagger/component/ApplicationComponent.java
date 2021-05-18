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

package com.amazon.apl.android.render.dagger.component;

import android.content.Context;

import com.amazon.apl.android.render.dagger.ApplicationContext;
import com.amazon.apl.android.render.dagger.ApplicationScope;
import com.amazon.apl.android.render.dagger.module.ApplicationModule;
import com.amazon.apl.android.render.dagger.module.MediaPlayerModule;
import com.amazon.apl.android.render.dagger.module.NetworkModule;
import com.amazon.apl.android.render.dagger.module.TtsModule;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.android.render.media.APLMediaPlayerProvider;
import com.amazon.apl.android.render.network.NetworkExecutor;
import com.amazon.apl.android.render.network.OkHttpClientWrapper;
import com.amazon.apl.android.render.tts.APLTtsPlayerProvider;

import java.util.concurrent.ExecutorService;

import dagger.Component;

/**
 * This provides all the application level dependencies.
 */
@ApplicationScope
@Component(modules = {MediaPlayerModule.class, NetworkModule.class, TtsModule.class})
public interface ApplicationComponent {
    final class Initializer {
        public static ApplicationComponent init(
                Context context, IAPLEventSender aplEventSender, IAPLTokenProvider aplTokenProvider) {
            return DaggerApplicationComponent.builder()
                    .applicationModule(new ApplicationModule(context, aplEventSender, aplTokenProvider))
                    .build();
        }
    }

    /**
     * @return The application context.
     */
    @ApplicationContext
    Context getContext();

    /**
     * @return The {@link OkHttpClientWrapper}.
     */
    OkHttpClientWrapper getOkHttpClientWrapper();

    /**
     * @return The {@link APLTtsPlayerProvider}
     */
    APLTtsPlayerProvider getTtsPlayerProvider();

    /**
     * @return The {@link NetworkExecutor}.
     */
    NetworkExecutor getNetworkExecutor();

    /**
     * @return the application level executor service.
     */
    ExecutorService getExecutorService();

    /**
     * @return The {@link APLMediaPlayerProvider}
     */
    APLMediaPlayerProvider getMediaPlayerProvider();
}
