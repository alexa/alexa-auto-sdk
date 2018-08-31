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

package com.amazon.sampleapp.impl.AuthProvider;

import android.app.Activity;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class AuthProviderHandler extends AuthProvider {

    private static final String sTag = "AuthProvider";

    private final LoggerHandler mLogger;
    private final LoginWithAmazon mLwa;
    private AuthState mAuthState = AuthState.UNINITIALIZED;
    private String mAuthToken = "";

    public AuthProviderHandler( Activity activity, LoggerHandler logger ) {
        mLogger = logger;

        // Authenticate with LWA
        mLwa = new LoginWithAmazon( mLogger, activity, this );
    }

    @Override
    public String getAuthToken() {
        if ( mAuthToken.equals( "" ) ) {
            mLogger.postWarn( sTag, "Auth token is not set" );
        }
        return mAuthToken;
    }

    @Override
    public AuthState getAuthState() {
        mLogger.postVerbose( sTag, String.format( "Auth State Retrieved. STATE: %s", mAuthState ) );
        return mAuthState;
    }

    public void onAuthStateChanged( AuthState authState, AuthError authError ) {
        mAuthState = authState;
        mLogger.postInfo( sTag, String.format( "Auth State Changed. STATE: %s, ERROR: %s",
                authState, authError ) );
        authStateChange( authState, authError );
    }

    void setAuthToken( String authToken ) { mAuthToken = authToken; }

    void clearAuthToken() { mAuthToken = ""; }

    public void onResume() { if ( mLwa != null ) mLwa.onResume(); }
    public void onInitialize() { if ( mLwa != null ) mLwa.onInitialize(); }
}
