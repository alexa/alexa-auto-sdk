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

import com.amazon.alexa.auto.settings.AACSLocaleReceiver;
import com.amazon.alexa.auto.settings.AACSPreferenceFragment;
import com.amazon.alexa.auto.settings.SettingsActivityViewModel;
import com.amazon.alexa.auto.settings.SettingsAlexaLanguageFragment;
import com.amazon.alexa.auto.settings.SettingsLanguageFragment;
import com.amazon.alexa.auto.settings.ThingsToTryDetailsFragment;
import com.amazon.alexa.auto.settings.home.AlexaLandingPageFragment;
import com.amazon.alexa.auto.settings.home.AlexaSettingsHomeFragment;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Alexa Settings Dependencies.
 */
@Component(modules = {AndroidModule.class, AACSModule.class, ConfigModule.class, MenuModule.class})
@Singleton
public interface SettingsComponent {
    /**
     * Inject to @{link SettingsActivityViewModel}.
     *
     * @param viewModel Object whose dependencies are resolved by dagger.
     */
    void inject(SettingsActivityViewModel viewModel);

    /**
     * Inject to @{link AlexaSettingsHomeFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AlexaSettingsHomeFragment fragment);

    /**
     * Inject to @{link AACSPreferenceFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AACSPreferenceFragment fragment);

    void inject(AACSLocaleReceiver aacsLocaleReceiver);

    /**
     * Inject to @{link ThingsToTryDetailsFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(ThingsToTryDetailsFragment fragment);

    void inject(AlexaLandingPageFragment fragment);

    void inject(SettingsLanguageFragment fragment);

    void inject(SettingsAlexaLanguageFragment fragment);
}
