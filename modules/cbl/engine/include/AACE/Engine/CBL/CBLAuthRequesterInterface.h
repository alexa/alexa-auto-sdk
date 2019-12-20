/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CBL_CBL_AUTH_REQUESTER_INTERFACE_H
#define AACE_ENGINE_CBL_CBL_AUTH_REQUESTER_INTERFACE_H

#include <string>
#include <AACE/CBL/CBL.h>

namespace aace {
namespace engine {
namespace cbl {

/**
 * Interface for receiving requests for authorization from a CBLAuthDelegate.
 */
class CBLAuthRequesterInterface {
public:

    using CBLStateChangedReason = aace::cbl::CBL::CBLStateChangedReason;
    using CBLState = aace::cbl::CBL::CBLState;
    
    virtual void cblStateChanged( CBLState state, CBLStateChangedReason reason, const std::string& url = "", const std::string& code = "" ) = 0;
    virtual void clearRefreshToken() = 0;
    virtual void setRefreshToken( const std::string& refreshToken ) = 0;
    virtual std::string getRefreshToken() = 0;
    virtual void setUserProfile( const std::string& name, const std::string& email ) = 0;

    virtual ~CBLAuthRequesterInterface() = default;
};

} // aace::engine::cbl
} // aace::engine
} // aace

#endif
