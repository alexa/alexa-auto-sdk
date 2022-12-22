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
package com.amazon.alexa.auto.app;

import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.getModuleAsync;
import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.getModuleSync;

import android.app.Application;
import android.os.Handler;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.app.dependencies.AndroidAppModule;
import com.amazon.alexa.auto.app.dependencies.AppComponent;
import com.amazon.alexa.auto.app.dependencies.DaggerAppComponent;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.app.BuildConfig;

import java.lang.Thread.UncaughtExceptionHandler;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

import javax.inject.Inject;

import androidx.annotation.NonNull;

/**
 * Application class which also provides component registry to constituent
 * components through implementation of {@link AlexaApp}
 */
public class AutoApplication extends Application implements AlexaApp {
    private static final String TAG = AutoApplication.class.getSimpleName();

    // Version code and name
    private String versionCode = Integer.toString(BuildConfig.VERSION_CODE);
    private String versionName = BuildConfig.VERSION_NAME;

    private AppComponent mDaggerAppComponent;
    @Inject
    AlexaAppRootComponent mAppRootComponent;

    @Override
    public void onCreate() {
        super.onCreate();
        mDaggerAppComponent =
                DaggerAppComponent.builder().androidAppModule(new AndroidAppModule(getApplicationContext())).build();

        mDaggerAppComponent.inject(this);

        List<ModuleInterface> moduleInterfaces = getModuleSync(getApplicationContext()).orElse(Arrays.asList());
        Log.i(TAG, "Alexa Auto App getModuleSync done");

        for (ModuleInterface moduleInterface : moduleInterfaces) {
            moduleInterface.initialize(getApplicationContext());
        }
        Log.i(TAG, "Alexa Auto App initialized");
        Log.i(TAG, "APK Version Code: " + versionCode);
        Log.i(TAG, "APK Version Name: " + versionName);

        //TODO: Remove once Google fix ready: https://issuetracker.google.com/issues/245258072
        setupDefaultUncaughtExceptionHeader();
    }

    /**
     * Provide Dagger Component for injection into the android modules
     * of this project.
     *
     * @return Dagger application scope object.
     */
    public AppComponent getAppComponent() {
        return mDaggerAppComponent;
    }

    @Override
    public AlexaAppRootComponent getRootComponent() {
        // This method will mostly be used by external projects which
        // would like to get their dependencies from the returned
        // component.
        return mAppRootComponent;
    }

    /**
     * Workaround solution: https://issuetracker.google.com/issues/245258072#comment39 to avoid
     * application to be killed after failed to deliver a broadcast
     */
    private void setupDefaultUncaughtExceptionHeader() {
        UncaughtExceptionHandler uncaughtExceptionHandler =
            Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(new CustomUncaughtExceptionHandler(uncaughtExceptionHandler));
    }

    private static class CustomUncaughtExceptionHandler implements UncaughtExceptionHandler {
        private static final String CANNOT_DELIVER_BROADCAST_EXCEPTION =
            "CannotDeliverBroadcastException";

        private final UncaughtExceptionHandler handler;

        CustomUncaughtExceptionHandler(UncaughtExceptionHandler handler) {
            this.handler = handler;
        }

        /**
         * Evaluate whether to silently absorb uncaught crashes such that they
         * don't crash the app. We generally want to avoid this practice - we would
         * rather know about them. However in some cases there's nothing we can do
         * about the crash (e.g. it is an OS fault) and we would rather not have them
         * pollute our reliability stats.
         */
        private boolean shouldAbsorb(Throwable exception) {
            return CANNOT_DELIVER_BROADCAST_EXCEPTION.equals(exception.getClass().getSimpleName());
        }

        @Override
        public void uncaughtException(@NonNull Thread t, @NonNull Throwable e) {
            if (shouldAbsorb(e)) {
                Log.e(TAG, "Absorbing exception", e);
            } else {
                handler.uncaughtException(t, e);
            }
        }
    }
}
