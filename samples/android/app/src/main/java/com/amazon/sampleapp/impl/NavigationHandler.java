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

import com.amazon.aace.logger.Logger;
import com.amazon.aace.navigation.Navigation;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.view.LogEntry;

import org.json.JSONObject;

import java.util.Observable;
import java.util.Observer;

public class NavigationHandler extends Navigation
{
    private LoggerHandler mLogger = null;
    private int mLogColor;
    static NavigationObservable sNavigationObservable = null;

    public NavigationHandler( Context context, LoggerHandler logger )
    {
        mLogger = logger;
        mLogColor = ContextCompat.getColor( context, R.color.logTextTemplate ) & 0x00FFFFFF;
    }

    @Override
    public boolean setDestination( String payload )
    {
        try
        {
            // Log payload
            JSONObject template = new JSONObject( payload );
            mLogger.post( template.toString( 4 ), Logger.Level.INFO, mLogColor );

            // Log display card
            JSONObject json = new JSONObject();
            json.put( "template", template );
            json.put( "source", LoggerHandler.Source.CLI.toString() );
            json.put( "level", Logger.Level.INFO.toString() );
            sNavigationObservable.log( "setDestinationTemplate", json );
            return true;
        }
        catch( Throwable ex )
        {
            ex.printStackTrace();
            return false;
        }
    }

    //
    // NavigationObservable
    //
    static public void addNavigationObserver( Observer observer )
    {
        if( sNavigationObservable == null ) {
            sNavigationObservable = new NavigationHandler.NavigationObservable();
        }

        sNavigationObservable.addObserver( observer );
    }

    static private class NavigationObservable extends Observable
    {
        public void log( String message ) {
            setChanged();
            notifyObservers( message );
        }

        public void log( String type, JSONObject obj )
        {
            setChanged();
            notifyObservers( new LogEntry( type, obj ) );
        }
    }
}