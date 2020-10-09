package com.amazon.alexaautoclientservice.aacs_extra;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;

import java.util.List;

public interface AACSModuleFactoryInterface {
    List<EngineConfiguration> getConfiguration(AACSContext aacsContext);
    List<PlatformInterface> getModulePlatformInterfaces(AACSContext aacsContext);
}
