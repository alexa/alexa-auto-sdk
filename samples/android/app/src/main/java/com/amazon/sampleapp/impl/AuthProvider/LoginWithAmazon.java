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
import android.content.Context;
import android.content.SharedPreferences;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ToggleButton;

import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

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

    private View mLoginView, mLogoutView;

    LoginWithAmazon( LoggerHandler logger,
                            Activity activity,
                            AuthProviderHandler authProvider) {
        mActivity = activity;
        mLogger = logger;
        mPreferences = activity.getSharedPreferences(
                activity.getString( R.string.preference_file_key ), Context.MODE_PRIVATE );
        mLwaBrowser = new LoginWithAmazonBrowser( activity, mPreferences, logger, authProvider );
        mLwaCBL = new LoginWithAmazonCBL( mActivity, mPreferences, logger, authProvider );

        setupGUI();
    }

    void onInitialize() {
        String loginMethod = mPreferences.getString( mActivity.getString( R.string.preference_login_method ), "" );
        if ( loginMethod.equals( CBL_LOGIN_METHOD_KEY ) ) mLwaCBL.onInitialize();
        else if ( loginMethod.equals( LWA_LOGIN_METHOD_KEY ) ) mLwaBrowser.onInitialize();
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
                // Use selected LWA method
                boolean lwaWithBrowser =
                        ( ( ToggleButton ) mActivity.findViewById( R.id.toggleLwaMode ) ).isChecked();
                // cancel pending cbl auth
                mLwaCBL.cancelPendingAuthorization();
                if ( lwaWithBrowser ) {
                    mLwaBrowser.login();
                } else mLwaCBL.login();

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
}
