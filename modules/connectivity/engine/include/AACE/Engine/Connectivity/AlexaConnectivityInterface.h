/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_INTERFACE_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_INTERFACE_H

#include <string>
#include <vector>

#include <AVSCommon/AVS/AlexaResponseType.h>
#include <AVSCommon/SDKInterfaces/AlexaStateChangeCauseType.h>
#include <AVSCommon/Utils/Optional.h>

#include "AACE/Engine/Connectivity/AlexaConnectivityListenerInterface.h"

namespace aace {
namespace engine {
namespace connectivity {

class AlexaConnectivityInterface {
public:
    /// Alias to improve readability.
    using DataPlan = AlexaConnectivityListenerInterface::DataPlan;
    using DataPlansAvailable = AlexaConnectivityListenerInterface::DataPlansAvailable;
    using DataPlansAvailableState = AlexaConnectivityListenerInterface::DataPlansAvailableState;
    using DataPlanState = AlexaConnectivityListenerInterface::DataPlanState;
    using DataPlanType = AlexaConnectivityListenerInterface::DataPlanType;
    using ManagedProvider = AlexaConnectivityListenerInterface::ManagedProvider;
    using ManagedProviderState = AlexaConnectivityListenerInterface::ManagedProviderState;
    using ManagedProviderType = AlexaConnectivityListenerInterface::ManagedProviderType;
    using TermsStatus = AlexaConnectivityListenerInterface::TermsStatus;
    using TermsStatusState = AlexaConnectivityListenerInterface::TermsStatusState;

    /**
     * A unique identifier provided by the network provider to identify the device connected to the Internet.
     */
    using NetworkIdentifier = std::string;

    /**
     * Virtual destructor to assure proper cleanup of derived types.
     */
    virtual ~AlexaConnectivityInterface() = default;

    /**
     * Get the data plan.
     *
     * @return The DataPlan object.
     */
    virtual DataPlan getDataPlan() const = 0;

    /**
     * Get the data plans available.
     *
     * @return The DataPlansAvailable array.
     */
    virtual DataPlansAvailable getDataPlansAvailable() const = 0;

    /**
     * Get the managed provider.
     *
     * @return The ManagedProvider object.
     */
    virtual ManagedProvider getManagedProvider() const = 0;

    /**
     * Get the network identifier.
     *
     * @return The NetworkIdentifier string.
     */
    virtual NetworkIdentifier getNetworkIdentifier() const = 0;

    /**
     * Get the terms status.
     *
     * @return The TermsStatus enum.
     */
    virtual TermsStatus getTermsStatus() const = 0;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_INTERFACE_H
