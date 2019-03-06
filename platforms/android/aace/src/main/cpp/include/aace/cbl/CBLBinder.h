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

// aace/cbl/CBL.h

#ifndef AACE_CBL_CBL_BINDER_H
#define AACE_CBL_CBL_BINDER_H

#include "AACE/CBL/CBL.h"
#include "aace/core/PlatformInterfaceBinder.h"


class CBLBinder : public PlatformInterfaceBinder, public aace::cbl::CBL {
public:
    CBLBinder() = default;

public:
    void cblStateChanged( CBLState state, CBLStateChangedReason reason, const std::string& url, const std::string& code ) override;
    void clearRefreshToken() override;
    void setRefreshToken( const std::string& refreshToken ) override;
    std::string getRefreshToken() override;

protected:
    void initialize( JNIEnv* env ) override;

private:
    jobject convert( CBLState state );
    jobject convert( CBLStateChangedReason reason );

private:
    jmethodID m_javaMethod_cblStateChanged_state_reason_url_code = nullptr;
    jmethodID m_javaMethod_clearRefreshToken = nullptr;
    jmethodID m_javaMethod_setRefreshToken_refreshToken = nullptr;
    jmethodID m_javaMethod_getRefreshToken = nullptr;

    // State
    ObjectRef m_enum_CBLState_STARTING;
    ObjectRef m_enum_CBLState_REQUESTING_CODE_PAIR;
    ObjectRef m_enum_CBLState_CODE_PAIR_RECEIVED;
    ObjectRef m_enum_CBLState_REFRESHING_TOKEN;
    ObjectRef m_enum_CBLState_REQUESTING_TOKEN;
    ObjectRef m_enum_CBLState_STOPPING;

    // Reason
    ObjectRef m_enum_CBLStateChangedReason_SUCCESS;
    ObjectRef m_enum_CBLStateChangedReason_ERROR;
    ObjectRef m_enum_CBLStateChangedReason_TIMEOUT;
    ObjectRef m_enum_CBLStateChangedReason_CODE_PAIR_EXPIRED;
    ObjectRef m_enum_CBLStateChangedReason_NONE;
};

#endif //AACE_CBL_CBL_BINDER_H
