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
import android.app.AlertDialog;
import android.arch.lifecycle.Lifecycle;
import android.arch.lifecycle.LifecycleObserver;
import android.arch.lifecycle.OnLifecycleEvent;
import android.content.Context;
import android.content.SharedPreferences;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.aace.network.NetworkInfoProvider;
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
import com.amazon.sampleapp.impl.NetworkInfoProvider.NetworkConnectionObserver;

import org.json.JSONObject;

import java.util.HashSet;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/*
* Login with Amazon in a browser workflow with the login-with-amazon-sdk library.
* See the README for full setup instructions.
* See https://developer.amazon.com/docs/login-with-amazon/android-docs.html for additional reference.
*/
public class LoginWithAmazonBrowser implements AuthHandler, NetworkConnectionObserver, LifecycleObserver {

    private static final String sTag = "LWA";

    private static final String sAlexaAllScope = "alexa:all";
    private static final String sProfileScope = "profile";

    // To fetch User Profile data, set the sUserProfileEnabled to true
    // You will need additional parameters in your Security Profile for the profile scope request to succeed,
    // please see the README CBL section for more.
    private static final boolean sUserProfileEnabled = false;

    // Refresh access token 2 minutes before it expires
    private static final int sRefreshAccessTokenTime = 120000;
    // Access token expires after one hour
    private static final int sAccessTokenExpirationTime = 3600000;

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final RequestContext mRequestContext;
    private String mProductID;
    private String mProductDSN;

    private Timer mTimer = new Timer();
    private TimerTask mRefreshTimerTask;

    // List of Authentication observers
    private Set<AuthStateObserver> mObservers;

    private AuthProvider.AuthState mCurrentAuthState;
    private AuthProvider.AuthError mCurrentAuthError;
    private String mCurrentAuthToken;

    // assume connected in case of no network info provider
    private boolean mConnected = true;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    public LoginWithAmazonBrowser( Activity activity,
                            LoggerHandler logger, Lifecycle lifecycle ) {
        mActivity = activity;
        SharedPreferences preferences = activity.getSharedPreferences(
                activity.getString( R.string.preference_file_key ), Context.MODE_PRIVATE );
        mLogger = logger;
        mRequestContext = RequestContext.create( mActivity );
        mObservers = new HashSet<>(1);

        mCurrentAuthState = AuthProvider.AuthState.UNINITIALIZED;
        mCurrentAuthError = AuthProvider.AuthError.NO_ERROR;
        mCurrentAuthToken = "";

        mProductID = preferences.getString( mActivity.getString( R.string.preference_product_id ), "" );
        mProductDSN = preferences.getString( mActivity.getString( R.string.preference_product_dsn ), "" );

        lifecycle.addObserver( this );
        setupLWA();
    }

    // Auth State Observable interface implementation
    public void authorize() {
        final JSONObject scopeData = new JSONObject();
        final JSONObject productInstanceAttributes = new JSONObject();
        if ( mConnected ) {
            mLogger.postInfo( sTag, "Attempting to authenticate" );
            try {
                productInstanceAttributes.put( "deviceSerialNumber", mProductDSN );
                scopeData.put( "productInstanceAttributes", productInstanceAttributes );
                scopeData.put( "productID", mProductID );

                if ( sUserProfileEnabled ) {
                    AuthorizationManager.authorize( new AuthorizeRequest
                            .Builder( mRequestContext )
                            .addScopes( ScopeFactory.scopeNamed( sAlexaAllScope, scopeData ), ScopeFactory.scopeNamed( sProfileScope ) )
                            .forGrantType( AuthorizeRequest.GrantType.ACCESS_TOKEN )
                            .shouldReturnUserData( true )
                            .build()
                    );
                } else {
                    AuthorizationManager.authorize(new AuthorizeRequest
                            .Builder( mRequestContext )
                            .addScope( ScopeFactory.scopeNamed( sAlexaAllScope, scopeData ) )
                            .forGrantType( AuthorizeRequest.GrantType.ACCESS_TOKEN )
                            .shouldReturnUserData( false )
                            .build()
                    );
                }
            } catch ( Exception e ) { mLogger.postError( sTag, e.getMessage() ); }
        } else {
            mLogger.postWarn( sTag, "Internet not available. Please verify your network settings." );
            AlertDialog.Builder builder = new AlertDialog.Builder( mActivity ) ;
            builder.setTitle( "Internet not available" );
            builder.setIcon( android.R.drawable.ic_dialog_alert );
            builder.setMessage( "Please verify your network settings." );
            builder.setCancelable( false );
            builder.setPositiveButton( "OK", null );
            AlertDialog alert = builder.create();
            alert.show();
        }
    }

