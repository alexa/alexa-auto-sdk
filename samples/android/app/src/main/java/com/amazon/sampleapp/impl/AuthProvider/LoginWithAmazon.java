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
import android.widget.TextView;
import android.widget.ToggleButton;

import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.Observable;
import java.util.Observer;

class LoginWithAmazon implements Observer {

    private final LoginWithAmazonBrowser mLwaBrowser;
    private final LoginWithAmazonCBL mLwaCBL;
    private final SharedPreferences mPreferences;
    private final Activity mActivity;

    private View mLoginView, mLogoutView;

    LoginWithAmazon( LoggerHandler logger,
                            Activity activity,
                            AuthProviderHandler authProvider) {
        mActivity = activity;
        mPreferences = activity.getSharedPreferences(
                activity.getString( R.string.preference_file_key ), Context.MODE_PRIVATE );
        mLwaBrowser = new LoginWithAmazonBrowser( activity, mPreferences, logger, authProvider );
        mLwaCBL = new LoginWithAmazonCBL( mActivity, mPreferences, logger, authProvider );

        setupGUI();
    }

    void onResume() {
        // Refresh auth token with CBL if refreshToken stored in preferences, else with LWA browser
        String refreshToken = mPreferences.getString( mActivity.getString( R.string.preference_refresh_token ), "" );
        if ( !refreshToken.equals( "" ) ) mLwaCBL.onResume( refreshToken );
        else mLwaBrowser.onResume();
    }

    private void logout() {
        // Log out with CBL if refreshToken stored in preferences, else with LWA browser
        String refreshToken = mPreferences.getString( mActivity.getString( R.string.preference_refresh_token ), "" );
        if ( !refreshToken.equals( "" ) ) mLwaCBL.logout();
        else mLwaBrowser.logout();
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
                if ( lwaWithBrowser ) mLwaBrowser.login();
                else mLwaCBL.login();
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
