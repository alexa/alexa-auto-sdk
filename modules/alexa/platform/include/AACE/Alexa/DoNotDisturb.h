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

#ifndef AACE_ALEXA_DO_NOT_DISTURB_H
#define AACE_ALEXA_DO_NOT_DISTURB_H

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * DoNotDisturb should be extended to handle receving state changes from the engine, 
 * and for sending state change events for the Do Not Disturb Setting.
 * For more information about Do Not Disturb see the interface overview: 
 * https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html
 */
class DoNotDisturb : public aace::core::PlatformInterface {
protected:
    DoNotDisturb() = default;

public:
    virtual ~DoNotDisturb();

    /**
     * Handle setting of DND directive. 
     * 
     * @param [in] doNotDisturb setting state
     */
    virtual void setDoNotDisturb(const bool doNotDisturb) = 0;

    /**
     * Notifies the Engine of a platform request to set the DND State
     * 
     * @param [in] doNotDisturb setting state
     * @return true if successful, false if change was rejected
     */
    bool doNotDisturbChanged(const bool doNotDisturb);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::alexa::DoNotDisturbEngineInterface> doNotDisturbEngineInterface);

private:
    std::weak_ptr<aace::alexa::DoNotDisturbEngineInterface> m_doNotDisturbEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_DO_NOT_DISTURB_H