    public void deauthorize() {
        mLogger.postInfo( sTag, "Attempting to de-authenticate" );
        AuthorizationManager.signOut( mActivity.getApplicationContext(),
                new Listener<Void, AuthError>() {
                    @Override
                    public void onSuccess( Void aVoid ) {
                        if ( mRefreshTimerTask != null ) mRefreshTimerTask.cancel();
                        updateCurrentAuthStatus( AuthProvider.AuthState.UNINITIALIZED, AuthProvider.AuthError.NO_ERROR, "" );
                    }

                    @Override
                    public void onError( AuthError ae ) {
                        mLogger.postError( sTag, "Unable to log out. Error: " + ae.getMessage() );
                    }
                });
    }

    public void registerAuthStateObserver( AuthStateObserver observer ) {
        if (observer != null) {
            mObservers.add(observer);
            notifyAuthObservers();
        }
    }

    private void updateCurrentAuthStatus( AuthProvider.AuthState state, AuthProvider.AuthError error, String token ){
        if ( mCurrentAuthState != state || mCurrentAuthError != error || !mCurrentAuthToken.equals(token) ) {
            mCurrentAuthState = state;
            mCurrentAuthError = error;
            mCurrentAuthToken = token;
            notifyAuthObservers();
        }
    }

    private void notifyAuthObservers(){
        for (AuthStateObserver observer : mObservers) {
            observer.onAuthStateChanged( mCurrentAuthState, mCurrentAuthError, mCurrentAuthToken );
        }
    }

    // Network Connection Observer interface implementation
    public void onConnectionStatusChanged( NetworkInfoProvider.NetworkStatus status ){
        mConnected = status == NetworkInfoProvider.NetworkStatus.CONNECTED;
        mExecutor.execute( new LoginWithAmazonBrowser.ConnectionStateChangedRunnable() );
    }

    private class ConnectionStateChangedRunnable implements Runnable {
        ConnectionStateChangedRunnable(){}
        public void run() {
            // Refresh access token on restored connection if not already refreshed
            if ( mConnected && mCurrentAuthState != AuthProvider.AuthState.REFRESHED ) {
                getToken();
            }
        }
    }

    // Activity lifecycle event listener
    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    public void onResume(){
        mRequestContext.onResume();
    }

    // Class methods
    private void setupLWA() {
        try {
            mRequestContext.registerListener( new AuthorizeListener() {

                @Override
                public void onSuccess( AuthorizeResult result ) {
                    String accessToken = result.getAccessToken();
                    if ( accessToken != null && !accessToken.equals( "" ) ) {
                        updateCurrentAuthStatus( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR, accessToken);

                        // show user profile info
                        if ( sUserProfileEnabled ) {
                            logUserProfile(result.getUser());
                        }

                        startRefreshTimer();

                    } else mLogger.postError( sTag, "Authorization failed. Access token was not set." );

                }

                @Override
                public void onError( AuthError ae ) {
                    updateCurrentAuthStatus( AuthProvider.AuthState.UNINITIALIZED, AuthProvider.AuthError.AUTHORIZATION_FAILED, "");

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

        } catch ( Exception e ) { mLogger.postError( sTag, e ); }
    }

    private void getToken() {
        try {
            if (sUserProfileEnabled) {
                AuthorizationManager.getToken( mActivity,
                        new Scope[]{ScopeFactory.scopeNamed(sAlexaAllScope),
                        ScopeFactory.scopeNamed(sProfileScope)},
                        new TokenListener());
            } else {
                AuthorizationManager.getToken(mActivity,
                        new Scope[]{ScopeFactory.scopeNamed(sAlexaAllScope)},
                        new TokenListener());
            }
        } catch (Exception e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    private class TokenListener implements Listener<AuthorizeResult, AuthError> {

        /* will authorize if client already has access token from previous session */
        @Override
        public void onSuccess( AuthorizeResult result ) {
            String accessToken = result.getAccessToken();
            if ( accessToken != null ) {
                updateCurrentAuthStatus( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR, accessToken);
                startRefreshTimer();
            }
        }

        @Override
        public void onError( AuthError ae ) {
            updateCurrentAuthStatus( AuthProvider.AuthState.UNINITIALIZED, AuthProvider.AuthError.AUTHORIZATION_FAILED, "");
            mLogger.postError( sTag, "Authorization failed. Error: " + ae.getMessage() );
        }
    }

    private void startRefreshTimer() {
        mTimer.schedule( mRefreshTimerTask = new TimerTask() {
            public void run() {
                if ( mConnected ) {
                    getToken();
                } else {
                    updateCurrentAuthStatus( AuthProvider.AuthState.EXPIRED, AuthProvider.AuthError.AUTHORIZATION_EXPIRED, "");
                    mLogger.postInfo(sTag, "Authorization refresh failed due to no internet connection");
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