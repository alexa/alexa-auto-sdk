package com.amazon.sampleapp.core;

import android.support.v4.app.Fragment;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;

import java.util.List;

public interface ModuleFactoryInterface {
    List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext);

    List<Fragment> getFragments(SampleAppContext sampleAppContext);

    List<Integer> getLayoutResourceNums();

    List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext);
}
