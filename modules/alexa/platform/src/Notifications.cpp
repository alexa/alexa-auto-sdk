/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Alexa/Notifications.h>

namespace aace {
namespace alexa {

Notifications::~Notifications() = default; // key function

#ifdef OBIGO_AIDAEMON
bool Notifications::StateDoNotDisturb() {
    if( auto m_notificationsEngineInterface_lock = m_notificationsEngineInterface.lock() ) {
        return m_notificationsEngineInterface_lock->onStateDoNotDisturb();
    }
    else {
        return false;
    }
}

void Notifications::setEngineInterface( std::shared_ptr<aace::alexa::NotificationsEngineInterface> notificationsEngineInterface ) {
    m_notificationsEngineInterface = notificationsEngineInterface;
}
#endif

} // aace::alexa
} // aac
