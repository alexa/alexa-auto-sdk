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
package com.amazon.alexa.auto.settings.home;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.preference.PreferenceScreen;

/**
 * A helper interface to assist {@link AlexaSettingsHomeFragment} with preferences
 * in a specific area.
 * Alexa settings are dynamic and the items in the settings list can change based on
 * the mode in which app is running. As an example, some settings are shown and processed
 * only when app is running with debug build. Such concerns can be entirely encapsulated
 * with help of this interface.
 * Please note, that role of the concrete implementations of this interface is limited
 * only to main Alexa menu and it doesn't apply on any secondary settings screen navigated
 * from Alexa menu.
 */
public interface AlexaSettingsScreenBuilder {
    /**
     * Add or remove settings from Alexa menu preference screen.
     *
     * @param screen Preference screen that has list of settings.
     */
    void addRemovePreferences(@NonNull PreferenceScreen screen);

    /**
     * Install event handlers for settings that made it to the list
     * after {@link AlexaSettingsScreenBuilder#addRemovePreferences}
     * has been invoked on all {@link AlexaSettingsScreenBuilder}s.
     *
     * @param screen Preference screen that has list of settings.
     * @param view View where preference screen resides.
     */
    void installEventHandlers(@NonNull PreferenceScreen screen, @NonNull View view);

    /**
     * Perform cleanup if required.
     */
    void dispose();
}
