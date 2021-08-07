/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CONSTANTS_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CONSTANTS_H

#include <string>

/** @file */

namespace aace {
namespace engine {
namespace connectivity {

/// The configuration key.
static const std::string CONFIGURATION_KEY{"configurations"};

/// The data plan key.
static const std::string DATAPLAN_KEY{"dataPlan"};

/// The data plans available key.
static const std::string DATAPLANSAVAILABLE_KEY{"dataPlansAvailable"};

/// The end date key.
static const std::string ENDDATE_KEY{"endDate"};

/// The id key.
static const std::string ID_KEY{"id"};

/// The managed provider key.
static const std::string MANAGEDPROVIDER_KEY{"managedProvider"};

/// The network identifier key.
static const std::string NETWORKIDENTIFIER_KEY{"networkIdentifier"};

/// The terms key.
static const std::string TERMS_KEY{"terms"};

/// The terms status key for capability agent.
static const std::string STATUS_KEY{"status"};

/// The terms version key for capability agent.
static const std::string VERSION_KEY{"version"};

/// The terms status key for JSON parsing.
static const std::string TERMSSTATUS_KEY{"termsStatus"};

/// The terms version key for JSON parsing.
static const std::string TERMSVERSION_KEY{"termsVersion"};

/// The type key.
static const std::string TYPE_KEY{"type"};

/// The value key.
static const std::string VALUE_KEY{"value"};

/// UNKNOWN_VERSION terms version.
static const std::string UNKNOWN_VERSION{"UNKNOWN"};

/// Max length restriction for terms version.
static const int VERSION_SIZE_MAX_LIMIT{250};

/// The ACTIVATE_TRIAL event key.
static const std::string ACTIVATE_TRIAL_KEY{"ACTIVATE_TRIAL"};

/// The ACTIVATE_PAID_PLAN event key.
static const std::string ACTIVATE_PAID_PLAN_KEY{"ACTIVATE_PAID_PLAN"};

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CONSTANTS_H