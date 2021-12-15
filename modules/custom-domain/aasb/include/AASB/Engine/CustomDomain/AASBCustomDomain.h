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

#ifndef AASB_ENGINE_CUSTOMDOMAIN_AASB_CUSTOMDOMAIN_H
#define AASB_ENGINE_CUSTOMDOMAIN_AASB_CUSTOMDOMAIN_H

#include <AACE/CustomDomain/CustomDomain.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace customDomain {

/**
 * Provides a AASB CustomDomain handler.
 */
class AASBCustomDomain
        : public aace::customDomain::CustomDomain
        , public std::enable_shared_from_this<AASBCustomDomain> {
private:
    AASBCustomDomain() = default;

    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBCustomDomain> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    // aace::customDomain::CustomDomain
    void handleDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& payload,
        const std::string& correlationToken,
        const std::string& messageId) override;
    void cancelDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& correlationToken,
        const std::string& messageId) override;
    std::string getContext(const std::string& contextNamespace) override;

private:
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
};

}  // namespace customDomain
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_CUSTOMDOMAIN_AASB_CUSTOMDOMAIN_H