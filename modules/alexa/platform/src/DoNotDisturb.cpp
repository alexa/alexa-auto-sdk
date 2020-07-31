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

#include "AACE/Alexa/DoNotDisturb.h"

namespace aace {
namespace alexa {

DoNotDisturb::~DoNotDisturb() = default;  // key function

bool DoNotDisturb::doNotDisturbChanged(const bool doNotDisturb) {
    if (auto doNotDisturbEngineInterface_lock = m_doNotDisturbEngineInterface.lock()) {
        return doNotDisturbEngineInterface_lock->onDoNotDisturbChanged(doNotDisturb);
    }
    return false;
}

void DoNotDisturb::setEngineInterface(
    std::shared_ptr<aace::alexa::DoNotDisturbEngineInterface> doNotDisturbEngineInterface) {
    m_doNotDisturbEngineInterface = doNotDisturbEngineInterface;
}

}  // namespace alexa
}  // namespace aace
