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
package com.amazon.sampleapp.core;

import android.app.Activity;

import androidx.viewpager.widget.ViewPager;

import com.amazon.aace.alexa.SpeechRecognizer;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.propertyManager.PropertyManager;

import org.json.JSONObject;

public interface SampleAppContext {
    String CERTS_DIR = "certsDir";
    String MODEL_DIR = "modelsDir";
    String PRODUCT_DSN = "productDsn";
    String APPDATA_DIR = "appDataDir";
    String JSON = "json";

    Activity getActivity();

    AudioOutputProvider getAudioOutputProvider();

    LoggerControllerInterface getLoggerController();

    SpeechRecognizer getSpeechRecognizer();

    ViewPager getViewPager();

    String getData(String key);

    JSONObject getConfigFromFile(String configAssetName, String configRootKey);

    PropertyManager getPropertyManager();

    void registerPropertyListener(String name, PropertyListener propertyListener);

    void registerAuthStateObserver(AuthStateObserver observer);

    void addEngineStatusListener(EngineStatusListener listener);

    void addPlatformInterfaceHandler(String name, PlatformInterface handler);

    public PlatformInterface getPlatformInterfaceHandler(String name);
}
