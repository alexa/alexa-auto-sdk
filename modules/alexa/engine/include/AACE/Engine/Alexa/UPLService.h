/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_METRICS_UPL_SERVICE_H
#define AACE_ENGINE_METRICS_UPL_SERVICE_H

#include "AACE/Engine/Metrics/MetricEvent.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace aace {
namespace engine {
namespace alexa {

class UPLService {
public:
    /**
     * Return the one and only @c UPLService instance.
     *
     * @return The one and only @c UPLService instance.
     */
    static std::shared_ptr<UPLService> getInstance();

    /**
     * Destructor.
     */
    virtual ~UPLService() = default;

    /**
     * An enum class to represent the state of the Dialog Request received from AVS.
     */
    enum class DialogState {
        NONE,
        START_CAPTURE,
        STOP_CAPTURE,
        FIRST_BYTE_JSON_RECEIVED,
        PLAYBACK_STARTED,
        PLAYBACK_FINISHED
    };

    /**
     * A struct to define how to calculate hash for the DialogState enum.
     */
    struct DialogStateHash {
        std::size_t operator()(DialogState e) const {
            return static_cast<std::size_t>(e);
        }
    };

    /**
     * Update UPL with the current state of the directive
     *
     * @param currentState The current state that AVS is in processing the directive.
     * @param dialogId The dialog id of the directive being processed
     */
    void updateDialogStateForId(const DialogState currentState, const std::string& dialogId, bool isOnline);

private:
    /**
     * Constructor.
     */
    UPLService();

    /**
     * Record the user perceived latency based on the STOP_CAPTURE and PLAYBACK_STARTED state information.
     */
    void recordUPL();

    /**
     * Manage the current state that AVS is in processing the directive. 
     *
     * @param currentState The current state that AVS is in processing the directive.
     * @param dialogId The dialog id of the directive being processed
     */
    void manageStates(const DialogState currentState, const std::string& dialogId);

    /**
     * Reset states by clearing previously saved information from m_stateToTimeMap and m_currentMetric.
     * 
     * @param dialogId The dialog id of the states to be cleared
     */
    void resetStatesForId(const std::string& dialogId);

    /**
     * Get current time in milliseconds.
     *
     * @return The time in milliseconds.
     */
    double getCurrentTimeInMs();

    /// Dialog id of current directive being processed by AVS.
    std::string m_dialogId;

    /// Current state that AVS is in processing the directive.
    DialogState m_currentState;

    /// Map of DialogState and the time it occurred in milliseconds. Used to calculate UPL.
    std::unordered_map<DialogState, double, DialogStateHash> m_stateToTimeMap;

    /// Boolean that defines if UPL measured is from local AHE or online AVS.
    bool m_isOnline;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_UPL_SERVICE_H
