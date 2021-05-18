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

#ifndef AACE_ENGINE_ALEXA_METRICSEMISSIONLISTENERINTERFACE_H_
#define AACE_ENGINE_ALEXA_METRICSEMISSIONLISTENERINTERFACE_H_

namespace aace {
namespace engine {
namespace alexa {
/**
 * Interface used to observe enablement of metric emission.
 */
class MetricsEmissionListenerInterface {
public:
    /**
     * Virtual destructor
     */
    virtual ~MetricsEmissionListenerInterface() = default;

    /**
     * Notifies the listener about metrics emission state change.
     *
     * @param emit The new state of metric emission.
     */
    virtual void onMetricEmissionStateChanged(bool emit) = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_METRICSEMISSIONLISTENERINTERFACE_H_
