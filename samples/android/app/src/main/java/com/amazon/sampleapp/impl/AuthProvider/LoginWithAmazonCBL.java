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
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Observable;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.net.ssl.HttpsURLConnection;

class LoginWithAmazonCBL extends Observable {

    private static final String sTag = "CBL";
    private static final int sResponseOk = 200;

    // Refresh access token 2 minutes before it expires
    private static final int sRefreshAccessTokenTime = 120000;

    // Poll every 10 seconds when requesting device token
    private static final int sPollInterval = 10;

    // Stop polling for device token within 30 seconds of its expiry time
    private static final int sStopPollingDeviceTokenTime = 30;

    // CBL auth endpoint URL's
    private static final String mBaseEndpointUrl = "https://api.amazon.com/auth/O2/";
    private static final String mAuthRequestUrl = mBaseEndpointUrl + "create/codepair";
    private static final String mTokenRequestUrl = mBaseEndpointUrl + "token";

    private final SharedPreferences mPreferences;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AuthProviderHandler mAuthProvider;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private Timer mRefreshTimer = new Timer();
    private String mClientId;
    private String mClientSecret;
    private String mProductID;
    private String mProductDSN;

    LoginWithAmazonCBL( Activity activity,
                        SharedPreferences preferences,
                        LoggerHandler logger,
                        AuthProviderHandler auth ) {
        mActivity = activity;
        mPreferences = preferences;
        mLogger = logger;
        mAuthProvider = auth;

        mClientId = mPreferences.getString( mActivity.getString( R.string.preference_client_id ), "" );
        mClientSecret = mPreferences.getString( mActivity.getString( R.string.preference_client_secret ), "" );
        mProductID = mPreferences.getString( mActivity.getString( R.string.preference_product_id ), "" );
        mProductDSN = mPreferences.getString( mActivity.getString( R.string.preference_product_dsn ), "" );
    }

    void login() {
        mLogger.postInfo( sTag, "Attempting to authenticate" );
        requestDeviceAuthorization();
    }

    private void requestDeviceAuthorization() {
        mExecutor.submit( new requestDeviceAuthorizationTask() );
    }

