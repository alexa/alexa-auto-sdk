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
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.android.thread.Threading;

import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;

import dagger.Module;
import dagger.Provides;

/**
 * Provides the application context.
 */
@Module
public class ApplicationModule {
    /**
     * Application context.
     */
    private final WeakReference<Context> mContext;

    /**
     * Object used to send events back to the cloud.
     */
    private final IAPLEventSender mAplEventSender;

    /**
     * Retrieves the token of the rendered APL document.
     */
    private final IAPLTokenProvider mAplTokenProvider;

    /**
     * Instantiates this module with the application context.
     *
     * @param context The activity context.
     * @param aplEventSender The object used to send events back to cloud.
     * @param aplTokenProvider The object that provides the current presentation token.
     */
    public ApplicationModule(@ApplicationContext final Context context, IAPLEventSender aplEventSender,
            IAPLTokenProvider aplTokenProvider) {
        // Ensure application context
        mContext = new WeakReference<>(context);
        mAplEventSender = aplEventSender;
        mAplTokenProvider = aplTokenProvider;
    }

    /**
     * @return The application {@link Context}
     */
    @Provides
    @ApplicationScope
    @ApplicationContext
    Context provideContext() {
        return mContext.get();
    }

    /**
     * @return The executor service {@link ExecutorService}
     */
    @Provides
    @ApplicationScope
    ExecutorService provideExecutorService() {
        return Threading.THREAD_POOL_EXECUTOR;
    }

    /**
     * @return The APL event sender {@link IAPLEventSender}
     */
    @Provides
    @ApplicationScope
    IAPLEventSender provideAplEventSender() {
        return mAplEventSender;
    }

    /**
     * @return The presentation token provider {@link IAPLTokenProvider}
     */
    @Provides
    @ApplicationScope
    IAPLTokenProvider provideAplTokenProvider() {
        return mAplTokenProvider;
    }
}
