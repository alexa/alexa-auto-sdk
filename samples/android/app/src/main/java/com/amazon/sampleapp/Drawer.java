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

package com.amazon.sampleapp;

import android.app.Activity;
import android.content.Context;
import android.support.v7.widget.SwitchCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.impl.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackControllerHandler;
import com.amazon.sampleapp.view.LogRecyclerViewAdapter;

import java.util.Observable;
import java.util.Observer;

public class Drawer implements Observer
{
    private LinearLayout mDrawerLayout;
    private Activity mActivity;
    private LayoutInflater mInf;
    private LogRecyclerViewAdapter mRecyclerAdapter;
    private LoginWithAmazon mLWA;
    private EditText mProductID;
    private EditText mProductDSN;
    private PlaybackControllerHandler mPlaybackController;
    private ImageButton mLWAButton;
    private View mLogoutButton;

    public Drawer( LinearLayout drawerLayout, LogRecyclerViewAdapter recyclerAdapter,
                  Activity activity, LoginWithAmazon lwa, PlaybackControllerHandler playbackController )
    {
        mDrawerLayout = drawerLayout;
        mActivity = activity;
        mInf = activity.getLayoutInflater();
        mLWA = lwa;
        mRecyclerAdapter = recyclerAdapter;
        mPlaybackController = playbackController;

        mProductID = mDrawerLayout.findViewById( R.id.product_id );
        mProductDSN = mDrawerLayout.findViewById( R.id.product_dsn );

        mProductID.setOnFocusChangeListener( new View.OnFocusChangeListener()
        {
            @Override
            public void onFocusChange( View v, boolean hasFocus )
            {
                if( !hasFocus )
                {
                    InputMethodManager imm = ( InputMethodManager ) mActivity.getSystemService( Context.INPUT_METHOD_SERVICE );
                    imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                }
            }
        });

        mProductDSN.setOnFocusChangeListener( new View.OnFocusChangeListener()
        {
            @Override
            public void onFocusChange( View v, boolean hasFocus )
            {
                if( !hasFocus )
                {
                    InputMethodManager imm = ( InputMethodManager ) mActivity.getSystemService( Context.INPUT_METHOD_SERVICE );
                    imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                }
            }
        });
    }

    public void configure()
    {
        configureLWA();
        configurePlaybackController();
        configureLogFilterOptions();
    }

    private void configureLWA()
    {

        if( mLWA != null )
        {

            mLWAButton = mDrawerLayout.findViewById( R.id.action_login_with_amazon );
            mLWAButton.setBackgroundResource( R.drawable.lwa_btn_src );
            mLWAButton.setOnClickListener( new View.OnClickListener()
            {
                @Override
                public void onClick( View v )
                {
                    mLWA.setProductID( mProductID.getText().toString() );
                    mLWA.setProductDSN( mProductDSN.getText().toString() );
                    mLWA.loginWithAmazonBrowser();
                }
            });

            mLogoutButton = mDrawerLayout.findViewById( R.id.action_logout );
            mLogoutButton.setOnClickListener( new View.OnClickListener()
            {
                @Override
                public void onClick( View v )
                {
                    mLWA.logout();
                }
            });
            mLogoutButton.setVisibility( View.GONE );

            mLWA.addObserver( this );
        }
    }

    private void configurePlaybackController()
    {
        if ( mPlaybackController != null )
        {
            View controller = mDrawerLayout.findViewById( R.id.playback_controller );
            ImageButton prev = controller.findViewById( R.id.prev );
            ImageButton playPause = controller.findViewById( R.id.play );
            ImageButton next = controller.findViewById( R.id.next );

            prev.setOnClickListener( new View.OnClickListener()
            {
                @Override
                public void onClick( View v )
                {
                    mPlaybackController.previousButtonPressed();
                }
            });

            playPause.setOnClickListener( new View.OnClickListener()
            {
                @Override
                public void onClick( View v )
                {
                    mPlaybackController.playButtonPressed();
                }
            });

            next.setOnClickListener( new View.OnClickListener()
            {
                @Override
                public void onClick( View v )
                {
                    mPlaybackController.nextButtonPressed();
                }
            });

            prev.setEnabled( false );
            playPause.setEnabled( false );
            next.setEnabled( false );

            mPlaybackController.setControls( prev, playPause, next );
        }
    }

    private void configureLogFilterOptions()
    {
        // Add switch for each source type
        LinearLayout sourceContainer = mDrawerLayout.findViewById( R.id.container_source );
        for( final LoggerHandler.Source source : LoggerHandler.Source.values() )
        {
            View switchItem = ( mInf.inflate( R.layout.drawer_switch, sourceContainer, false ) );
            ( (TextView) switchItem.findViewById( R.id.text ) ).setText( source.toString() );
            SwitchCompat drawerSwitch = switchItem.findViewById( R.id.drawer_switch );
            drawerSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener()
                 {
                     @Override
                     public void onCheckedChanged( CompoundButton buttonView, boolean isChecked )
                     {
                         toggleFilterSwitch( isChecked, source.toString(), "source" );
                     }
                 }
            );
            sourceContainer.addView( switchItem );
        }

        // Add switch for each level type
        LinearLayout levelContainer = mDrawerLayout.findViewById( R.id.container_level );
        for( final Logger.Level level : Logger.Level.values() )
        {
            View switchItem = ( mInf.inflate( R.layout.drawer_switch, levelContainer, false ) );
            ( ( TextView ) switchItem.findViewById( R.id.text ) ).setText( level.toString() );
            SwitchCompat drawerSwitch = switchItem.findViewById( R.id.drawer_switch );
            drawerSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener()
                 {
                     @Override
                     public void onCheckedChanged( CompoundButton buttonView, boolean isChecked )
                     {
                         toggleFilterSwitch( isChecked, level.toString(), "level" );
                     }
                 }
            );
            levelContainer.addView( switchItem );
        }
    }

    private void toggleFilterSwitch( boolean isChecked, String filter, String type )
    {
        if( !isChecked )
        {
            // filter out the level
            mRecyclerAdapter.addFilterItem( type, filter );
        } else
        {
            // add level back to log
            mRecyclerAdapter.removeFilterItem( type, filter );
        }
    }

    @Override
    public void update( Observable o, Object arg )
    {
        if( mLWAButton != null)
        {
            final String message = arg.toString();
            mActivity.runOnUiThread( new Runnable()
            {
                @Override
                public void run()
                {
                    if ( message.equals( "logged in" ) )
                    {
                        mLWAButton.setVisibility( View.GONE );
                        mLogoutButton.setVisibility( View.VISIBLE );
                    }
                    else if ( message.equals( "logged out" ) )
                    {
                        mLogoutButton.setVisibility( View.GONE );
                        mLWAButton.setVisibility( View.VISIBLE );
                    }
                }
            });
        }
    }
}