    private class requestDeviceAuthorizationTask implements Runnable {
        @Override
        public void run() {
            try {
                if ( !mClientId.equals( "" ) ) {
                    final JSONObject scopeData = new JSONObject();
                    final JSONObject scope = new JSONObject();
                    final JSONObject productInstanceAttributes = new JSONObject();

                    productInstanceAttributes.put( "deviceSerialNumber", mProductDSN );
                    scope.put( "productInstanceAttributes", productInstanceAttributes );
                    scope.put( "productID", mProductID );
                    scopeData.put( "alexa:all", scope );

                    final String urlParameters = "response_type=device_code"
                            + "&client_id=" + mClientId
                            + "&scope=alexa:all"
                            + "&scope_data=" + scopeData.toString();

                    HttpsURLConnection con = null;
                    DataOutputStream os = null;
                    InputStream response = null;

                    try {
                        URL obj = new URL( mAuthRequestUrl );
                        con = ( HttpsURLConnection ) obj.openConnection();
                        con.setRequestMethod( "POST" );

                        con.setDoOutput( true );
                        os = new DataOutputStream( con.getOutputStream() );
                        os.writeBytes( urlParameters );

                        int responseCode = con.getResponseCode();
                        if ( responseCode == sResponseOk ) response = con.getInputStream();

                    } catch ( IOException e ) {
                        mLogger.postError( sTag, e.getMessage() );
                    } finally {
                        if ( con != null ) con.disconnect();
                        if ( os != null ) {
                            try {
                                os.flush();
                                os.close();
                            } catch ( IOException e ) {
                                mLogger.postWarn( sTag, "Cannot close resource. Error: "
                                        + e.getMessage() );
                            }
                        }
                    }

                    JSONObject responseJSON = getResponseJSON( response );
                    if ( responseJSON != null ) {
                        String uri = responseJSON.getString( "verification_uri" );
                        String code = responseJSON.getString( "user_code" );

                        // Log card
                        JSONObject renderJSON = new JSONObject();
                        renderJSON.put( "verification_uri", uri );
                        renderJSON.put( "user_code", code );
                        mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE );

                        // Log response
                        mLogger.postInfo( sTag,
                                String.format( "Verification URI with user code: %s?cbl-code=%s",
                                uri, code ) );

                        requestDeviceToken( responseJSON );

                    } else mLogger.postError( sTag, "Error requesting device authorization" );

                } else mLogger.postWarn( sTag, "Cannot authenticate. Invalid client ID" );

            } catch ( Exception e ) { mLogger.postError( sTag, e.getMessage() ); }
        }
    }

    private void requestDeviceToken( JSONObject response ) {
        try {
            final String deviceCode = response.getString( "device_code" );
            final String userCode = response.getString( "user_code" );
            final String expirySeconds = response.getString( "expires_in" );
            final String urlParameters = "grant_type=device_code"
                    +"&device_code=" + deviceCode
                    +"&user_code=" + userCode;

            final Timer timer = new Timer();
            timer.schedule( new TimerTask() {
                int i = ( Integer.parseInt( expirySeconds ) - sStopPollingDeviceTokenTime ) / sPollInterval;
                public void run() {
                    if ( i > 0 ) {
                        HttpsURLConnection con = null;
                        DataOutputStream os = null;
                        BufferedReader in = null;
                        try {
                            URL obj = new URL( mTokenRequestUrl );
                            con = ( HttpsURLConnection ) obj.openConnection();

                            con.setRequestMethod( "POST" );
                            con.setRequestProperty( "Host", "api.amazon.com" );
                            con.setRequestProperty( "Content-Type", "application/x-www-form-urlencoded" );

                            con.setDoOutput( true );

                            os = new DataOutputStream( con.getOutputStream() );
                            os.writeBytes( urlParameters );

                            int responseCode = con.getResponseCode();
                            if ( responseCode == sResponseOk ) {
                                timer.cancel();

                                in = new BufferedReader(
                                        new InputStreamReader( con.getInputStream() ) );
                                String inputLine;
                                StringBuilder response = new StringBuilder();

                                while ( ( inputLine = in.readLine() ) != null ) {
                                    response.append( inputLine );
                                }

                                JSONObject responseJSON = new JSONObject( response.toString() );
                                String accessToken = responseJSON.getString( "access_token" );
                                String refreshToken = responseJSON.getString( "refresh_token" );
                                String expiresInSeconds = responseJSON.getString( "expires_in" );

                                // Write refresh token to shared preferences
                                SharedPreferences.Editor editor = mPreferences.edit();
                                editor.putString( mActivity.getString( R.string.preference_refresh_token ), refreshToken );
                                editor.apply();

                                // Refresh access token automatically before expiry
                                startRefreshTimer( Long.parseLong( expiresInSeconds ), refreshToken );

                                // Inform AuthProvider of refreshed state
                                mLogger.postVerbose( sTag,
                                        "Refreshing Auth State with token: " + accessToken );
                                mAuthProvider.setAuthToken( accessToken );
                                mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.REFRESHED,
                                        AuthProvider.AuthError.NO_ERROR );
                                setChanged();
                                notifyObservers( "logged in" );
                            }

                        } catch ( Exception e ) {
                            timer.cancel();
                            mLogger.postError( sTag, e.getMessage() );
                            return;
                        } finally {
                            if ( con != null ) con.disconnect();
                            if ( os != null ) {
                                try {
                                    os.flush();
                                    os.close();
                                } catch ( IOException e ) {
                                    mLogger.postWarn( sTag, "Cannot close resource. Error: "
                                            + e.getMessage() );
                                }
                            }
                            if ( in != null ) {
                                try {
                                    in.close();
                                } catch ( IOException e ) {
                                    mLogger.postWarn( sTag, "Cannot close resource. Error: "
                                            + e.getMessage() );
                                }
                            }
                        }
                        i--;

                    } else { // User didn't authorize with code before it expired

                        timer.cancel();

                        // Prompt to attempt authorization again
                        String expiredMessage = "The code has expired. Retry to generate a new code.";
                        try {
                            // Log code expired card
                            JSONObject renderJSON = new JSONObject();
                            renderJSON.put( "message", expiredMessage );
                            mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED );
                        } catch ( JSONException e ) {
                            mLogger.postError( sTag, e.getMessage() );
                            return;
                        }
                        mLogger.postWarn( sTag, expiredMessage );
                    }
                }
            }, 0, sPollInterval * 1000 );

        } catch ( Exception e ) {
            mLogger.postError( sTag, "Error requesting device token. Error: " + e.getMessage() );
        }
    }

    private void refreshAuthToken( final String refreshToken ) {
        mExecutor.submit( new refreshAuthTokenTask( refreshToken ) );
    }

    private class refreshAuthTokenTask implements Runnable {
        String mRefreshToken = "";
        refreshAuthTokenTask( String refreshToken ) { mRefreshToken = refreshToken; }

        @Override
        public void run() {
            if ( !mRefreshToken.equals( "" )
                    && !mClientId.equals( "" ) && !mClientSecret.equals( "" ) ) {

                final String urlParameters = "grant_type=refresh_token"
                        + "&refresh_token=" + mRefreshToken
                        + "&client_id=" + mClientId
                        + "&client_secret=" + mClientSecret;
                    HttpsURLConnection con = null;
                    DataOutputStream os = null;
                    InputStream response = null;

                try {
                    URL obj = new URL( mTokenRequestUrl );
                    con = ( HttpsURLConnection ) obj.openConnection();
                    con.setRequestMethod( "POST" );

                    con.setDoOutput( true );
                    os = new DataOutputStream( con.getOutputStream() );
                    os.writeBytes( urlParameters );

                    int responseCode = con.getResponseCode();
                    if ( responseCode == sResponseOk ) response = con.getInputStream();

                } catch ( IOException e ) {
                    mLogger.postError( sTag, e.getMessage() );
                } finally {
                    if ( con != null ) con.disconnect();
                    if ( os != null ) {
                        try {
                            os.flush();
                            os.close();
                        } catch ( IOException e ) {
                            mLogger.postWarn( sTag, "Cannot close resource. Error: "
                                    + e.getMessage() );
                        }
                    }
                }

                JSONObject responseJSON = getResponseJSON( response );

                if ( responseJSON != null ) {
                    try {
                        String expiresInSeconds = responseJSON.getString( "expires_in" );
                        String accessToken = responseJSON.getString( "access_token" );

                        // Refresh access token automatically before expiry
                        startRefreshTimer( Long.parseLong( expiresInSeconds ), mRefreshToken );

                        // Inform Auth Provider of refreshed state
                        mLogger.postVerbose( sTag,
                                "Refreshing Auth State with token: " + accessToken );
                        mAuthProvider.setAuthToken( accessToken );
                        mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.REFRESHED,
                                AuthProvider.AuthError.NO_ERROR );
                        setChanged();
                        notifyObservers( "logged in" );

                    } catch ( JSONException e ) {
                        mLogger.postError( sTag, "Error refreshing auth token. Error: "
                                + e.getMessage() );
                    }

                } else  mLogger.postError( sTag, "Error refreshing auth token" );

            } else mLogger.postWarn( sTag, "Invalid client ID or client secret" );
        }
    }

    private void startRefreshTimer( Long delaySeconds, final String refreshToken ) {
        mRefreshTimer.schedule( new TimerTask() {
            public void run() { refreshAuthToken( refreshToken ); }
        }, delaySeconds * 1000 - sRefreshAccessTokenTime );
    }

    void logout() {
        mLogger.postInfo( sTag, "Attempting to un-authenticate" );

        // Clear refresh token in preferences
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( mActivity.getString( R.string.preference_refresh_token ), "" );
        editor.apply();

        // Notify AuthProvider of unauthenticated state
        mAuthProvider.clearAuthToken();
        mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                AuthProvider.AuthError.NO_ERROR );

        // Notify observers to update GUI
        setChanged();
        notifyObservers( "logged out" );
    }

    void onResume( String refreshToken ) { refreshAuthToken( refreshToken ); }

    private JSONObject getResponseJSON( InputStream inStream ) {
        if ( inStream != null ) {

            String inputLine;
            StringBuilder response = new StringBuilder();

            try ( BufferedReader in = new BufferedReader( new InputStreamReader( inStream ) ) ) {
                while ( ( inputLine = in.readLine() ) != null ) response.append( inputLine );
                return new JSONObject( response.toString() );
            } catch ( Exception e ) {
                mLogger.postError( sTag, e.getMessage() );
            } finally {
                try {
                    inStream.close();
                } catch ( IOException e ) {
                    mLogger.postWarn( sTag, "Cannot close resource. Error: "
                            + e.getMessage() );
                }
            }
        }
        return null;
    }
}
