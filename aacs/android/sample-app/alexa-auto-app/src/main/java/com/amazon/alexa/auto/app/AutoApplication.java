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

import java.util.Arrays;
import java.util.List;
import java.util.Optional;

import javax.inject.Inject;

/**
 * Application class which also provides component registry to constituent
 * components through implementation of {@link AlexaApp}
 */
public class AutoApplication extends Application implements AlexaApp {
    private static final String TAG = AutoApplication.class.getSimpleName();

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
}
