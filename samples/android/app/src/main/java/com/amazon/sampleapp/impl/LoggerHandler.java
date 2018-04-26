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
import android.util.Log;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.view.LogEntry;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Observable;
import java.util.Observer;

import java.util.Calendar;

public class LoggerHandler extends Logger
{
    private Level m_level;
    private int m_logNormalColor;
    private int m_logWarningColor;
    private int m_logErrorColor;
    private boolean m_androidLogger = true;

    static LoggerObservable s_loggerObservable = null;
    static private SimpleDateFormat s_timeFormat = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.SSS" );

    public LoggerHandler( Context context ) {
        this( context, Level.INFO );
    }

    public LoggerHandler( Context context, Level level )
    {
        m_level = level;
        m_logNormalColor = ContextCompat.getColor( context, R.color.logTextNormal ) & 0x00FFFFFF;
        m_logWarningColor = ContextCompat.getColor( context, R.color.logTextWarning ) & 0x00FFFFFF;
        m_logErrorColor = ContextCompat.getColor( context, R.color.logTextError ) & 0x00FFFFFF;
    }

    // AVS / AAC log events
    @Override
    public boolean logEvent( Level level, long time, String source, String message )
    {
        if( level.ordinal() >= m_level.ordinal() )
        {
            int color;

            switch( level )
            {
                case WARN:
                    color = m_logWarningColor;
                    break;
                case ERROR:
                    color = m_logErrorColor;
                    break;
                default:
                    color = m_logNormalColor;
            }

            if( s_loggerObservable != null )
            {
                JSONObject json = new JSONObject();
                try
                {
                    json.put( "text", String.format( "%-25s [%s] %3c%n%s",
                            s_timeFormat.format( time ), source, level.toChar(), message ) );
                    json.put("textColor", color );
                    json.put( "source", source );
                    json.put( "level", level.toString() );
                } catch ( Throwable thr )
                {
                    consoleLog( Level.ERROR, Source.valueOf( source ), thr.getMessage() );
                }

                s_loggerObservable.log( "logText", json );
                consoleLog( level, Source.valueOf( source ), message );
            }
        }
        return false;
    }

    public void consoleLog( Level level, Source source, String message )
    {
        if( m_androidLogger )
        {
            switch( level )
            {
                case INFO:
                    Log.i( source.toString(), message );
                    break;
                case WARN:
                    Log.w( source.toString(), message );
                    break;
                case CRITICAL:
                case ERROR:
                    Log.e( source.toString(), message );
                    break;
                default:
                    Log.v( source.toString(), message );
                    break;
            }
        }
    }

    public void post( Throwable ex ) // default client error log
    {
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        PrintStream ps = new PrintStream( os );

        ex.printStackTrace( ps );

        String str = os.toString();

        if( s_loggerObservable != null )
        {
            post( str, Level.ERROR, m_logErrorColor );
        }
    }

    public void post( String message ) // default client info log
    {
        if( s_loggerObservable != null )
        {
            post( message, Level.INFO, m_logNormalColor );
        }
    }

    public void post( String message, Level level, int highlight ) // explicit client log
    {
        Date currentTime = Calendar.getInstance().getTime();

        if( s_loggerObservable != null )
        {
            JSONObject json = new JSONObject();
            try
            {
                json.put( "text", String.format( "%-25s [%s] %3c%n%s",
                        s_timeFormat.format( currentTime ), Source.CLI.toString(), level.toChar(), message ) );
                json.put( "textColor", highlight );
                json.put( "source", Source.CLI.toString() );
                json.put( "level", level.toString() );
            } catch ( Throwable thr )
            {
                Log.e( "ERROR", thr.getMessage() );
            }
            s_loggerObservable.log( "logText", json );
            consoleLog( level, Source.CLI, message );
        }
    }

    //
    // LoggerObservable
    //
    static public void addLogObserver( Observer observer )
    {
        if( s_loggerObservable == null )
        {
            s_loggerObservable = new LoggerObservable();
        }

        s_loggerObservable.addObserver( observer );
    }

    static public void removeLogObserver( Observer observer )
    {
        if( s_loggerObservable != null )
        {
            s_loggerObservable.deleteObserver( observer );

            if( s_loggerObservable.countObservers() == 0 )
            {
                s_loggerObservable = null;
            }
        }
    }

    static private class LoggerObservable extends Observable
    {
        public void log( String message )
        {
            setChanged();
            notifyObservers( message );
        }

        public void log( String type, JSONObject obj )
        {
            setChanged();
            notifyObservers( new LogEntry( type, obj ) );
        }
    }

    /**
     * Enum used to specify the source of a log message.
     */
    public enum Source
    {
        AVS("AVS"),

        AAC("AAC"),

        CLI("CLI");

        private String m_name;

        Source( String name ) {
            m_name = name;
        }

        public String toString() {
            return m_name;
        }
    }
}

