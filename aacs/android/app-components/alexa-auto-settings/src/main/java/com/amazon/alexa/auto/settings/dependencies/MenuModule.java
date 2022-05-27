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
package com.amazon.alexa.auto.settings.dependencies;

import com.amazon.alexa.auto.settings.home.AlexaSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.AuthSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.DebugSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.VoiceAssistanceSettingsScreenBuilder;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module for Menu classes.
 */
@Module
public class MenuModule {
    /**
     * Provides list of {@link AlexaSettingsScreenBuilder}.
     *
     */
    @Provides
    @Singleton
    public List<AlexaSettingsScreenBuilder> provideMenuScreenBuilders() {
        // The order is important, later the item, final a say it would have in determining
        // what menu items would be presented on Alexa Settings screen.
        List<AlexaSettingsScreenBuilder> builders = new ArrayList<>();
        builders.add(new AuthSettingsScreenBuilder());
        builders.add(new VoiceAssistanceSettingsScreenBuilder());
        builders.add(new DebugSettingsScreenBuilder());

        return builders;
    }
}
