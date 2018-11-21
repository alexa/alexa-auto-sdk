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
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Observable;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

import javax.net.ssl.HttpsURLConnection;

class LoginWithAmazonCBL extends Observable {

    private static final String sTag = "CBL";

    private static final int sResponseOk = 200;

    // Refresh access token 2 minutes before it expires
    private static final int sRefreshAccessTokenTime = 120000;

    // Poll every 10 seconds when requesting device token
    private static final int sPollInterval = 10;

    // CBL auth endpoint URLs
    private static final String sBaseEndpointUrl = "https://api.amazon.com/auth/O2/";
    private static final String sAuthRequestUrl = LoginWithAmazonCBL.sBaseEndpointUrl + "create/codepair";
    private static final String sTokenRequestUrl = LoginWithAmazonCBL.sBaseEndpointUrl + "token";
    private static final String sTokenVerificationRequestUrl = LoginWithAmazonCBL.sBaseEndpointUrl + "tokeninfo?access_token=";
    private static final String sProfileRequestUrl = "https://api.amazon.com/user/profile";

    //    To fetch User Profile data, set the sUserProfileEnabled to true
    //    You will need additional parameters in your Security Profile for the profile scope request to succeed,
    //    please see the README CBL section for more.
    private static final boolean sUserProfileEnabled = false;
    private static final String sScopeValue = sUserProfileEnabled ? "alexa:all+profile" : "alexa:all";

    // default client id regexpr <>
    private static String sDefaultRegExpr = "^<[^>]*>$";

    private final SharedPreferences mPreferences;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AuthProviderHandler mAuthProvider;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private String mClientId;
    private String mClientSecret;
    private String mProductID;
    private String mProductDSN;

