/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED,
 * OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 * A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

package com.amazon.sampleapp.connectivity;

import android.app.Activity;
import android.support.v4.app.Fragment;
import android.util.Log;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.sampleapp.connectivity.AlexaConnectivityHandler;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class ConnectivityModuleFactory implements ModuleFactoryInterface {
    private static final String TAG = "ConnectivityModuleFactory";

    @Override
    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        List<EngineConfiguration> list = new ArrayList<>();
        return list;
    }

    @Override
    public List<Fragment> getFragments(SampleAppContext sampleAppContext) {
        List<Fragment> list = new ArrayList<>();
        return list;
    }

    @Override
    public List<Integer> getLayoutResourceNums() {
        List<Integer> list = new ArrayList<>();
        list.add(R.layout.connectivity_view);
        return list;
    }

    @Override
    public List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext) {
        Activity activity = sampleAppContext.getActivity();
        AlexaConnectivityHandler connectivityHandler = new AlexaConnectivityHandler(sampleAppContext);
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        platformInterfaceList.add(connectivityHandler);

        return platformInterfaceList;
    }
}