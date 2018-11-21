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
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ToggleButton;

import com.amazon.aace.network.NetworkInfoProvider;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.NetworkInfoProvider.NetworkInfoProviderHandler;

import java.util.Observable;
import java.util.Observer;

class LoginWithAmazon implements Observer {

    static final String CBL_LOGIN_METHOD_KEY = "CBL";
    static final String LWA_LOGIN_METHOD_KEY = "LWA";

    private static final String sTag = "LoginWithAmazon";

    private final LoginWithAmazonBrowser mLwaBrowser;
    private final LoginWithAmazonCBL mLwaCBL;
    private final SharedPreferences mPreferences;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final NetworkInfoProviderHandler mNetworkInfoProviderHandler;

    private View mLoginView, mLogoutView;

    LoginWithAmazon( LoggerHandler logger,
                            Activity activity,
                            AuthProviderHandler authProvider,
                            NetworkInfoProviderHandler networkInfoProviderHandler ) {
        mActivity = activity;
        mLogger = logger;
        mNetworkInfoProviderHandler = networkInfoProviderHandler;
        mPreferences = activity.getSharedPreferences(
                activity.getString( R.string.preference_file_key ), Context.MODE_PRIVATE );
        mLwaBrowser = new LoginWithAmazonBrowser( activity, mPreferences, logger, authProvider );
        mLwaCBL = new LoginWithAmazonCBL( mActivity, mPreferences, logger, authProvider );

        setupGUI();
    }

    synchronized void onInitialize() {
        String loginMethod = mPreferences.getString( mActivity.getString( R.string.preference_login_method ), "" );
        if ( !loginMethod.isEmpty() ) { //We were already logged in previous session.
            if ( isConnected() ) {
                if ( loginMethod.equals( CBL_LOGIN_METHOD_KEY ) ) mLwaCBL.onInitialize();
                else if ( loginMethod.equals( LWA_LOGIN_METHOD_KEY ) ) mLwaBrowser.onInitialize();
            } else {
                mLogger.postWarn( sTag, "No Internet connection, cannot refresh connection for the logged in user. Please verify your network settings and restart the app to retry." );
                AlertDialog.Builder builder = new AlertDialog.Builder( mActivity );
                builder.setTitle( "No Internet Connection" );
                builder.setIcon( android.R.drawable.ic_dialog_alert );
                builder.setMessage( "Cannot refresh connection for the logged in user.\nPlease verify your network settings and restart the app to retry or click 'Logout' to log out." );
                builder.setCancelable( false );
                builder.setPositiveButton( "OK", null );
                builder.setNegativeButton( "Logout", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick( DialogInterface dialogInterface, int i ) {
                        logout();
                    }
                });
                AlertDialog alert = builder.create();
                alert.show();
            }
        }
    }

    void onResume() {
        String loginMethod = mPreferences.getString( mActivity.getString( R.string.preference_login_method ), "" );
        if ( loginMethod.equals( LWA_LOGIN_METHOD_KEY ) ) mLwaBrowser.onResume();
    }

    private void logout() {
        String loginMethod = mPreferences.getString( mActivity.getString( R.string.preference_login_method ), "" );
        if ( loginMethod.equals( CBL_LOGIN_METHOD_KEY ) ) mLwaCBL.logout();
        else if ( loginMethod.equals( LWA_LOGIN_METHOD_KEY ) ) mLwaBrowser.logout();
        else mLogger.postError(sTag, "Logout Called, but no Login method saved in preferences" );
    }

    //
    // For updating GUI
    //

    @Override
    public void update( final Observable o, final Object arg ) {
        if ( o instanceof LoginWithAmazonBrowser || o instanceof LoginWithAmazonCBL ) {
            final String message = arg.toString();
            mActivity.runOnUiThread( new Runnable() {
                @Override
                public void run() {
                    if ( message.equals( "logged in" ) ) {
                        mLoginView.setVisibility( View.GONE ) ;
                        mLogoutView.setVisibility( View.VISIBLE );
                    } else if ( message.equals( "logged out" ) ) {
                        mLoginView.setVisibility( View.VISIBLE );
                        mLogoutView.setVisibility( View.GONE );
                    }
                }
            });
        }
    }

    private void setupGUI() {

        // Login button
        mLoginView = mActivity.findViewById( R.id.login );
        ImageButton loginButton = mActivity.findViewById( R.id.loginButton );
        loginButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                if ( isConnected() ) {
                    // Use selected LWA method
                    boolean lwaWithBrowser =
                            ((ToggleButton) mActivity.findViewById(R.id.toggleLwaMode)).isChecked();
                    // cancel pending cbl auth
                    mLwaCBL.cancelPendingAuthorization();
                    if ( lwaWithBrowser ) {
                        mLwaBrowser.login();
                    } else {
                        mLwaCBL.login();
                    }
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
        } );

        // Logout button
        mLogoutView = mActivity.findViewById( R.id.logout );
        mActivity.findViewById( R.id.logoutButton ).setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick( View v ) { logout(); }
            }
        );

        mLwaCBL.addObserver( this );
        mLwaBrowser.addObserver( this );
    }

    private boolean isConnected() {
        return ( mNetworkInfoProviderHandler.getNetworkStatus().equals( NetworkInfoProvider.NetworkStatus.CONNECTED ) );
    }
}
