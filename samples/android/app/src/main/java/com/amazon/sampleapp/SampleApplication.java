/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.sampleapp;

import android.app.Application;
import android.content.Intent;

import com.google.android.gms.security.ProviderInstaller;

/**
 * Application class
 */
public class SampleApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        /**
         * The below code snippet is added due to a platform bug present in Android Pie that prevents
         * loading of http urls from loading over https protocol. The platform bug is that the security
         * provider does not get automatically sometimes and this method call manually updates the
         * security provider if required. More info here: https://github.com/google/ExoPlayer/issues/5103.
         */
        ProviderInstaller.installIfNeededAsync(this, new ProviderInstaller.ProviderInstallListener() {
            @Override
            public void onProviderInstalled() {

            }

            @Override
            public void onProviderInstallFailed(int i, Intent intent) {

            }
        });

    }
}
