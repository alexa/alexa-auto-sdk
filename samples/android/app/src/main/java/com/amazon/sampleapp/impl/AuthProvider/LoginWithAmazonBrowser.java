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
import android.content.SharedPreferences;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.identity.auth.device.AuthError;
import com.amazon.identity.auth.device.api.Listener;
import com.amazon.identity.auth.device.api.authorization.AuthCancellation;
import com.amazon.identity.auth.device.api.authorization.AuthorizationManager;
import com.amazon.identity.auth.device.api.authorization.AuthorizeListener;
import com.amazon.identity.auth.device.api.authorization.AuthorizeRequest;
import com.amazon.identity.auth.device.api.authorization.AuthorizeResult;
import com.amazon.identity.auth.device.api.authorization.Scope;
import com.amazon.identity.auth.device.api.authorization.ScopeFactory;
import com.amazon.identity.auth.device.api.authorization.User;
import com.amazon.identity.auth.device.api.workflow.RequestContext;

import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.Arrays;
import java.util.Observable;
import java.util.Timer;
import java.util.TimerTask;

class LoginWithAmazonBrowser extends Observable {

    private static final String sTag = "LWA";

    private static final String sAlexaAllScope = "alexa:all";
    private static final String sProfileScope = "profile";

    // Refresh access token 2 minutes before it expires
    private static final int sRefreshAccessTokenTime = 120000;
    // Access token expires after one hour
    private static final int sAccessTokenExpirationTime = 3600000;

    private final SharedPreferences mPreferences;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AuthProviderHandler mAuthProvider;
    private final RequestContext mRequestContext;
    private String mProductID;
    private String mProductDSN;
    private boolean mHasApiKey = false;
    private Timer mTimer = new Timer();
    private TimerTask mRefreshTimerTask;

    LoginWithAmazonBrowser( Activity activity,
                            SharedPreferences preferences,
                            LoggerHandler logger,
                            AuthProviderHandler authProvider ) {
        mActivity = activity;
        mPreferences = preferences;
        mLogger = logger;
        mAuthProvider = authProvider;
        mHasApiKey = false;

        mProductID = mPreferences.getString( mActivity.getString( R.string.preference_product_id ), "" );
        mProductDSN = mPreferences.getString( mActivity.getString( R.string.preference_product_dsn ), "" );

        // Check for API key
        try {
            if ( Arrays.asList( activity.getResources().getAssets().list( "" ) )
                    .contains( "api_key.txt" ) ) {
                mHasApiKey = true;
            } else mLogger.postWarn( sTag, "api_key.txt does not exist in assets folder" );
        } catch ( IOException e ) {
            mLogger.postWarn( sTag, "Cannot find api_key.txt in assets folder" );
        }

        mRequestContext = RequestContext.create( activity );

        setupLWA();
    }

