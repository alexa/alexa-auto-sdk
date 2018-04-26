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

package com.amazon.sampleapp;

import android.app.Activity;
import android.graphics.Color;
import android.util.Log;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.aace.logger.Logger;
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
import com.amazon.sampleapp.impl.AuthProviderHandler;
import com.amazon.sampleapp.impl.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Observable;

public class LoginWithAmazon extends Observable
{
    private static final Scope ALEXA_ALL_SCOPE = ScopeFactory.scopeNamed( "alexa:all" );

    private String mProductID;
    private String mProductDSN;
    private boolean mAPIKey;
    private LoggerHandler mLogger;
    private AuthProviderHandler mAuthProvider;
    private RequestContext mRequestContext;
    private Activity mActivity;

    public LoginWithAmazon( LoggerHandler logger, AuthProviderHandler authProvider, boolean APIKey, Activity activity )
    {
        mLogger = logger;
        mAuthProvider = authProvider;
        mAPIKey = APIKey;
        mRequestContext = RequestContext.create( activity );
        mActivity = activity;
    }

    public void loginWithAmazon()
    {
        try
        {
            mRequestContext.registerListener( new AuthorizeListener()
            {
                /* Authorization was completed successfully. */
                @Override
                public void onSuccess( AuthorizeResult result )
                {
                    if ( result.getAccessToken() != null && !result.getAccessToken().equals( "" ) )
                    {
                        mAuthProvider.authToken = result.getAccessToken();
                        mAuthProvider.authState = AuthProvider.AuthState.REFRESHED;
                        mAuthProvider.authStateChange( mAuthProvider.authState, AuthProvider.AuthError.NO_ERROR );
                        mLogger.post( "LWA: AUTHSTATE REFRESHED WITH TOKEN:" + mAuthProvider.authToken, Logger.Level.INFO, Color.GREEN );
                        setChanged();
                        notifyObservers( "logged in" );
                    } else mLogger.post( "LWA: AUTHORIZATION FAILED", Logger.Level.ERROR, Color.RED );
                }

                /* There was an error during the attempt to authorize the
                application. */
                @Override
                public void onError( AuthError ae )
                {
                    Log.e( "MainActivity", "LWA Error: " + ae );
                }

                /* Authorization was cancelled before it could be completed. */
                @Override
                public void onCancel( AuthCancellation cancellation )
                {
                    Log.w( "MainActivity", "LWA Cancelled: " + cancellation );
                }
            });

        } catch ( Throwable ex )
        {
            ex.printStackTrace();
        }
    }

    public void loginWithAmazonBrowser()
    {
        final JSONObject scopeData = new JSONObject();
        final JSONObject productInstanceAttributes = new JSONObject();
        if( mAPIKey )
        {
            try
            {
                productInstanceAttributes.put( "deviceSerialNumber", mProductDSN );
                scopeData.put( "productInstanceAttributes", productInstanceAttributes );
                scopeData.put( "productID", mProductID );

                AuthorizationManager.authorize( new AuthorizeRequest
                    .Builder( mRequestContext )
                    .addScopes( ScopeFactory.scopeNamed( "alexa:all", scopeData ) )
                    .forGrantType( AuthorizeRequest.GrantType.ACCESS_TOKEN )
                    .shouldReturnUserData( false )
                    .build() );
            } catch ( JSONException e )
            {
                Log.e( "Error", "err", e );
                mLogger.post( e );
            }
        } else mLogger.post( "LWA: assets/api_key.txt does not exist", Logger.Level.WARN, Color.YELLOW );
    }

    public void logout()
    {
        AuthorizationManager.signOut( mActivity.getApplicationContext(), new Listener<Void, AuthError>()
        {
            @Override
            public void onSuccess( Void aVoid )
            {
                mAuthProvider.authState = AuthProvider.AuthState.UNINITIALIZED;
                mAuthProvider.authStateChange( AuthProvider.AuthState.UNINITIALIZED, AuthProvider.AuthError.NO_ERROR );
                setChanged();
                notifyObservers( "logged out" );
            }

            @Override
            public void onError( AuthError authError )
            {
                mLogger.post( "LWA: cannot log out. " + authError.getMessage(), Logger.Level.WARN, Color.YELLOW );
            }
        });
    }

    public void onResume()
    {
        if( mLogger != null )
        {
            if( mRequestContext != null )
            {
                mRequestContext.onResume();
            }
            if( mAPIKey )
            {
                Scope[] scopes = { ALEXA_ALL_SCOPE };
                AuthorizationManager.getToken( mActivity, scopes, new TokenListener() );
            }
        }
    }

    public void setProductID( String productId )
    {
        mProductID = productId;
    }

    public void setProductDSN( String productDSN )
    {
        mProductDSN = productDSN;
    }

    private class TokenListener implements Listener<AuthorizeResult, AuthError>
    {
        /* getToken completed successfully. */
        @Override
        public void onSuccess( AuthorizeResult result )
        {
            if( result.getAccessToken() != null )
            {
                mAuthProvider.authToken = result.getAccessToken();
                mLogger.post("LWA: AUTHSTATE REFRESHED ON START WITH TOKEN:" + mAuthProvider.authToken, Logger.Level.INFO, Color.GREEN);
                mAuthProvider.authState = AuthProvider.AuthState.REFRESHED;
                mAuthProvider.authStateChange( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR );
                setChanged();
                notifyObservers( "logged in" );

            } else mLogger.post("LWA: NO REFRESH TOKEN", Logger.Level.WARN, Color.YELLOW);
        }

        @Override
        public void onError( AuthError ae )
        {
            mLogger.post( "LWA: USER NOT LOGGED IN AT START AND ERROR: " + ae, Logger.Level.ERROR, Color.RED );
        }
    }
}
