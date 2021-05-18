/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.apl;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

import androidx.fragment.app.Fragment;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.ConfigurationFile;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class APLModuleFactory implements ModuleFactoryInterface {
    private static final String TAG = "APLModuleFactory";
    private static final String CONFIG_FILE_NAME = "/APLViewport.json";

    private APLHandler mAplHandler;
    private JSONArray mVisualConfig;
    private String mDefaultWindowId;

    @Override
    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        List<EngineConfiguration> aplConfiguration = new ArrayList<>();
        updateConfigurationFromFile(aplConfiguration, sampleAppContext);

        return aplConfiguration;
    }

    @Override
    public List<Fragment> getFragments(SampleAppContext sampleAppContext) {
        Activity activity = sampleAppContext.getActivity();
        APLFragment aplFragment = new APLFragment(activity);
        List<Fragment> list = new ArrayList<>();
        list.add(aplFragment);
        return list;
    }

    @Override
    public List<Integer> getLayoutResourceNums() {
        List<Integer> list = new ArrayList<>();
        return list;
    }

    @Override
    public List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext) {
        mAplHandler = new APLHandler(sampleAppContext);
        // Initialize APL presenter
        mAplHandler.buildAPLPresenter(mVisualConfig, mDefaultWindowId);
        List<PlatformInterface> list = new ArrayList<>();
        list.add(mAplHandler);
        return list;
    }

    private void updateConfigurationFromFile(
            List<EngineConfiguration> aplConfiguration, SampleAppContext sampleAppContext) {
        File aplVisualConfigFile = new File(Environment.getExternalStorageDirectory(), CONFIG_FILE_NAME);
        // Look in /sdcard first, otherwise get default config
        if (aplVisualConfigFile.exists()) {
            Log.i(TAG, "Using APL Visual Characteristics config file on SD card");
            EngineConfiguration aplVisualConfig = ConfigurationFile.create(aplVisualConfigFile.getPath());
            aplConfiguration.add(aplVisualConfig);
        } else {
            String configFilePath = "config" + CONFIG_FILE_NAME;
            AssetManager am = sampleAppContext.getActivity().getAssets();

            try {
                EngineConfiguration aplVisualConfig = StreamConfiguration.create(am.open(configFilePath));
                aplConfiguration.add(aplVisualConfig);
                // This needs to be reported after platform interface is registered
                setDeviceWindowState(am, configFilePath);
            } catch (IOException e) {
                Log.e(TAG, "updateConfigurationFromFile: ", e);
            }
        }
    }

    /**
     * Builds the initial device window state that will be reported after
     * platform interface registration.
     * {
     *                     "defaultWindowId": "string",
     *                     "instances" : [
     *                         {
     *                           "id": "string",
     *                           "templateId": "string",
     *                           "token" : "",
     *                           "configuration": {
     *                              "interactionMode": "string",
     *                              "sizeConfigurationId": "string"
     *                           }
     *                         }
     *                   ]
     * }
     *
     *
     * @param am The AssetManager instance.
     * @param path The path to the configuration file.
     */
    private void setDeviceWindowState(AssetManager am, String path) {
        try (InputStream is = am.open(path)) {
            byte[] buffer = new byte[is.available()];
            is.read(buffer);
            String json = new String(buffer, "UTF-8");
            JSONObject config = new JSONObject(json);

            mVisualConfig = config.getJSONObject("aace.alexa")
                                    .getJSONObject("avsDeviceSDK")
                                    .getJSONObject("gui")
                                    .getJSONArray("visualCharacteristics");

            // Find the first window id and set it as the default. In a production
            // application the default window id would be chosen more carefully.
            if (mVisualConfig.length() > 0) {
                for (int i = 0; i < mVisualConfig.length(); i++) {
                    JSONObject currentElement = mVisualConfig.getJSONObject(i);
                    if (currentElement.getString("interface").equals("Alexa.Display.Window")) {
                        JSONArray templates = currentElement.getJSONObject("configurations").getJSONArray("templates");
                        for (int j = 0; j < templates.length(); j++) {
                            JSONObject template = templates.getJSONObject(j);
                            mDefaultWindowId = template.getString("id");
                            break;
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.w(TAG,
                    String.format(
                            "Cannot read %s from assets directory. Error: %s", "APLViewport.json", e.getMessage()));
        }
    }
}
