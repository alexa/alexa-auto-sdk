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

#ifndef AACE_CBL_CBL_H
#define AACE_CBL_CBL_H

#include <iostream>
#include "CBLEngineInterface.h"
#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace cbl {

/**
 * CBL should be extended to perform CBL authentication 
 * 
 * @deprecated This platform interface is being deprecated. Use `aace::authorization::Authorization` instead.
 */
class CBL : public aace::core::PlatformInterface {
protected:
    CBL() = default;

public:
    /**
     * Specifies the state of the authorization flow
     * 
     * @deprecated This enum is being deprecated as this platform interface is being deprecated.
     */
    enum class CBLState {

        /**
         * CBL process is starting
         */
        STARTING,

        /**
         * Initiating the process to request a code pair
         */
        REQUESTING_CODE_PAIR,

        /**
         * Code pair is received and is waiting on user to authenticate 
         */
        CODE_PAIR_RECEIVED,

        /**
         * Refreshing token stage has begun 
         */
        REFRESHING_TOKEN,

        /**
         * Requesting for authorization token
         */
        REQUESTING_TOKEN,

        /**
         * CBL process is ending
         */
        STOPPING
    };

    /**
     * Specifies the reason for the state change
     * 
     * @deprecated This enum is being deprecated as this platform interface is being deprecated.
     */
    enum class CBLStateChangedReason {

        /**
         * The CBL state changed successfully
         */
        SUCCESS,

        /**
         * Error occured in the CBL process
         */
        ERROR,

        /**
         * Request timed out
         */
        TIMEOUT,

        /**
         * Code pair has expired and user will need to initiate the authentication process again
         */
        CODE_PAIR_EXPIRED,

        /**
         * The refresh token is invalid, revoked, or was issued to a different client.
         */
        AUTHORIZATION_EXPIRED,

        /**
         * No reason specified
         */
        NONE
    };

    virtual ~CBL();

    /**
     * Notifies the platform implementation of an authorization flow state change
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    virtual void cblStateChanged(
        CBLState state,
        CBLStateChangedReason reason,
        const std::string& url,
        const std::string& code) = 0;

    /**
     * Notifies the platform implementation to clear the refresh token
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    virtual void clearRefreshToken() = 0;

    /**
     * Notifies the platform implemnentation to set the refresh token
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    virtual void setRefreshToken(const std::string& refreshToken) = 0;

    /**
     * Returns the refresh token stored by the platform implementation, otherwise return an empty string
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    virtual std::string getRefreshToken() = 0;

    /**
     * Notifies the platform implementation to set the user profile
     * requestUserProfile must be enabled in configuration
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    virtual void setUserProfile(const std::string& name, const std::string& email) = 0;

    /**
     * Notifies the Engine to begin the authorization process
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    void start();

    /** 
     * Notifies the Engine to cancel the authorization process
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    void cancel();

    /**
     * Notifies the Engine to reset the authorization state
     * 
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     * 
     * Note: Starting with Auto SDK 3.1, this API ensures that all the persisted user-related data is removed. You no longer have to restart the Engine before starting the next user login process.
     */
    void reset();

    /**
     * @internal
     * Sets the Engine interface delagate
     * 
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<CBLEngineInterface> cblEngineInterface);

private:
    std::shared_ptr<CBLEngineInterface> m_cblEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const CBL::CBLState& state) {
    switch (state) {
        case CBL::CBLState::STARTING:
            stream << "STARTING";
            break;
        case CBL::CBLState::REQUESTING_CODE_PAIR:
            stream << "REQUESTING_CODE_PAIR";
            break;
        case CBL::CBLState::CODE_PAIR_RECEIVED:
            stream << "CODE_PAIR_RECEIVED";
            break;
        case CBL::CBLState::REFRESHING_TOKEN:
            stream << "REFRESHING_TOKEN";
            break;
        case CBL::CBLState::REQUESTING_TOKEN:
            stream << "REQUESTING_TOKEN";
            break;
        case CBL::CBLState::STOPPING:
            stream << "STOPPING";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const CBL::CBLStateChangedReason& reason) {
    switch (reason) {
        case CBL::CBLStateChangedReason::SUCCESS:
            stream << "SUCCESS";
            break;
        case CBL::CBLStateChangedReason::ERROR:
            stream << "ERROR";
            break;
        case CBL::CBLStateChangedReason::TIMEOUT:
            stream << "TIMEOUT";
            break;
        case CBL::CBLStateChangedReason::CODE_PAIR_EXPIRED:
            stream << "CODE_PAIR_EXPIRED";
            break;
        case CBL::CBLStateChangedReason::AUTHORIZATION_EXPIRED:
            stream << "AUTHORIZATION_EXPIRED";
            break;
        case CBL::CBLStateChangedReason::NONE:
            stream << "NONE";
            break;
    }
    return stream;
}

}  // namespace cbl
}  // namespace aace

#endif
