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
import com.amazon.identity.auth.device.api.workflow.RequestContext;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.Arrays;
import java.util.Observable;

class LoginWithAmazonBrowser extends Observable {

    private static final String sTag = "LWA";
    private static final Scope ALEXA_ALL_SCOPE = ScopeFactory.scopeNamed( "alexa:all" );

    private final SharedPreferences mPreferences;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AuthProviderHandler mAuthProvider;
    private final RequestContext mRequestContext;
    private String mProductID;
    private String mProductDSN;
    private boolean mHasApiKey = false;

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
                /* Authorization was completed successfully. */
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
                    } else mLogger.postError( sTag, "AUTHORIZATION FAILED" );
                }

                /* There was an error during the attempt to authorize the application. */
                @Override
                public void onError( AuthError ae ) {
                    mLogger.postError( sTag, "%s: AUTHORIZATION FAILED. Error: "
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

                AuthorizationManager.authorize( new AuthorizeRequest
                        .Builder( mRequestContext )
                        .addScopes( ScopeFactory.scopeNamed( "alexa:all", scopeData ) )
                        .forGrantType( AuthorizeRequest.GrantType.ACCESS_TOKEN )
                        .shouldReturnUserData( false )
                        .build()
                );

                // Clear refresh token in preferences (used only for CBL)
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.putString( mActivity.getString( R.string.preference_refresh_token ), "" );
                editor.apply();

            } catch ( JSONException e ) {
                mLogger.postError( sTag, e.getMessage() );
            }
        } else mLogger.postWarn( sTag,
                "Cannot authenticate. assets/api_key.txt does not exist" );
    }

    void logout() {
        AuthorizationManager.signOut( mActivity.getApplicationContext(),
                new Listener<Void, AuthError>() {
            @Override
            public void onSuccess( Void aVoid ) {
                mLogger.postInfo( sTag, "Attempting to de-authenticate" );
                mAuthProvider.clearAuthToken();
                mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                        AuthProvider.AuthError.NO_ERROR );
                setChanged();
                notifyObservers( "logged out" );
            }

            @Override
            public void onError( AuthError ae ) {
                mLogger.postError( sTag, "Cannot log out. Error: " + ae.getMessage() );
            }
        });
    }

    void onResume() {
        if ( mRequestContext != null ) {
            mRequestContext.onResume();
            if ( mHasApiKey ) {
                Scope[] scopes = { ALEXA_ALL_SCOPE };
                AuthorizationManager.getToken( mActivity, scopes, new TokenListener() );
            }
        }
    }

    private class TokenListener implements Listener<AuthorizeResult, AuthError> {

        /* getToken completed successfully. */
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
            }
        }

        @Override
        public void onError( AuthError ae ) {
            mLogger.postError( sTag, "AUTHORIZATION FAILED. Error: " + ae.getMessage() );
        }
    }
}
