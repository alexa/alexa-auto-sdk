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

import com.amazon.aace.alexa.TemplateRuntime;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.view.LogEntry;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.Observable;
import java.util.Observer;

public class TemplateRuntimeHandler extends TemplateRuntime
{
    private LoggerHandler mLogger = null;
    private PlaybackControllerHandler mPlaybackController;
    private String mCurrentAudioItemId = null;
    private int mLogColor;
    static TemplateObservable sTemplateObservable = null;

    public TemplateRuntimeHandler( Context context, LoggerHandler logger, PlaybackControllerHandler playbackController )
    {
        mLogger = logger;
        mPlaybackController = playbackController;
        mLogColor = ContextCompat.getColor( context, R.color.logTextTemplate ) & 0x00FFFFFF;
    }

    @Override
    public void renderTemplate( String payload )
    {
        try
        {
            // Log payload
            JSONObject template = new JSONObject( payload );
            String type = template.getString( "type" );
            mLogger.post( template.toString( 4 ), Logger.Level.INFO, mLogColor );

            // Log card
            JSONObject json = new JSONObject();
            json.put( "template", template );
            json.put( "source", LoggerHandler.Source.CLI.toString() );
            json.put( "level", Logger.Level.INFO.toString() );

            switch( type )
            {
                case "BodyTemplate1":
                    sTemplateObservable.log( "bodyTemplate1", json );
                    break;
                case "BodyTemplate2":
                    sTemplateObservable.log( "bodyTemplate2", json );
                    break;
                case "ListTemplate1":
                    sTemplateObservable.log( "listTemplate1", json );
                    break;
                case "WeatherTemplate":
                    sTemplateObservable.log( "weatherTemplate", json );
                    break;
                case "LocalSearchListTemplate1":
                    sTemplateObservable.log( "localSearchListTemplate1", json );
                    break;
                default:
                    String message = renderTemplate_defaultTemplate( template );
                    mLogger.post( message, Logger.Level.INFO, mLogColor );
                    break;
            }
        } catch( Throwable ex )
        {
            ex.printStackTrace();
        }
    }

    @Override
    public void renderPlayerInfo( String payload )
    {
        try
        {
            JSONObject playerInfo = new JSONObject( payload );
            String audioItemId = playerInfo.getString( "audioItemId" );

            // Update playback controller buttons
            if( mPlaybackController != null )
            {
                JSONArray controls = playerInfo.getJSONArray("controls" );
                for ( int j = 0; j < controls.length(); j++ )
                {
                    JSONObject control = controls.getJSONObject( j );
                    if ( control.getString( "type" ).equals( "BUTTON" ) )
                    {
                        boolean enabled = control.getBoolean( "enabled" );
                        switch ( control.getString( "name" ) )
                        {
                            case "PREVIOUS":
                                mPlaybackController.getControlPrev().setEnabled( enabled );
                                break;
                            case "PLAY_PAUSE":
                                mPlaybackController.getControlPlayPause().setEnabled( enabled );
                                break;
                            case "NEXT":
                                mPlaybackController.getControlNext().setEnabled( enabled );
                                break;
                        }
                    }
                }
            }

            // Log only if audio item has changed
            if( !audioItemId.equals( mCurrentAudioItemId ) )
            {
                mCurrentAudioItemId = audioItemId;

                // Log payload
                mLogger.post( playerInfo.toString( 4 ), Logger.Level.INFO, mLogColor );

                // Log card
                JSONObject content = playerInfo.getJSONObject( "content" );
                JSONObject json = new JSONObject();
                json.put( "template", content );
                json.put( "source", LoggerHandler.Source.CLI.toString() );
                json.put( "level", Logger.Level.INFO.toString() );
                sTemplateObservable.log( "renderPlayerInfo", json );
            }
        } catch( Throwable ex )
        {
            ex.printStackTrace();
        }
    }

    private String renderTemplate_defaultTemplate( JSONObject template )
    {
        StringBuilder builder = new StringBuilder();

        try
        {
            JSONObject title = template.has( "title" ) ? template.getJSONObject( "title" ) : null;
            JSONArray listItems = template.has( "listItems" ) ? template.getJSONArray( "listItems" ) : null;
            String textField = template.has( "textField" ) ? template.getString( "textField" ) : null;

            if( title != null )
            {
                if( title.has( "mainTitle" ) )
                {
                    builder.append( title.getString( "mainTitle" ) );
                }

                if( title.has( "subTitle" ) )
                {
                    builder.append( title.getString( "subTitle" ) );
                }
            } else {
                builder.append(template.getString( "type" ) );
            }

            if( textField != null )
            {
                builder.append( template.getString( "textField" ) );
            }

            if( listItems != null )
            {
                for( int j = 0; j < listItems.length(); j++ )
                {
                    JSONObject next = listItems.getJSONObject( j );
                    builder.append( String.format( "%s %s",next.getString( "leftTextField" ),
                            next.getString( "rightTextField" ) ) );
                }
            }
        } catch( Throwable ex )
        {
            ex.printStackTrace();
        }

        return builder.toString();
    }

    //
    // TemplateObservable
    //
    static public void addTemplateObserver( Observer observer )
    {
        if( sTemplateObservable == null )
        {
            sTemplateObservable = new TemplateObservable();
        }

        sTemplateObservable.addObserver( observer );
    }

    static public void removeTemplateObserver( Observer observer )
    {
        if( sTemplateObservable != null )
        {
            sTemplateObservable.deleteObserver( observer );

            if( sTemplateObservable.countObservers() == 0 )
            {
                sTemplateObservable = null;
            }
        }
    }

    static private class TemplateObservable extends Observable
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
}