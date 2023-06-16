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

#include <memory>

#include "AACE/Engine/Utils/Timing/Timer.h"

namespace aace {
namespace engine {
namespace utils {
namespace timing {

/// String to identify log entries originating from this file.
static const std::string TAG(Timer::getTag());

Timer::Timer() {
    m_timer = std::unique_ptr<TimerDelegateInterface>(new TimerDelegate());
}

Timer::~Timer() {
    stop();
}

void Timer::stop() {
    if (m_timer) {
        m_timer->stop();
    }
}

bool Timer::isActive() const {
    return m_timer && m_timer->isActive();
}

bool Timer::activate() {
    return m_timer && m_timer->activate();
}

}  // namespace timing
}  // namespace utils
}  // namespace engine
}  // namespace aace
