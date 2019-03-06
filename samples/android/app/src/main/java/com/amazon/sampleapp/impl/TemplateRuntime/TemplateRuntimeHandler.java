/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.TemplateRuntime;

import android.support.annotation.Nullable;

import com.amazon.aace.alexa.ExternalMediaAdapter;
import com.amazon.aace.alexa.TemplateRuntime;
import com.amazon.sampleapp.impl.ExternalMediaPlayer.MACCPlayer;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class TemplateRuntimeHandler extends TemplateRuntime {

    private static final String sTag = "TemplateRuntime";

    private final LoggerHandler mLogger;
    private final PlaybackControllerHandler mPlaybackController;
    private String mCurrentAudioItemId;

    public TemplateRuntimeHandler( LoggerHandler logger,
                                   @Nullable PlaybackControllerHandler playbackController ) {
        mLogger = logger;
        mPlaybackController = playbackController;
    }

    @Override
    public void renderTemplate( String payload ) {
        try {
            // Log payload
            JSONObject template = new JSONObject( payload );
            mLogger.postJSONTemplate( sTag, template.toString( 4 ) );

            // Log card
            String type = template.getString( "type" );
            switch ( type ) {
                case "BodyTemplate1":
                    mLogger.postDisplayCard( template, LogRecyclerViewAdapter.BODY_TEMPLATE1 );
                    break;
                case "BodyTemplate2":
                    mLogger.postDisplayCard( template, LogRecyclerViewAdapter.BODY_TEMPLATE2 );
                    break;
                case "ListTemplate1":
                    mLogger.postDisplayCard( template, LogRecyclerViewAdapter.LIST_TEMPLATE1 );
                    break;
                case "WeatherTemplate":
                    mLogger.postDisplayCard( template, LogRecyclerViewAdapter.WEATHER_TEMPLATE );
                    break;
                case "LocalSearchListTemplate1":
                    mLogger.postDisplayCard( template, LogRecyclerViewAdapter.LOCAL_SEARCH_LIST_TEMPLATE1 );
                    break;
                default:
                    break;
            }
        } catch ( JSONException e ) {
            mLogger.postError( sTag, e.getMessage() );
        }
    }

    @Override
    public void renderPlayerInfo( String payload ) {
        try {
            JSONObject playerInfo = new JSONObject( payload );
            String audioItemId = playerInfo.getString( "audioItemId" );
            JSONObject content = playerInfo.getJSONObject( "content" );
            JSONObject audioProvider = content.getJSONObject( "provider" );
            String providerName = audioProvider.getString( "name" );

            // Update playback controller buttons and player info labels
            if ( mPlaybackController != null ) {
                //reset visual state
                mPlaybackController.hidePlayerInfoControls();
                JSONArray controls = playerInfo.getJSONArray("controls" );
                for ( int j = 0; j < controls.length(); j++ ) {
                    JSONObject control = controls.getJSONObject( j );
                    if ( control.getString( "type" ).equals( "BUTTON" ) ) {
                        final boolean enabled = control.getBoolean( "enabled" );
                        final String name = control.getString( "name" );
                        mPlaybackController.updateControlButton( name, enabled );
                    } else if ( control.getString( "type" ).equals( "TOGGLE" ) ) {
                        final boolean selected = control.getBoolean( "selected" );
                        final boolean enabled = control.getBoolean( "enabled" );
                        final String name = control.getString( "name" );
                        mPlaybackController.updateControlToggle( name, enabled, selected );
                    }
                }

                String title = content.has( "title" ) ? content.getString( "title" ) : "";
                String artist = content.has( "titleSubtext1" ) ? content.getString( "titleSubtext1" ) : "";
                JSONObject provider = content.getJSONObject( "provider" );
                String name = provider.has( "name" ) ? provider.getString( "name" ) : "";
                mPlaybackController.setPlayerInfo( title, artist, name );
            }

            // Log only if audio item has changed
            if ( !audioItemId.equals( mCurrentAudioItemId ) ) {
                mCurrentAudioItemId = audioItemId;

                // Log payload
                mLogger.postJSONTemplate( sTag, playerInfo.toString( 4 ) );

                // Log card
                mLogger.postDisplayCard( content, LogRecyclerViewAdapter.RENDER_PLAYER_INFO );

            } else {
                mLogger.postJSONTemplate( sTag, playerInfo.toString( 4 ) );
            }
        } catch ( JSONException e ) {
            mLogger.postError( sTag, e.getMessage() );
        }
    }

    @Override
    public void clearTemplate() {
        // Handle dismissing display card here
        mLogger.postInfo( sTag, "handle clearTemplate()" );
    }

    @Override
    public void clearPlayerInfo() {
        mLogger.postInfo( sTag, "handle clearPlayerInfo()" );
        if ( mPlaybackController != null && !mPlaybackController.getProvider().equals(MACCPlayer.SPOTIFY_PROVIDER_NAME) ) {
            mPlaybackController.setPlayerInfo("", "", "");
            mPlaybackController.hidePlayerInfoControls();
        }
    }
}
