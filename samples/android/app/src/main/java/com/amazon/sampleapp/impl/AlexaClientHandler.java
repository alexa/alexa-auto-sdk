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

import com.amazon.aace.alexa.AlexaClient;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.view.LogEntry;

import org.json.JSONObject;

import java.util.Observable;
import java.util.Observer;

public class AlexaClientHandler extends AlexaClient
{
    private LoggerHandler m_logger = null;
    private int m_logColor;
    public ConnectionStatus m_connectionStatus = ConnectionStatus.DISCONNECTED;

    public AlexaClientHandler( Context context, LoggerHandler logger ) {
        m_logger = logger;
        m_logColor = ContextCompat.getColor( context, R.color.logTextStatus) & 0x00FFFFFF;
    }

    @Override
    public void dialogStateChanged( DialogState state ) {
        m_logger.post( String.format( "DialogState: %s", state ), Logger.Level.INFO, m_logColor );
    }

    @Override
    public void authStateChanged( AuthState state, AuthError error ) {

        if(error == AuthError.NO_ERROR) {
            m_logger.post( String.format( "AuthState: %s AuthError: %s", state, error ), Logger.Level.INFO, m_logColor);
        } else m_logger.post( String.format( "AuthState: %s AuthError: %s", state, error ), Logger.Level.ERROR, Color.RED);

    }

    @Override
    public void connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) {
        m_connectionStatus = status;
        m_logger.post( String.format( "ConnectionStatus: %s ConnectionChangedReason: %s", status, reason ),Logger.Level.INFO, m_logColor );
    }
}