    private void setupLWA() {
        try {
            mRequestContext.registerListener( new AuthorizeListener() {

                @Override
                public void onSuccess( AuthorizeResult result ) {
                    String accessToken = result.getAccessToken();
                    if ( accessToken != null && !accessToken.equals( "" ) ) {
                        mLogger.postVerbose(  sTag,
                                "Refreshing Auth State with token: " + accessToken );
                        mAuthProvider.setAuthToken( accessToken );
                        mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.REFRESHED,
                                AuthProvider.AuthError.NO_ERROR );
                        setChanged();
                        notifyObservers( "logged in" );

                        // show user profile info
                        logUserProfile( result.getUser() );

                        startRefreshTimer();

                    } else mLogger.postError( sTag, "Authorization failed. Access token was not set." );

                }

                /* Inform the AuthProvider of auth failure  */
                @Override
                public void onError( AuthError ae ) {
                    mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                            AuthProvider.AuthError.AUTHORIZATION_FAILED );
                    mLogger.postError( sTag, "Authorization failed. Error: "
                            + ae.getMessage() );
                }

                /* Authorization was cancelled before it could be completed. */
                @Override
                public void onCancel( AuthCancellation cancellation )
                {
                    mLogger.postWarn( sTag,
                            "Authorization cancelled before completion. Message: "
                            + cancellation.getDescription() );
                }
            });

        } catch ( Throwable e ) {
            mLogger.postError( sTag, e );
        }

    }

    void login() {
        final JSONObject scopeData = new JSONObject();
        final JSONObject productInstanceAttributes = new JSONObject();
        if ( mHasApiKey ) {
            mLogger.postInfo( sTag, "Attempting to authenticate" );
            try {
                productInstanceAttributes.put( "deviceSerialNumber", mProductDSN );
                scopeData.put( "productInstanceAttributes", productInstanceAttributes );
                scopeData.put( "productID", mProductID );

                // Save logged in method and access token
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.putString( mActivity.getString( R.string.preference_login_method ), LoginWithAmazon.LWA_LOGIN_METHOD_KEY );
                editor.apply();

                AuthorizationManager.authorize( new AuthorizeRequest
                        .Builder( mRequestContext )
                        .addScopes( ScopeFactory.scopeNamed( sAlexaAllScope, scopeData ), ScopeFactory.scopeNamed( sProfileScope ) )
                        .forGrantType( AuthorizeRequest.GrantType.ACCESS_TOKEN )
                        .shouldReturnUserData( true )
                        .build()
                );

            } catch ( JSONException e ) {
                mLogger.postError( sTag, e.getMessage() );
            }
        } else mLogger.postWarn( sTag, "Cannot authenticate. assets/api_key.txt does not exist" );
    }

    void logout() {
        mLogger.postInfo( sTag, "Attempting to de-authenticate" );
        AuthorizationManager.signOut( mActivity.getApplicationContext(),
                new Listener<Void, AuthError>() {
            @Override
            public void onSuccess( Void aVoid ) {
                if ( mRefreshTimerTask != null ) mRefreshTimerTask.cancel();

                // Save logged in method and access token
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.putString( mActivity.getString( R.string.preference_login_method ), "" );
                editor.apply();

                mAuthProvider.clearAuthToken();
                mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                        AuthProvider.AuthError.NO_ERROR );
                setChanged();
                notifyObservers( "logged out" );
            }

            @Override
            public void onError( AuthError ae ) {
                mLogger.postError( sTag, "Unable to log out. Error: " + ae.getMessage() );
            }
        });
    }

    void onInitialize() {
        if ( mHasApiKey ) {
            AuthorizationManager.getToken( mActivity, new Scope[]{ ScopeFactory.scopeNamed( sAlexaAllScope ),
                    ScopeFactory.scopeNamed( sProfileScope ) }, new TokenListener() );
        }
    }

    void onResume() {
        if ( mRequestContext != null ) mRequestContext.onResume();
    }

    private class TokenListener implements Listener<AuthorizeResult, AuthError> {

        /* will authorize if client already has access token from previous session */
        @Override
        public void onSuccess( AuthorizeResult result ) {
            String accessToken = result.getAccessToken();
            if ( accessToken != null ) {
                mAuthProvider.setAuthToken( accessToken );
                mLogger.postVerbose( sTag,
                        "Refreshing auth state with token: " + accessToken );
                mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.REFRESHED,
                        AuthProvider.AuthError.NO_ERROR );
                setChanged();
                notifyObservers( "logged in" );
                startRefreshTimer();
            }
        }

        @Override
        public void onError( AuthError ae ) {
            mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                    AuthProvider.AuthError.AUTHORIZATION_FAILED );
            mLogger.postError( sTag, "Authorization failed. Error: " + ae.getMessage() );
        }
    }

    private void startRefreshTimer() {
        mTimer.schedule( mRefreshTimerTask = new TimerTask() {
            public void run() {
                if ( mHasApiKey ) {
                    AuthorizationManager.getToken( mActivity, new Scope[]{ ScopeFactory.scopeNamed( sAlexaAllScope ),
                            ScopeFactory.scopeNamed( sProfileScope ) }, new TokenListener() );
                }
            }
        }, sAccessTokenExpirationTime - sRefreshAccessTokenTime );
    }

    private void logUserProfile( User user ) {
        if ( user != null ) {
            mLogger.postInfo( sTag, String.format( "User Profile: Name: %s, Email: %s, User ID: %s",
                    user.getUserName(), user.getUserEmail(), user.getUserId() ) );
        } else mLogger.postError( sTag, "Fetched user is null." );
    }

}
