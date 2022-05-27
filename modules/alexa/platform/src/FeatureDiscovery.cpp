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

#include "AACE/Alexa/FeatureDiscovery.h"

namespace aace {
namespace alexa {

FeatureDiscovery::~FeatureDiscovery() = default;

void FeatureDiscovery::setEngineInterface(
    std::shared_ptr<aace::alexa::FeatureDiscoveryEngineInterface> featureDiscoveryEngineInterface) {
    m_featureDiscoveryEngineInterface = featureDiscoveryEngineInterface;
}

void FeatureDiscovery::getFeatures(const std::string& requestId, const std::string& discoveryRequests) {
    if (auto featureDiscoveryEngineInterface = m_featureDiscoveryEngineInterface.lock()) {
        featureDiscoveryEngineInterface->onGetFeatures(requestId, discoveryRequests);
    }
}

}  // namespace alexa
}  // namespace aace
