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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_LISTENER_INTERFACE_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_LISTENER_INTERFACE_H

#include <chrono>
#include <utility>

#include <AVSCommon/SDKInterfaces/AlexaStateChangeCauseType.h>
#include <AVSCommon/Utils/Timing/TimePoint.h>

namespace aace {
namespace engine {
namespace connectivity {

/**
 * This interface is used to observe changes to the data plan and terms status
 * properties of an endpoint that are caused by the @c AlexaConnectivityInterface.
 */
class AlexaConnectivityListenerInterface {
public:
    /**
     * An enum representing the data plan type.
     */
    enum class DataPlanType {
        /// Indicates the device is utilizing an amazon sponsored data plan.
        AMAZON_SPONSORED,

        /// Indicates the device is utilizing a paid data plan.
        PAID,

        /// Indicates the device is utilizing a free trial data plan.
        TRIAL,

        /// @internal
        UNKNOWN
    };

    /**
     * The property describes the details of the device's internet plan.
     */
    struct DataPlan {
        /**
         * Constructor. Initializes the property to default.
         */
        DataPlan() : type{DataPlanType::UNKNOWN}, endDate{""} {};

        /**
         * Constructor for initializing with specified values.
         *
         * @param type
         * @param endDate
         */
        DataPlan(const DataPlanType& type, const std::string& endDate = "") : type{type}, endDate{endDate} {};

        /// The DataPlanType enum.
        DataPlanType type;

        /// The RFC 3339 format date on which the current data plan ends.
        std::string endDate;
    };

    /**
     * Struct that represents the data plan state of the endpoint.
     */
    struct DataPlanState {
        /**
         * Constructor for initializing with specified values.
         *
         * @param dataPlan
         * @param timeOfSample
         * @param uncertaintyInMilliseconds
         */
        DataPlanState(
            const DataPlan& dataPlan,
            const alexaClientSDK::avsCommon::utils::timing::TimePoint& timeOfSample =
                alexaClientSDK::avsCommon::utils::timing::TimePoint::now(),
            const std::chrono::milliseconds uncertaintyInMilliseconds = std::chrono::milliseconds(0)) :
                dataPlan{dataPlan}, timeOfSample{timeOfSample}, uncertaintyInMilliseconds{uncertaintyInMilliseconds} {};

        /// The DataPlan object.
        DataPlan dataPlan;

        /// Represents time at which the state value was recorded.
        alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample;

        /// The number of milliseconds that have elapsed since the state value was last confrimed.
        std::chrono::milliseconds uncertaintyInMilliseconds;
    };

    /**
     * Array that indicates the data plans that can be activated on a device.
     */
    using DataPlansAvailable = std::vector<std::string>;

    /**
     * Struct that represents the data plan state of the endpoint.
     */
    struct DataPlansAvailableState {
        /**
         * Constructor for initializing with specified values.
         *
         * @param dataPlansAvailable
         * @param timeOfSample
         * @param uncertaintyInMilliseconds
         */
        DataPlansAvailableState(
            const DataPlansAvailable& dataPlansAvailable,
            const alexaClientSDK::avsCommon::utils::timing::TimePoint& timeOfSample =
                alexaClientSDK::avsCommon::utils::timing::TimePoint::now(),
            const std::chrono::milliseconds uncertaintyInMilliseconds = std::chrono::milliseconds(0)) :
                dataPlansAvailable{dataPlansAvailable},
                timeOfSample{timeOfSample},
                uncertaintyInMilliseconds{uncertaintyInMilliseconds} {};

        /// The DataPlansAvailable array.
        DataPlansAvailable dataPlansAvailable;

        /// Represents time at which the state value was recorded.
        alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample;

        /// The number of milliseconds that have elapsed since the state value was last confrimed.
        std::chrono::milliseconds uncertaintyInMilliseconds;
    };

    /**
     * An enum representing the managed provider type.
     * Indicates if the device has managed internet connectivity.
     */
    enum class ManagedProviderType {
        /// The device has internet connectivity managed by a provider.
        MANAGED,

