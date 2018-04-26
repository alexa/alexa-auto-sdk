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

package com.amazon.sampleapp.impl;

import android.content.Context;
import android.graphics.Color;
import android.support.v4.content.ContextCompat;

import com.amazon.aace.alexa.AuthProvider;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.view.LogEntry;

import org.json.JSONObject;

import java.util.Observable;
import java.util.Observer;


public class AuthProviderHandler extends AuthProvider {

    private LoggerHandler m_logger = null;
    public AuthState authState = AuthState.UNINITIALIZED;

    public String authToken = "";
    private boolean m_authTokenSet = false;

    public AuthProviderHandler(Context context, LoggerHandler logger ) {
        m_logger = logger;
    }

    /**
     * Called when the platform implementation should retrieve the an auth token
     *
     * @return the auth token from the platform implementation of AuthProvider.
     */
    @Override
    public String getAuthToken() {
        if(authToken == ""){
            if(m_authTokenSet == true)
            {
                m_logger.post( "GET AUTH TOKEN: NOT SET", Logger.Level.WARN, Color.YELLOW);
                m_authTokenSet = false;
            }

        } else if(m_authTokenSet == false)
        {
            m_logger.post("GET AUTH TOKEN: SET", Logger.Level.INFO, Color.GREEN);
            m_authTokenSet = true;
        }
        return authToken;
    }

    /**
     * Called when the platform implementation should retrieve an authState
     *
     * @return the auth state from the platform implementation of AuthProvider.
     */
    @Override
    public AuthState getAuthState() {
        m_logger.post("GET AUTHSTATE:" + authState.toString(), Logger.Level.INFO, Color.GREEN);
        return authState;
    }

}
