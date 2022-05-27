/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_FEATURE_DISCOVERY_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_FEATURE_DISCOVERY_ENGINE_IMPL_H

#include <unordered_map>
#include <unordered_set>

#include "AVSCommon/Utils/RequiresShutdown.h"
#include "AACE/Alexa/FeatureDiscovery.h"
#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"
#include "AACE/Engine/Alexa/FeatureDiscoveryRESTAgent.h"
#include "AACE/Engine/Utils/Threading/Executor.h"

namespace aace {
namespace engine {
namespace alexa {

class FeatureDiscoveryEngineImpl
        : public aace::alexa::FeatureDiscoveryEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<FeatureDiscoveryEngineImpl> {
private:
    FeatureDiscoveryEngineImpl(std::shared_ptr<aace::alexa::FeatureDiscovery> platfromInterface);

public:
    static std::shared_ptr<FeatureDiscoveryEngineImpl> create(
        std::shared_ptr<aace::alexa::FeatureDiscovery> platfromInterface,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext);

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::FeatureDiscovery> m_platformInterface;
    bool initialize(std::shared_ptr<aace::engine::core::EngineContext> engineContext);
    void executeOnGetFeatures(const std::string& requestId, const std::string& discoveryRequests);

public:
    bool onGetFeatures(const std::string& requestId, const std::string& discoveryRequests) override;

private:
    std::weak_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> m_propertyManager;
    std::shared_ptr<aace::alexa::FeatureDiscovery> m_featureDiscoveryPlatformInterface;
    std::shared_ptr<aace::engine::alexa::FeatureDiscoveryRESTAgent> m_featureDiscoveryRESTAgent;
    std::string m_tag;
    std::unordered_set<std::string> m_validCombinations;
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_FEATURE_DISCOVERY_ENGINE_IMPL_H
