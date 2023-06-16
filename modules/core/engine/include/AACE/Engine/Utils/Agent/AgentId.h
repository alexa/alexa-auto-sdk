/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_UTILS_AGENT_AGENTID_H
#define AACE_ENGINE_UTILS_AGENT_AGENTID_H

namespace aace {
namespace engine {
namespace utils {
namespace agent {

typedef unsigned int AgentIdType;

/// A predefined value to express that no agent is associated with a given device-side event.
static constexpr AgentIdType AGENT_ID_NONE = 0;

/// A predefined value to express that a given device-side event is relevant to all available agents.
static constexpr AgentIdType AGENT_ID_ALL = 1;

/// A predefined value to express that a given device-side event is relevant to Alexa agent.
static constexpr AgentIdType AGENT_ID_ALEXA = 2;

}  // namespace agent
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_AGENT_AGENTID_H
