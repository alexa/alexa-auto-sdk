/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.PropertyManager;

import android.app.Activity;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;

import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aace.logger.Logger;
import com.amazon.aace.propertyManager.PropertyManager;
import com.amazon.sampleapp.MainActivity;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class PropertyManagerHandler extends PropertyManager {
    private final LoggerHandler mLogger;
    private final MainActivity mActivity;

    private static String TAG = "PropertyManager";

    public PropertyManagerHandler(final MainActivity activity, final LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
    }

    @Override
    public void propertyStateChanged(String name, String value, PropertyState state) {
        mLogger.postInfo(TAG, "name : " + name + ", value : " + value + ", state : " + state.name());
    }

    @Override
    public void propertyChanged(String key, String newValue) {
        mLogger.postInfo(TAG, "key : " + key + " new value : " + newValue);
        if (key.equals(AlexaProperties.TIMEZONE)) {
            mActivity.updateTimezoneSpinner(newValue);
        }
    }
}