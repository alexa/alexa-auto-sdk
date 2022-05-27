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

#ifndef AACE_JNI_CUSTOMDOMAIN_CUSTOMDOMAIN_BINDER_H
#define AACE_JNI_CUSTOMDOMAIN_CUSTOMDOMAIN_BINDER_H

#include <AACE/CustomDomain/CustomDomain.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace customDomain {

//
// CustomDomainHandler
//

class CustomDomainHandler : public aace::customDomain::CustomDomain {
public:
    CustomDomainHandler(jobject obj);

    // aace::customDomain::CustomDomainHandler
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
    JObject m_obj;
};

//
// CustomDomainBinder
//

class CustomDomainBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    CustomDomainBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_customDomainHandler;
    }

    std::shared_ptr<CustomDomainHandler> getCustomDomain() {
        return m_customDomainHandler;
    }

private:
    std::shared_ptr<CustomDomainHandler> m_customDomainHandler;
};

//
// JResultType
//

class JResultTypeConfig : public EnumConfiguration<CustomDomainHandler::ResultType> {
public:
    using T = CustomDomainHandler::ResultType;

    const char* getClassName() override {
        return "com/amazon/aace/customDomain/CustomDomain$ResultType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNEXPECTED_INFORMATION_RECEIVED, "UNEXPECTED_INFORMATION_RECEIVED"},
                {T::UNSUPPORTED_OPERATION, "UNSUPPORTED_OPERATION"},
                {T::INTERNAL_ERROR, "INTERNAL_ERROR"},
                {T::SUCCESS, "SUCCESS"}};
    }
};

using JResultType = JEnum<CustomDomainHandler::ResultType, JResultTypeConfig>;

}  // namespace customDomain
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_CUSTOMDOMAIN_CUSTOMDOMAIN_BINDER_H