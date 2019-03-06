/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.modules;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.InputStream;
import java.util.Properties;

/**
 * This class loads the modules.props generated at build time and answers whether a given
 * extra modules is enabled or disabled.
 */
public class ExtraModules {
    private static String sTag = "ExtraModules";

    private boolean mIsAlexaCommsEnabled = false;
    private boolean mIsLocalVoiceControlEnabled = false;
    private boolean mIsAmazonLiteEnabled = false;

    public ExtraModules(Context context) {
        AssetManager assetManager = context.getAssets();
        String name = "modules.props";
        try {
            InputStream stream = assetManager.open(name);
            Properties prop = new Properties();
            prop.load(stream);

            final String enabled = "true";
            mIsAlexaCommsEnabled = enabled.equals(prop.getProperty("communications"));
            mIsLocalVoiceControlEnabled = enabled.equals(prop.getProperty("localvoicecontrol"));
            mIsAmazonLiteEnabled = enabled.equals(prop.getProperty("amazonlite"));

            StringBuilder summary = new StringBuilder();
            summary.append("Extra Module Status***\n")
                    .append("Alexa Comms :").append(mIsAlexaCommsEnabled).append("\n")
                    .append("Local Voice Control:").append(mIsLocalVoiceControlEnabled).append("\n")
                    .append("AmazonLite:").append(mIsAmazonLiteEnabled).append("\n")
                    .append("****");
            android.util.Log.i(sTag, summary.toString());
        } catch (Exception exp) {
            android.util.Log.e(sTag, "Failed to load properties from modules.props " + exp);
        }
    }

    public boolean isAlexaCommsEnabled() {
        return mIsAlexaCommsEnabled;
    }

    public boolean isLocalVoiceControlEnabled() {
        return mIsLocalVoiceControlEnabled;
    }

    public boolean isAmazonLiteEnabled() {
        return mIsAmazonLiteEnabled;
    }
}