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

import com.amazon.apl.android.render.dagger.ApplicationContext;
import com.amazon.apl.android.render.dagger.ApplicationScope;
import com.amazon.apl.android.render.network.NetworkExecutor;
import com.amazon.apl.android.render.network.OkHttpClientWrapper;

import dagger.Lazy;
import dagger.Module;
import dagger.Provides;
import okhttp3.Cache;
import okhttp3.OkHttpClient;

/**
 * Provides an instance of {@link OkHttpClient} and the application cache.
 */
@Module(includes = {ApplicationModule.class})
public class NetworkModule {
    private static final long CACHE_SIZE = 4L * 1024L * 1024L; // cache size 4MiB on disk

    /**
     * Provides an instance of {@link OkHttpClient}.
     *
     * @param cache The cache to be used by the OkHttpClient.
     * @return An instance of {@link OkHttpClient}.
     */
    @Provides
    @ApplicationScope
    OkHttpClient provideOkHttpClient(final Cache cache) {
        return new OkHttpClient.Builder().cache(cache).build();
    }

    /**
     * Provides a wrapper around {@link OkHttpClient} to handle cache control and refreshes.
     *
     * @param okHttpClient the OkHttp client
     * @return An instance of {@link OkHttpClientWrapper}
     */
    @Provides
    @ApplicationScope
    OkHttpClientWrapper provideOkHttpClientWrapper(Lazy<OkHttpClient> okHttpClient) {
        return new OkHttpClientWrapper(okHttpClient);
    }

    /**
     * Provides an instance of the application {@link Cache}.
     *
     * @param context The application {@link Context}.
     * @return An instance of the application {@link Cache}.
     */
    @Provides
    @ApplicationScope
    Cache provideCache(@ApplicationContext final Context context) {
        return new Cache(context.getCacheDir(), CACHE_SIZE);
    }

    @Provides
    @ApplicationScope
    NetworkExecutor provideNetworkExecutor() {
        return new NetworkExecutor();
    }
}
