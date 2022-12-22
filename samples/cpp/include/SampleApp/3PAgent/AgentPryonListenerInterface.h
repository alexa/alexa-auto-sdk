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
#ifndef SAMPLEAPP_AGENT_PRYON_LISTENER_INTERFACE_H
#define SAMPLEAPP_AGENT_PRYON_LISTENER_INTERFACE_H

#include <string>
#include <chrono>

namespace sampleApp {
namespace arbitrator {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AgentPryonListenerInterface
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Interface to notify the events from PryonLite Handler.
 */
class AgentPryonListenerInterface {
public:
    virtual ~AgentPryonListenerInterface() = default;

    /**
     *  To send the event to the @c AgentHandler
     * 
     * @param startofSpeechTimeStamp Represents the start Of Speech Timestamp given to requestSIRI event.
     */
    virtual void setStartofSpeechTimeStamp(std::chrono::time_point<std::chrono::system_clock>& startofSpeechTimeStamp) = 0;
};

}  // namespace arbitrator
}  // namespace sampleApp

#endif  // SAMPLEAPP_AGENT_PRYON_LISTENER_INTERFACE_H
