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

package com.amazon.sampleapp.impl.AuthProvider;

import android.app.Activity;
import android.view.View;
import android.widget.ImageButton;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AuthProviderHandler extends AuthProvider implements AuthStateObserver {

    private static final String sTag = "AuthProvider";

    private final LoggerHandler mLogger;
    private final Activity mActivity;
    private AuthHandler mAuthHandler;

    private AuthState mAuthState = AuthState.UNINITIALIZED;
    private String mAuthToken = "";

    private View mLoginView, mLogoutView;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    public AuthProviderHandler(Activity activity, LoggerHandler logger, AuthHandler handler ) {
        mLogger = logger;
        mActivity = activity;

        mAuthHandler = handler;

        setupGUI();
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

    public void onAuthStateChanged( AuthState state, AuthError error, String token ) {
        mAuthToken = token;
        mAuthState = state;
        mLogger.postVerbose( sTag, String.format( "Auth State Changed. STATE: %s, ERROR: %s ", mAuthState, error) );
        mExecutor.execute( new AuthStateChangedRunnable( mAuthState, error));
    }

    private class AuthStateChangedRunnable implements Runnable {
        AuthState state;
        AuthError error;
        AuthStateChangedRunnable( AuthState s, AuthError e){
            state = s;
            error = e;
        }
        public void run() {
            switch( state ){
                case REFRESHED:
                    updateLoginView( true );
                    break;
                case UNINITIALIZED:
                    updateLoginView( false );
                    break;
            }
            // call to update engine
            authStateChange( state, error );
        }
    }

    private void setupGUI() {
        // Login button
        mLoginView = mActivity.findViewById( R.id.login );
        ImageButton loginButton = mLoginView.findViewById( R.id.loginButton );
        loginButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                mAuthHandler.authorize();
            }
        } );

        // Logout button
        mLogoutView = mActivity.findViewById( R.id.logout );
        mLogoutView.findViewById( R.id.logoutButton ).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick( View v ) { mAuthHandler.deauthorize(); }
                }
        );
    }

    private void updateLoginView(final boolean loggedIn ) {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                if ( loggedIn ) {
                    mLoginView.setVisibility( View.GONE ) ;
                    mLogoutView.setVisibility( View.VISIBLE );
                } else {
                    mLoginView.setVisibility( View.VISIBLE );
                    mLogoutView.setVisibility( View.GONE );
                }
            }
        });
    }

    // After Engine has been started, register this as observer of the auth handler
    public void onInitialize(){
        mAuthHandler.registerAuthStateObserver( this );
    }

}
