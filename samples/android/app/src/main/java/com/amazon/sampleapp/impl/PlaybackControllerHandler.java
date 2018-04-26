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
import android.widget.ImageButton;

import com.amazon.aace.alexa.PlaybackController;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;

public class PlaybackControllerHandler extends PlaybackController
{
    private LoggerHandler mLogger = null;
    private ImageButton mControlPrev;
    private ImageButton mControlPlayPause;
    private ImageButton mControlNext;
    private int mLogColor;

    public PlaybackControllerHandler( Context context, LoggerHandler logger )
    {
        mLogger = logger;
        mLogColor = ContextCompat.getColor( context, R.color.logTextStatus ) & 0x00FFFFFF;
    }

    @Override
    public void previousButtonPressed()
    {
        super.previousButtonPressed();
        mLogger.post( "PlaybackController previousButtonPressed", Logger.Level.INFO, mLogColor );
    }

    @Override
    public void playButtonPressed()
    {
        super.playButtonPressed();
        mLogger.post( "PlaybackController playButtonPressed", Logger.Level.INFO, mLogColor );
    }

    @Override
    public void pauseButtonPressed()
    {
        super.pauseButtonPressed();
        mLogger.post( "PlaybackController pauseButtonPressed", Logger.Level.INFO, mLogColor );
    }

    @Override
    public void nextButtonPressed()
    {
        super.nextButtonPressed();
        mLogger.post( "PlaybackController nextButtonPressed", Logger.Level.INFO, mLogColor );
    }

    public void setControls( ImageButton controlPrev, ImageButton controlPlayPause, ImageButton controlNext )
    {
        mControlPrev = controlPrev;
        mControlPlayPause = controlPlayPause;
        mControlNext = controlNext;
    }

    public ImageButton getControlPrev() { return mControlPrev; }

    public ImageButton getControlPlayPause() { return mControlPlayPause; }

    public ImageButton getControlNext() { return mControlNext; }


}
