/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/CustomDomain/CustomDomain.h"

namespace aace {
namespace customDomain {

CustomDomain::~CustomDomain() = default;  // key function

void CustomDomain::sendEvent(
    const std::string& eventNamespace,
    const std::string& name,
    const std::string& payload,
    bool requiresContext,
    const std::string& correlationToken,
    const std::string& customContext) {
    if (m_customDomainEngineInterface != nullptr) {
        m_customDomainEngineInterface->onSendEvent(
            eventNamespace, name, payload, requiresContext, correlationToken, customContext);
    }
}

void CustomDomain::reportDirectiveHandlingResult(
    const std::string& directiveNamespace,
    const std::string& correlationToken,
    ResultType result) {
    if (m_customDomainEngineInterface != nullptr) {
        m_customDomainEngineInterface->onReportDirectiveHandlingResult(
            directiveNamespace, correlationToken, static_cast<ResultType>(result));
    }
}

void CustomDomain::setEngineInterface(std::shared_ptr<CustomDomainEngineInterface> customDomainEngineInterface) {
    m_customDomainEngineInterface = customDomainEngineInterface;
}

}  // namespace customDomain
}  // namespace aace
