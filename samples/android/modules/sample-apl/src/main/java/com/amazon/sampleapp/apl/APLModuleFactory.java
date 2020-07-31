/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.support.v4.app.Fragment;
import android.support.v4.view.ViewPager;

import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.sampleapp.apl.APLFragment;
import com.amazon.sampleapp.apl.R;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import java.util.ArrayList;
import java.util.List;

public class APLModuleFactory implements ModuleFactoryInterface {
    @Override
    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        List<EngineConfiguration> list = new ArrayList<>();
        return list;
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
        APLHandler aplHandler = new APLHandler(sampleAppContext);
        List<PlatformInterface> list = new ArrayList<>();
        list.add(aplHandler);
        return list;
    }
}
