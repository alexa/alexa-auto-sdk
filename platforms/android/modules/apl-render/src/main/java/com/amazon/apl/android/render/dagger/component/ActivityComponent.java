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

import com.amazon.apl.android.APLOptions;
import com.amazon.apl.android.RootConfig;
import com.amazon.apl.android.render.dagger.ActivityContext;
import com.amazon.apl.android.render.dagger.ActivityScope;
import com.amazon.apl.android.render.dagger.module.APLOptionsModule;
import com.amazon.apl.android.render.dagger.module.ActivityModule;

import dagger.Component;

/**
 * This provides all the activity level dependencies.
 */
@ActivityScope
@Component(modules = {ActivityModule.class, APLOptionsModule.class}, dependencies = ApplicationComponent.class)
public interface ActivityComponent {
    /**
     * @return the context.
     */
    @ActivityContext
    Context getContext();

    /**
     * @return The {@link APLOptions.Builder}.
     */
    APLOptions.Builder getAPLOptionsBuilder();

    /**
     * @return The {@link RootConfig}.
     */
    RootConfig getRootConfig();
}
