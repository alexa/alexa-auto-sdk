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
import android.support.v4.content.ContextCompat;

import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Notifications;
import com.amazon.aace.alexa.Speaker;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;

public class NotificationsHandler extends Notifications
{

    private LoggerHandler m_logger;
    private int m_logColor;

    public NotificationsHandler( Context context, LoggerHandler logger, MediaPlayer mediaPlayer, Speaker speaker )
    {
        super( mediaPlayer, speaker );
        m_logger = logger;
        m_logColor = ContextCompat.getColor( context, R.color.logTextStatus ) & 0x00FFFFFF;
    }

    public NotificationsHandler( Context context, LoggerHandler logger, AndroidMediaPlayer mediaPlayer )
    {
        this( context, logger, mediaPlayer, mediaPlayer.getSpeaker() );
    }

    @Override
    public void setIndicator( IndicatorState state )
    {
        m_logger.post( String.format( "Notification IndicatorState: %s", state ), Logger.Level.INFO, m_logColor );
    }
}