        /// The device has internet connectivity from a source that is not managed by a provider.
        NOT_MANAGED,

        /// @internal
        UNKNOWN
    };

    /**
     * The property describes the details of the device's internet connectivity.
     */
    struct ManagedProvider {
        /**
         * Constructor. Initializes the property to default.
         */
        ManagedProvider() : type{ManagedProviderType::UNKNOWN}, id{""} {};

        /**
         * Constructor for initializing with specified values.
         *
         * @param type
         * @param id
         */
        ManagedProvider(ManagedProviderType type, std::string id = "") : type{type}, id{id} {};

        /// The ManagedProviderType enum.
        ManagedProviderType type;

        /// The ManagedProvider identifier string.
        std::string id;
    };

    /**
     * Struct that represents the managed provider state of the endpoint.
     */
    struct ManagedProviderState {
        /**
         * Constructor for initializing with specified values.
         *
         * @param managedProvider
         * @param timeOfSample
         * @param uncertaintyInMilliseconds
         */
        ManagedProviderState(
            const ManagedProvider& managedProvider,
            const alexaClientSDK::avsCommon::utils::timing::TimePoint& timeOfSample =
                alexaClientSDK::avsCommon::utils::timing::TimePoint::now(),
            const std::chrono::milliseconds uncertaintyInMilliseconds = std::chrono::milliseconds(0)) :
                managedProvider{managedProvider},
                timeOfSample{timeOfSample},
                uncertaintyInMilliseconds{uncertaintyInMilliseconds} {};

        /// The ManagedProvider object.
        ManagedProvider managedProvider;

        /// Represents time at which the state value was recorded.
        alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample;

        /// The number of milliseconds that have elapsed since the state value was last confrimed.
        std::chrono::milliseconds uncertaintyInMilliseconds;
    };

    /**
     * An enum representing the terms status.
     */
    enum class TermsStatus {
        /// Indicates the customer has accepted the terms and conditions of the OEM and network provider.
        ACCEPTED,

        /// Indicates the customer has declined the terms and conditions of the OEM and network provider.
        DECLINED,

        /// @internal
        UNKNOWN
    };

    /**
     * Struct that represents the terms status state of the endpoint.
     */
    struct TermsStatusState {
        /**
         * Constructor for initializing with specified values.
         *
         * @param termsStatus 
         * @param timeOfSample
         * @param uncertaintyInMilliseconds
         */
        TermsStatusState(
            const TermsStatus& termsStatus,
            const alexaClientSDK::avsCommon::utils::timing::TimePoint& timeOfSample =
                alexaClientSDK::avsCommon::utils::timing::TimePoint::now(),
            const std::chrono::milliseconds uncertaintyInMilliseconds = std::chrono::milliseconds(0)) :
                termsStatus{termsStatus},
                timeOfSample{timeOfSample},
                uncertaintyInMilliseconds{uncertaintyInMilliseconds} {};

        /// The TermsStatus enum.
        TermsStatus termsStatus;

        /// Represents time at which the state value was recorded.
        alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample;

        /// The number of milliseconds that have elapsed since the state value was last confrimed.
        std::chrono::milliseconds uncertaintyInMilliseconds;
    };

    /**
     * Destructor.
     */
    virtual ~AlexaConnectivityListenerInterface() = default;

    /**
     * Notifies the change in properties of the endpoint.
     *
     * @param dataPlanState The changed DataPlanState object.
     * @param cause The cause for this change specified using @c AlexaStateChangeCauseType.
     */
    virtual void onDataPlanStateChanged(
        const DataPlanState& dataPlanState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) = 0;

    /**
     * Notifies the change in properties of the endpoint.
     *
     * @param dataPlansAvailableState The changed DataPlansAvailableState object.
     * @param cause The cause for this change specified using @c AlexaStateChangeCauseType.
     */
    virtual void onDataPlansAvailableStateChanged(
        const DataPlansAvailableState& dataPlansAvailableState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) = 0;

