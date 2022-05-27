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
package com.amazon.alexa.auto.navigation.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.handlers.NaviDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects to communicate with AACS.
 */
@Module
public class AACSModule {
    @Provides
    @Singleton
    public NaviDirectiveHandler provideNaviDirectiveHandler(NaviProvider naviProvider) {
        return new NaviDirectiveHandler(naviProvider);
    }

    @Provides
    @Singleton
    public LocalSearchDirectiveHandler providePOIDirectiveHandler(
            WeakReference<Context> context, NaviProvider naviProvider) {
        return new LocalSearchDirectiveHandler(context, naviProvider);
    }
}
