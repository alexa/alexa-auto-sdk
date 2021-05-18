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

package com.amazon.alexaautoclientservice.bluetooth;
import android.util.Log;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.alexaautoclientservice.aacs_extra.AACSContext;
import com.amazon.alexaautoclientservice.aacs_extra.AACSModuleFactoryInterface;

import java.util.ArrayList;
import java.util.List;

public class BluetoothModuleFactory implements AACSModuleFactoryInterface {
    private static final String TAG = BluetoothModuleFactory.class.getSimpleName();

    public List<PlatformInterface> getModulePlatformInterfaces(AACSContext aacsContext) {
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        platformInterfaceList.add(new BluetoothProviderHandler(aacsContext.getContext()));
        Log.i(TAG, "Bluetooth provider handler registered.");
        return platformInterfaceList;
    }

    public List<EngineConfiguration> getConfiguration(AACSContext aacsContext) {
        // No engine configuration is needed for bluetooth
        return new ArrayList<>();
    }
}