    /**
     * Notifies the change in properties of the endpoint.
     *
     * @param managedProviderState The changed ManagedProviderState object.
     * @param cause The cause for this change specified using @c AlexaStateChangeCauseType.
     */
    virtual void onManagedProviderStateChanged(
        const ManagedProviderState& managedProviderState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) = 0;

    /**
     * Notifies the change in properties of the endpoint.
     *
     * @param termsStatusState The changed TermsStatusState object.
     * @param cause The cause for this change specified using @c AlexaStateChangeCauseType.
     */
    virtual void onTermsStatusStateChanged(
        const TermsStatusState& termsStatusState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) = 0;
};

/**
 * Converts a string to an enum.
 *
 * @param dataPlanType The string to convert to a DataPlanType enum.
 * @return std::pair with @c true if successful, @c false otherwise, and the enum value.
 */
inline std::pair<bool, AlexaConnectivityListenerInterface::DataPlanType> dataPlanTypeFromString(
    const std::string& dataPlanType) {
    // clang-format off
    static const std::map<std::string, AlexaConnectivityListenerInterface::DataPlanType> DataPlanTypeEnumerator{
        {"AMAZON_SPONSORED", AlexaConnectivityListenerInterface::DataPlanType::AMAZON_SPONSORED},
        {"PAID", AlexaConnectivityListenerInterface::DataPlanType::PAID},
        {"TRIAL", AlexaConnectivityListenerInterface::DataPlanType::TRIAL}
        // UNKNOWN is special and must not be added here.
    };
    // clang-format on
    if (DataPlanTypeEnumerator.count(dataPlanType) == 0) {
        return std::make_pair(false, AlexaConnectivityListenerInterface::DataPlanType::UNKNOWN);
    }
    return std::make_pair(true, DataPlanTypeEnumerator.at(dataPlanType));
}

/**
 * Converts an enum to a string.
 *
 * @param dataPlanType The DataPlanType enum to convert to a string.
 * @return The string form of the enum.
 */
inline std::string dataPlanTypeToString(AlexaConnectivityListenerInterface::DataPlanType dataPlanType) {
    switch (dataPlanType) {
        case AlexaConnectivityListenerInterface::DataPlanType::AMAZON_SPONSORED:
            return "AMAZON_SPONSORED";
        case AlexaConnectivityListenerInterface::DataPlanType::PAID:
            return "PAID";
        case AlexaConnectivityListenerInterface::DataPlanType::TRIAL:
            return "TRIAL";
        case AlexaConnectivityListenerInterface::DataPlanType::UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}

/**
 * Converts a string to an enum.
 *
 * @param managedProviderType The string to convert to a ManagedProviderType enum.
 * @return std::pair with @c true if successful, @c false otherwise, and the enum value.
 */
inline std::pair<bool, AlexaConnectivityListenerInterface::ManagedProviderType> managedProviderTypeFromString(
    const std::string& managedProviderType) {
    // clang-format off
    static const std::map<std::string, AlexaConnectivityListenerInterface::ManagedProviderType> ManagedProviderTypeEnumerator{
        {"MANAGED", AlexaConnectivityListenerInterface::ManagedProviderType::MANAGED},
        {"NOT_MANAGED", AlexaConnectivityListenerInterface::ManagedProviderType::NOT_MANAGED}
        // UNKNOWN is special and must not be added here.
    };
    // clang-format on
    if (ManagedProviderTypeEnumerator.count(managedProviderType) == 0) {
        return std::make_pair(false, AlexaConnectivityListenerInterface::ManagedProviderType::UNKNOWN);
    }
    return std::make_pair(true, ManagedProviderTypeEnumerator.at(managedProviderType));
}

/**
 * Converts an enum to a string.
 *
 * @param managedProviderType The ManagedProviderType enum to convert to a string.
 * @return The string form of the enum.
 */
inline std::string managedProviderTypeToString(
    AlexaConnectivityListenerInterface::ManagedProviderType managedProviderType) {
    switch (managedProviderType) {
        case AlexaConnectivityListenerInterface::ManagedProviderType::MANAGED:
            return "MANAGED";
        case AlexaConnectivityListenerInterface::ManagedProviderType::NOT_MANAGED:
            return "NOT_MANAGED";
        case AlexaConnectivityListenerInterface::ManagedProviderType::UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}

/**
 * Converts a string to an enum.
 *
 * @param termsStatus The string to convert to a TermsStatus enum.
 * @return std::pair with @c true if successful, @c false otherwise, and the enum value.
 */
inline std::pair<bool, AlexaConnectivityListenerInterface::TermsStatus> termsStatusFromString(
    const std::string& termsStatus) {
    // clang-format off
    static const std::map<std::string, AlexaConnectivityListenerInterface::TermsStatus> TermsStatusEnumerator{
        {"ACCEPTED", AlexaConnectivityListenerInterface::TermsStatus::ACCEPTED},
        {"DECLINED", AlexaConnectivityListenerInterface::TermsStatus::DECLINED}
        // UNKNOWN is special and must not be added here.
    };
    // clang-format on
    if (TermsStatusEnumerator.count(termsStatus) == 0) {
        return std::make_pair(false, AlexaConnectivityListenerInterface::TermsStatus::UNKNOWN);
    }
    return std::make_pair(true, TermsStatusEnumerator.at(termsStatus));
}

/**
 * Converts an enum to a string.
 *
 * @param termsStatus The TermsStatus enum to convert to a string.
 * @return The string form of the enum.
 */
inline std::string termsStatusToString(AlexaConnectivityListenerInterface::TermsStatus termsStatus) {
    switch (termsStatus) {
        case AlexaConnectivityListenerInterface::TermsStatus::ACCEPTED:
            return "ACCEPTED";
        case AlexaConnectivityListenerInterface::TermsStatus::DECLINED:
            return "DECLINED";
        case AlexaConnectivityListenerInterface::TermsStatus::UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}

/**
 * Operator == for @c AlexaConnectivityListenerInterface::DataPlan.
 * @param lhs The left hand side of the == operation.
 * @param rhs The right hand side of the == operation.
 * @return Whether or not this instance and @c rhs are equivalent.
 */
inline bool operator==(
    const AlexaConnectivityListenerInterface::DataPlan& lhs,
    const AlexaConnectivityListenerInterface::DataPlan& rhs) {
    return ((lhs.type == rhs.type) && (lhs.endDate == rhs.endDate));
}

/**
 * Operator != for @c AlexaConnectivityListenerInterface::DataPlan.
 * @param lhs The left hand side of the != operation.
 * @param rhs The right hand side of the != operation.
 * @return Whether or not this instance and @c rhs are not equivalent.
 */
inline bool operator!=(
    const AlexaConnectivityListenerInterface::DataPlan& lhs,
    const AlexaConnectivityListenerInterface::DataPlan& rhs) {
    return !(lhs == rhs);
}

/**
 * Operator == for @c AlexaConnectivityListenerInterface::ManagedProvider.
 * @param lhs The left hand side of the == operation.
 * @param rhs The right hand side of the == operation.
 * @return Whether or not this instance and @c rhs are equivalent.
 */
inline bool operator==(
    const AlexaConnectivityListenerInterface::ManagedProvider& lhs,
    const AlexaConnectivityListenerInterface::ManagedProvider& rhs) {
    return ((lhs.type == rhs.type) && (lhs.id == rhs.id));
}

/**
 * Operator != for @c AlexaConnectivityListenerInterface::ManagedProvider.
 * @param lhs The left hand side of the != operation.
 * @param rhs The right hand side of the != operation.
 * @return Whether or not this instance and @c rhs are not equivalent.
 */
inline bool operator!=(
    const AlexaConnectivityListenerInterface::ManagedProvider& lhs,
    const AlexaConnectivityListenerInterface::ManagedProvider& rhs) {
    return !(lhs == rhs);
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_LISTENER_INTERFACE_H
