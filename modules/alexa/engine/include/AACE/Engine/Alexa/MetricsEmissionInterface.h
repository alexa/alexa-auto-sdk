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

#ifndef AACE_ENGINE_ALEXA_METRICSEMISSIONINTERFACE_H_
#define AACE_ENGINE_ALEXA_METRICSEMISSIONINTERFACE_H_

#include <memory>
#include <string>

#include "MetricsEmissionListenerInterface.h"

namespace aace {
namespace engine {
namespace alexa {

/**
 * Interface used internally to register a listener to the metrics emission state changes.
 */
class MetricsEmissionInterface {
public:
    /**
     * Virtual destructor
     */
    virtual ~MetricsEmissionInterface() = default;

    /**
     * Add the listener object to observe the metric emission state change.
     *
     * @param listener The object to listen the metric emission state.
     */
    virtual void addListener(std::shared_ptr<MetricsEmissionListenerInterface> listener) = 0;

    /**
     * Remove the listener object.
     *
     * @param listener The listener to remove.
     */
    virtual void removeListener(std::shared_ptr<MetricsEmissionListenerInterface> listener) = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_METRICSEMISSIONINTERFACE_H_
