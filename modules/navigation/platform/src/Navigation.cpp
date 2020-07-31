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

#include "AACE/Navigation/Navigation.h"

namespace aace {
namespace navigation {

Navigation::~Navigation() = default;  // key function

void Navigation::navigationEvent(EventName event) {
    if (m_navigationEngineInterface != nullptr) {
        m_navigationEngineInterface->onNavigationEvent(event);
    }
}

void Navigation::navigationError(ErrorType type, ErrorCode code, const std::string& description) {
    if (m_navigationEngineInterface != nullptr) {
        m_navigationEngineInterface->onNavigationError(type, code, description);
    }
}

void Navigation::showAlternativeRoutesSucceeded(const std::string& payload) {
    if (m_navigationEngineInterface != nullptr) {
        m_navigationEngineInterface->onShowAlternativeRoutesSucceeded(payload);
    }
}

void Navigation::setEngineInterface(std::shared_ptr<NavigationEngineInterface> navigationEngineInterface) {
    m_navigationEngineInterface = navigationEngineInterface;
}

}  // namespace navigation
}  // namespace aace