    private Timer mTimer = new Timer();
    private TimerTask mAuthorizationTimerTask;
    private TimerTask mRefreshTimerTask;

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
                if ( !Pattern.matches( sDefaultRegExpr, mClientId ) ) {
                    final JSONObject scopeData = new JSONObject();
                    final JSONObject data = new JSONObject();
                    final JSONObject productInstanceAttributes = new JSONObject();

                    productInstanceAttributes.put( "deviceSerialNumber", mProductDSN );
                    data.put( "productInstanceAttributes", productInstanceAttributes );
                    data.put( "productID", mProductID );
                    scopeData.put( "alexa:all", data );

                    final String urlParameters = "response_type=device_code"
                            + "&client_id=" + mClientId
                            + "&scope=" + sScopeValue
                            + "&scope_data=" + scopeData.toString();

                    HttpsURLConnection con = null;
                    DataOutputStream os = null;
                    InputStream response = null;

                    try {
                        URL obj = new URL( sAuthRequestUrl );
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

                } else mLogger.postWarn( sTag, "Cannot authenticate. Please review the configuration file in the app's assets directory." );

            } catch ( Exception e ) { mLogger.postError( sTag, e.getMessage() ); }
        }
    }

    private void requestDeviceToken( JSONObject response ) {
        try {
            final String deviceCode = response.getString( "device_code" );
            final String userCode = response.getString( "user_code" );
            final String expirySeconds = response.getString( "expires_in" );
            final String urlParameters = "grant_type=device_code"
                    + "&device_code=" + deviceCode
                    + "&user_code=" + userCode;

            mTimer.schedule( mAuthorizationTimerTask = new TimerTask() {
                int i = ( Integer.parseInt( expirySeconds ) ) / sPollInterval;
                public void run() {
                    if ( i > 0 ) {
                        HttpsURLConnection con = null;
                        DataOutputStream os = null;
                        BufferedReader in = null;
                        try {
                            URL obj = new URL( sTokenRequestUrl );
                            con = ( HttpsURLConnection ) obj.openConnection();

                            con.setRequestMethod( "POST" );
                            con.setRequestProperty( "Host", "api.amazon.com" );
                            con.setRequestProperty( "Content-Type", "application/x-www-form-urlencoded" );

                            con.setDoOutput( true );

                            os = new DataOutputStream( con.getOutputStream() );
                            os.writeBytes( urlParameters );

                            int responseCode = con.getResponseCode();
                            if ( responseCode == sResponseOk ) {
                                this.cancel();
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
                                editor.putString( mActivity.getString( R.string.preference_login_method ), LoginWithAmazon.CBL_LOGIN_METHOD_KEY );
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

                                // Fetch User Profile if profile scope was authorized
                                if ( sScopeValue.contains( "profile" ) ) {
                                    requestUserProfile( accessToken );
                                }
                            }

                        } catch ( Exception e ) {
                            this.cancel();
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
                        this.cancel();
                        // Prompt to attempt authorization again
                        String expiredMessage = "The code has expired. Retry to generate a new code.";
                        try {
                            // Log code expired card
                            JSONObject renderJSON = new JSONObject();
                            renderJSON.put( "message", expiredMessage );
                            mLogger.postDisplayCard( renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED );
                        } catch ( JSONException e ) {
                            mLogger.postError( sTag, "JSON Error: " + e.getMessage() );
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

    private void requestUserProfile( final String accessToken ) {
        HttpsURLConnection urlConnection = null;
        try {
            // token authenticity verification
            URL requestUrl = new URL( sTokenVerificationRequestUrl + URLEncoder.encode(accessToken, "UTF-8" ) );

            urlConnection = ( HttpsURLConnection ) requestUrl.openConnection();
            urlConnection.setRequestMethod( "GET" );
            urlConnection.setRequestProperty( "Host", "api.amazon.com" );
            urlConnection.setRequestProperty( "access_token", URLEncoder.encode( accessToken, "UTF-8" ) );

            int responseCode = urlConnection.getResponseCode();

            if( responseCode == HttpURLConnection.HTTP_OK ) {
                JSONObject responseJSON = getResponseJSON( urlConnection.getInputStream() );
                urlConnection.disconnect();
                if ( responseJSON == null ) {
                    mLogger.postError( sTag, "Error requesting Token Info. Error: Null JSON Response" );
                } else {
                    if (!mClientId.equals( responseJSON.getString("aud") ) ) {
                        // the access token does not belong to us
                        mLogger.postError( sTag, "Error requesting Token Info. Error: Invalid access token");
                    } else {
                        try {
                            requestUrl = new URL(sProfileRequestUrl);
                            urlConnection = (HttpsURLConnection) requestUrl.openConnection();
                            urlConnection.setRequestMethod("GET");
                            urlConnection.setRequestProperty("Host", "api.amazon.com");
                            urlConnection.setRequestProperty("Authorization", "bearer " + accessToken);

                            responseCode = urlConnection.getResponseCode();

                            if (responseCode == HttpURLConnection.HTTP_OK) {
                                try {
                                    responseJSON = getResponseJSON(urlConnection.getInputStream());
                                    if ( responseJSON == null ) {
                                        mLogger.postError(sTag, "Error requesting User Profile. Error: null JSON Response");
                                    } else {
                                        mLogger.postInfo(sTag, String.format("USER PROFILE: Name: %s, Email: %s, User ID: %s",
                                                responseJSON.getString("name"),
                                                responseJSON.getString("email"),
                                                responseJSON.getString("user_id")));
                                    }

                                } catch (Exception e) {
                                    mLogger.postError(sTag, "Error requesting User Profile. Error: " + e.getMessage());
                                }
                            } else
                                mLogger.postInfo(sTag, "User Profile request failed with code: " + responseCode);
                            if (urlConnection != null) {
                                urlConnection.disconnect();
                            }
                        } catch (Exception e) {
                            mLogger.postError(sTag, "Error requesting Token Info. Error: " + e.getMessage());
                        }
                    }
                }
            } else mLogger.postInfo( sTag, "Token Info request failed with code: " + responseCode);
        } catch( Exception e ) {
            mLogger.postError( sTag, "Error while requesting User Profile. Error: " + e.getMessage() );
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
                    URL obj = new URL( sTokenRequestUrl );
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

                } else {
                    mAuthProvider.clearAuthToken();
                    mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                            AuthProvider.AuthError.AUTHORIZATION_FAILED );
                    mLogger.postError( sTag, "Error refreshing auth token" );
                }

            } else mLogger.postWarn( sTag, String.format(
                "Invalid Auth Parameters, clientID: %s, clientSecret: %s, refreshToken: %s",
                    mClientId, mClientSecret, mRefreshToken ) );
        }
    }

    private void startRefreshTimer( Long delaySeconds, final String refreshToken ) {
        mTimer.schedule( mRefreshTimerTask = new TimerTask() {
            public void run() { refreshAuthToken( refreshToken ); }
        }, delaySeconds * 1000 - sRefreshAccessTokenTime );
    }

    void logout() {
        mLogger.postInfo( sTag, "Attempting to un-authenticate" );

        // stop refresh timer task
        if ( mRefreshTimerTask != null ) mRefreshTimerTask.cancel();

        // Clear refresh token in preferences
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( mActivity.getString( R.string.preference_refresh_token ), "" );
        editor.putString( mActivity.getString( R.string.preference_login_method ), "" );
        editor.apply();

        // Notify AuthProvider of unauthenticated state
        mAuthProvider.clearAuthToken();
        mAuthProvider.onAuthStateChanged( AuthProvider.AuthState.UNINITIALIZED,
                AuthProvider.AuthError.NO_ERROR );

        // Notify observers to update GUI
        setChanged();
        notifyObservers( "logged out" );
    }

    void onInitialize() {
        String refreshToken = mPreferences.getString( mActivity.getString( R.string.preference_refresh_token ), "" );
        if ( !refreshToken.equals( "" ) ) refreshAuthToken( refreshToken );
    }

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

    void cancelPendingAuthorization() {
        if ( mAuthorizationTimerTask != null ) {
            mAuthorizationTimerTask.cancel();
        }
    }
}
