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

package com.amazon.sampleapp.bluetooth;

import android.util.Log;

import androidx.fragment.app.Fragment;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import java.util.ArrayList;
import java.util.List;

public class BluetoothModuleFactory implements ModuleFactoryInterface {
    private static final String TAG = BluetoothModuleFactory.class.getSimpleName();

    public List<Fragment> getFragments(SampleAppContext sampleAppContext) {
        return new ArrayList<>();
    }

    public List<Integer> getLayoutResourceNums() {
        return new ArrayList<>();
    }

    public List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext) {
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        platformInterfaceList.add(new BluetoothProviderHandler(sampleAppContext.getActivity()));
        Log.i(TAG, "BluetoothProvider handler registered.");
        return platformInterfaceList;
    }

    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        return new ArrayList<>();
    }
}
