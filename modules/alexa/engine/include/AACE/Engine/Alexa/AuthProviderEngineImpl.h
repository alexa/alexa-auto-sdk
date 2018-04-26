/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H

#include <AuthDelegate/AuthDelegate.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/AuthProvider.h"

namespace aace {
namespace engine {
namespace alexa {
    
class AuthProviderEngineImpl :
    public aace::alexa::AuthProviderEngineInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {

private:
    AuthProviderEngineImpl();
    
public:
    static std::shared_ptr<AuthProviderEngineImpl> create();
                                   
    void setAuthProvider( std::shared_ptr<aace::alexa::AuthProvider> authProvider );
    
    // AuthProviderEngineInterface
    void onAuthStateChanged( AuthState authState, AuthError authError ) override;
    
    // AuthDelegateInterface
    void addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    void removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    std::string getAuthToken() override;
    
private:
    /// token provider, supplied by the client
    std::shared_ptr<aace::alexa::AuthProvider> m_authProvider;
    
    /// holds a set of observers on the AuthProvider's state and error.
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface>> m_observers;
    
    /// the current auth status
    AuthState m_authState;
    
    /// the current auth error
    AuthError m_authError;
    
    /// mutex to ensure thread-safe access on the AuthProvider
    std::mutex m_mutex;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H
