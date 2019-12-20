/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.CBL;

import android.app.Activity;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;

import com.amazon.aace.cbl.CBL;
import com.amazon.aace.alexa.AlexaClient.AuthState;
import com.amazon.aace.alexa.AlexaClient.AuthError;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;
import com.amazon.sampleapp.impl.AlexaClient.AuthStateObserver;

import org.json.JSONException;
import org.json.JSONObject;

public class CBLHandler extends CBL implements AuthStateObserver {

    private static final String sTag = "CBL";

    private final LoggerHandler mLogger;
    private final Activity  mActivity;
    private final Handler mMainThreadHandler;
    private final SharedPreferences mPreferences;

    private View mLoginView, mCancelView, mLogoutView;

    public CBLHandler( Activity activity, LoggerHandler logger ) {
        mActivity = activity;
        mLogger = logger;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
        mPreferences = activity.getSharedPreferences( activity.getString( R.string.preference_file_key ), Context.MODE_PRIVATE );
        setupUI();
    }

    @Override
    public void cblStateChanged( CBLState state, CBLStateChangedReason reason, String url, String code ) {
        mLogger.postInfo( sTag, String.format( "cblStateChanged:state=%s,reason=%s,url=%s code=%s", state.name(), reason.name(), url, code ) );

        switch ( state ) {
            case CODE_PAIR_RECEIVED:
                try {
                    JSONObject renderJSON = new JSONObject();
                    renderJSON.put( "verification_uri", url );
                    renderJSON.put( "user_code", code );
                    mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE );
                } catch ( Exception e ) {
                    mLogger.postError( sTag, e.getMessage() );
                }
                break;
            case STARTING:
                showCancelButton();
                break;
            case STOPPING:
                switch ( reason ) {
                    case CODE_PAIR_EXPIRED:
                        showLoginButton();
                        try {
                            JSONObject renderJSON = new JSONObject();
                            String expiredMessage = "The code has expired. Retry to generate a new code.";
                            renderJSON.put( "message", expiredMessage );
                            mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED );
                        } catch ( JSONException e ) {
                            mLogger.postError( sTag, e.getMessage() );
                        }
                        break;
                    case AUTHORIZATION_EXPIRED:
                        try {
                            JSONObject renderJSON = new JSONObject();
                            String expiredMessage = "The token has expired. Log in again.";
                            renderJSON.put( "message", expiredMessage );
                            mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED );
                        } catch ( JSONException e ) {
                            mLogger.postError( sTag, e.getMessage() );
                        }
                        break;
                    case NONE:
                        // CBL stopped using cancel button
                        showLoginButton();
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    @Override
    public void clearRefreshToken() {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( mActivity.getString( R.string.preference_refresh_token ), "" );
        editor.apply();
    }

    @Override
    public void setRefreshToken( String refreshToken ) {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( mActivity.getString( R.string.preference_refresh_token ), refreshToken );
        editor.apply();
    }

    @Override
    public String getRefreshToken() {
        return mPreferences.getString( mActivity.getString( R.string.preference_refresh_token ), "" );
    }

    @Override
    public void setUserProfile( String name, String email ) {
        mLogger.postInfo( sTag, "User profile details updated." );
    }

    @Override
    public void onAuthStateChanged( final AuthState authState, final AuthError authError ) {
        mMainThreadHandler.post( new Runnable() {
            @Override
            public void run() {
                if ( authState == AuthState.REFRESHED ) {
                    showLogoutButton();
                } else {
                    showLoginButton();
                }
            }
        } );
    }

    private void setupUI() {
        mLoginView = mActivity.findViewById( R.id.login );
        mLoginView.findViewById( R.id.loginButton ).setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                mLogger.postInfo( sTag, "Starting CBL login flow..." );
                start();
            }
        } );

        mCancelView = mActivity.findViewById( R.id.cancel );
        mCancelView.findViewById( R.id.cancelButton ).setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                mLogger.postInfo( sTag, "Cancelling CBL login flow..." );
                cancel();
            }
        } );

        mLogoutView = mActivity.findViewById( R.id.logout );
        mLogoutView.findViewById( R.id.logoutButton ).setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                mLogger.postInfo( sTag, "Resetting CBL..." );
                reset();
            }
        } );
    }

    private void showLoginButton() {
        mMainThreadHandler.post( new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility( View.VISIBLE );
                mCancelView.setVisibility( View.GONE );
                mLogoutView.setVisibility( View.GONE );
            }
        } );
    }

    private void showCancelButton() {
        mMainThreadHandler.post( new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility( View.GONE ) ;
                mCancelView.setVisibility( View.VISIBLE );
                mLogoutView.setVisibility( View.GONE );
            }
        } );
    }

    private void showLogoutButton() {
        mMainThreadHandler.post( new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility( View.GONE ) ;
                mCancelView.setVisibility( View.GONE );
                mLogoutView.setVisibility( View.VISIBLE );
            }
        } );
    }
}
