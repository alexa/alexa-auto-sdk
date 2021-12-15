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

package com.amazon.apl.android.render;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.apl.android.render.dagger.component.ApplicationComponent;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.android.render.network.NetworkExecutor;

import java.util.concurrent.ExecutorService;

/**
 * This class initializes application components and network
 * executor.
 */
public class APLSingleton {
    // class loader init, represents the earliest possible process time.
    //
    // Since Java loads the fields in declaration order, place this at the top.
    private static final long INIT_TIME = System.currentTimeMillis();

    private static final String TAG = APLSingleton.class.getSimpleName();

    private static final APLSingleton INSTANCE = new APLSingleton();

    private ApplicationComponent mApplicationComponent;

    private ExecutorService mExecutorService;

    public static APLSingleton getInstance() {
        return INSTANCE;
    }

    private APLSingleton() {}

    public void init(final Context context, @NonNull IAPLEventSender aplEventSender,
            @NonNull IAPLTokenProvider aplTokenProvider) {
        Log.i(TAG, "Initializing");
        mApplicationComponent = ApplicationComponent.Initializer.init(context, aplEventSender, aplTokenProvider);
        mExecutorService = mApplicationComponent.getExecutorService();

        startNetworkStack();
    }

    private void startNetworkStack() {
        final NetworkExecutor networkExecutor = mApplicationComponent.getNetworkExecutor();

        networkExecutor.execute(() -> mApplicationComponent.getOkHttpClientWrapper().init());
    }

    public ApplicationComponent getApplicationComponent() {
        return mApplicationComponent;
    }
}
