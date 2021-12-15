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

#ifndef AACE_JNI_AASB_AASB_BINDER_H
#define AACE_JNI_AASB_AASB_BINDER_H

#include <AACE/AASB/AASB.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace aasb {

class AASBHandler : public aace::aasb::AASB {
public:
    AASBHandler(jobject obj);

    // aace::aasb::AASB
    void messageReceived(const std::string& message) override;

private:
    JObject m_obj;
};

class AASBBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AASBBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_aasbHandler;
    }

    std::shared_ptr<AASBHandler> getAASB() {
        return m_aasbHandler;
    }

private:
    std::shared_ptr<AASBHandler> m_aasbHandler;
};

}  // namespace aasb
}  // namespace jni
}  // namespace aace

#endif  //AACE_JNI_AASB_AASB_BINDER_H
