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

#ifndef AACE_JNI_CBL_CBL_BINDER_H
#define AACE_JNI_CBL_CBL_BINDER_H

#include <AACE/CBL/CBL.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace cbl {

//
// CBLHandler
//

class CBLHandler : public aace::cbl::CBL {
public:
    CBLHandler(jobject obj);

    // aace::cbl::CBL
    void cblStateChanged(CBLState state, CBLStateChangedReason reason, const std::string& url, const std::string& code)
        override;
    void clearRefreshToken() override;
    void setRefreshToken(const std::string& refreshToken) override;
    std::string getRefreshToken() override;
    void setUserProfile(const std::string& name, const std::string& email) override;

private:
    JObject m_obj;
};

//
// CBLBinder
//

class CBLBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    CBLBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_cblHandler;
    }

    std::shared_ptr<CBLHandler> getCBL() {
        return m_cblHandler;
    }

private:
    std::shared_ptr<CBLHandler> m_cblHandler;
};

//
// JCBLState
//

class JCBLStateConfig : public EnumConfiguration<CBLHandler::CBLState> {
public:
    using T = CBLHandler::CBLState;

    const char* getClassName() override {
        return "com/amazon/aace/cbl/CBL$CBLState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::STARTING, "STARTING"},
                {T::REQUESTING_CODE_PAIR, "REQUESTING_CODE_PAIR"},
                {T::CODE_PAIR_RECEIVED, "CODE_PAIR_RECEIVED"},
                {T::REFRESHING_TOKEN, "REFRESHING_TOKEN"},
                {T::REQUESTING_TOKEN, "REQUESTING_TOKEN"},
                {T::STOPPING, "STOPPING"}};
    }
};

using JCBLState = JEnum<CBLHandler::CBLState, JCBLStateConfig>;

//
// JCBLStateChangedReason
//

class JCBLStateChangedReasonConfig : public EnumConfiguration<CBLHandler::CBLStateChangedReason> {
public:
    using T = CBLHandler::CBLStateChangedReason;

    const char* getClassName() override {
        return "com/amazon/aace/cbl/CBL$CBLStateChangedReason";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SUCCESS, "SUCCESS"},
                {T::ERROR, "ERROR"},
                {T::TIMEOUT, "TIMEOUT"},
                {T::CODE_PAIR_EXPIRED, "CODE_PAIR_EXPIRED"},
                {T::AUTHORIZATION_EXPIRED, "AUTHORIZATION_EXPIRED"},
                {T::NONE, "NONE"}};
    }
};

using JCBLStateChangedReason = JEnum<CBLHandler::CBLStateChangedReason, JCBLStateChangedReasonConfig>;

}  // namespace cbl
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_CBL_CBL_BINDER_H