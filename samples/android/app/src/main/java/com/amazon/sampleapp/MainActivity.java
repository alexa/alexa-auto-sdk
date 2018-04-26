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

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.media.MediaPlayer;

import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.amazon.aace.alexa.AlexaClient;

import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.core.Engine;
import com.amazon.aace.core.config.ConfigurationFile;
import com.amazon.aace.core.config.EngineConfiguration;

import com.amazon.aace.logger.Logger;

import com.amazon.sampleapp.impl.AlertsHandler;
import com.amazon.sampleapp.impl.AlexaClientHandler;
import com.amazon.sampleapp.impl.AudioPlayerHandler;
import com.amazon.sampleapp.impl.AuthProviderHandler;
import com.amazon.sampleapp.impl.LocationProviderHandler;
import com.amazon.sampleapp.impl.LoggerHandler;
import com.amazon.sampleapp.impl.NotificationsHandler;
import com.amazon.sampleapp.impl.PlaybackControllerHandler;
import com.amazon.sampleapp.impl.SpeechRecognizerHandler;
import com.amazon.sampleapp.impl.SpeechSynthesizerHandler;
import com.amazon.sampleapp.impl.TemplateRuntimeHandler;
import com.amazon.sampleapp.impl.NavigationHandler;

import com.amazon.sampleapp.impl.AndroidMediaPlayer;
import com.amazon.sampleapp.view.LogEntry;
import com.amazon.sampleapp.view.LogRecyclerViewAdapter;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Observable;
import java.util.Observer;

public class MainActivity extends AppCompatActivity implements Observer
{
    // aace engine
    private Engine m_engine;

    // views
    private RecyclerView m_recyclerView;
    private RecyclerView.Adapter m_recyclerAdapter;
    private ArrayList<LogEntry> m_logList = new ArrayList<>(); // log model

    // audio sound effects
    private MediaPlayer m_startEffectMediaPlayer;

    // aace components
    private LoggerHandler m_logger;
    private LocationProviderHandler m_locationProvider;
    private AlexaClientHandler m_alexaClient;
    private SpeechRecognizerHandler m_speechRecognizer;
    private SpeechSynthesizerHandler m_speechSynthesizer;
    private AudioPlayerHandler m_audioPlayer;
    private TemplateRuntimeHandler m_templateRuntime;
    private AlertsHandler m_alerts;
    private AuthProviderHandler m_authProvider;
    private NavigationHandler m_navigation;
    private PlaybackControllerHandler m_playbackController;
    private NotificationsHandler m_notifications;

    // LWA components
    private LoginWithAmazon mLWA;
    private boolean m_lwaAPIKey = false;

    // Permissions
    private static final int PERMISSION_REQUEST_CODE = 0;
    String[] permissions = { Manifest.permission.RECORD_AUDIO, Manifest.permission.ACCESS_COARSE_LOCATION };

    @Override
    protected void onCreate( Bundle savedInstanceState )
    {
        super.onCreate( savedInstanceState );

        // Check if permissions are missing and must be requested
        ArrayList<String> requests = new ArrayList<>();

        for( String permission : permissions )
        {
            if( ActivityCompat.checkSelfPermission( this, permission ) == PackageManager.PERMISSION_DENIED )
            {
                requests.add( permission );
            }
        }

        if ( requests.size() > 0 )
        {
            ActivityCompat.requestPermissions( this, requests.toArray( new String[requests.size()] ), PERMISSION_REQUEST_CODE );
        } else
        {
            create();
        }
    }

    private void create()
    {
        // set the main view content bar
        setContentView( R.layout.activity_main );

        // add support action toolbar that we can add action buttons to
        setSupportActionBar( (Toolbar) findViewById( R.id.action_toolbar ) );

        // recycler list view
        m_recyclerView = findViewById( R.id.rvLog );
        m_recyclerView.setHasFixedSize( true );
        m_recyclerView.setLayoutManager( new LinearLayoutManager( this ) );
        m_recyclerAdapter = new LogRecyclerViewAdapter( m_logList, getApplicationContext() );
        m_recyclerView.setAdapter( m_recyclerAdapter );

        // observe log event changes so we can update the log view
        LoggerHandler.addLogObserver( this );
        TemplateRuntimeHandler.addTemplateObserver( this );
        NavigationHandler.addNavigationObserver( this );

        // initialize sound effects
        m_startEffectMediaPlayer = MediaPlayer.create( this, R.raw.med_ui_wakesound_touch );

        try
        {
            //
            // initialize aace engine and platform interfaces
            //
            m_engine = Engine.create();

            if( m_engine == null )
            {
                throw new RuntimeException( "Could not create Engine!" );
            }

            // set the TZ=UTC env variable (required for alerts)
            m_engine.setNativeEnv( "TZ", "UTC" );

            // setup the engine configuration
            File cacheDir = getCacheDir();
            File appDataDir = new File( cacheDir, "appdata" );

            // copy the engine config to the cache directory
            File configFile = new File( appDataDir, "config.json" );

            if( Arrays.asList(getResources().getAssets().list( "" ) ).contains( "api_key.txt" ) )
            {
                m_lwaAPIKey = true;
            } else Log.d( "W", "LWA Warning: api_key.txt does not exist in assets folder" );

            copyAsset( "config.json", configFile, true );

            //
            // copy certs to the cache directory
            //
            File certsDir = new File( appDataDir, "certs" );
            String[] certAssets = getAssets().list( "certs" );

            for( String next : certAssets ) {
                copyAsset( "certs/" + next, new File( certsDir, next ), false );
            }

            // configure the engine
            m_engine.configure( new EngineConfiguration[] {
                //ConfigurationFile.create( configFile.getPath() ),
                AlexaConfiguration.createCurlConfig( certsDir.getPath() ),
                AlexaConfiguration.createCertifiedSenderConfig( appDataDir.getPath() + "/certifiedSender.sqlite" ),
                AlexaConfiguration.createAlertsConfig( appDataDir.getPath() + "/alerts.sqlite" ),
                AlexaConfiguration.createSettingsConfig( appDataDir.getPath() + "/settings.sqlite" ),
                AlexaConfiguration.createNotificationsConfig( appDataDir.getPath() + "/notifications.sqlite" ),
            });

            // create the platform implementation handlers and register them with the engine
            m_engine.registerPlatformInterface( m_logger = new LoggerHandler( getApplicationContext(), Logger.Level.INFO ) );
            m_engine.registerPlatformInterface( m_locationProvider = new LocationProviderHandler( getApplicationContext(), m_logger ) );
            m_engine.registerPlatformInterface( m_alexaClient = new AlexaClientHandler( getApplicationContext(), m_logger ) );
            m_engine.registerPlatformInterface( m_playbackController = new PlaybackControllerHandler( getApplicationContext(), m_logger ) );
            m_engine.registerPlatformInterface( m_speechRecognizer = new SpeechRecognizerHandler( getApplicationContext(), m_logger, false ) );
            m_engine.registerPlatformInterface( m_audioPlayer = new AudioPlayerHandler( getApplicationContext(), new AndroidMediaPlayer( this, m_logger, "Audio Player", this, m_playbackController) ) );
            m_engine.registerPlatformInterface( m_speechSynthesizer = new SpeechSynthesizerHandler( getApplicationContext(), new AndroidMediaPlayer( this, m_logger, "Speech Synthesizer", this ) ) );
            m_engine.registerPlatformInterface( m_templateRuntime = new TemplateRuntimeHandler( getApplicationContext(), m_logger, m_playbackController ) );
            m_engine.registerPlatformInterface( m_alerts = new AlertsHandler( getApplicationContext(), m_logger, new AndroidMediaPlayer( this, m_logger, "Alerts", this ) ) );
            m_engine.registerPlatformInterface( m_authProvider = new AuthProviderHandler( getApplicationContext(), m_logger ) );
            m_engine.registerPlatformInterface( m_navigation = new NavigationHandler( getApplicationContext(), m_logger ) );
            m_engine.registerPlatformInterface( m_notifications = new NotificationsHandler( getApplicationContext(), m_logger, new AndroidMediaPlayer( this, m_logger, "Notifications", this ) ) );

        } catch( Throwable ex )
        {
            ex.printStackTrace();
        }

        // LWA
        mLWA = new LoginWithAmazon( m_logger, m_authProvider, m_lwaAPIKey, this );
        mLWA.loginWithAmazon();

        // Set drawer options
        LinearLayout drawerLayout = findViewById( R.id.drawer );
        Drawer drawer = new Drawer( drawerLayout, ( LogRecyclerViewAdapter ) m_recyclerAdapter, this, mLWA, m_playbackController ) ;
        drawer.configure();

        m_engine.start();
    }

    @Override
    protected void onSaveInstanceState( Bundle outState )
    {
        super.onSaveInstanceState( outState );
    }

    private void copyAsset( String assetPath, File destFile, boolean force ) throws java.io.IOException
    {
        if( !destFile.exists() || force )
        {
            if( destFile.getParentFile().exists() || destFile.getParentFile().mkdirs() )
            {
                // copy the asset to the dest path
                InputStream is = getAssets().open( assetPath );
                OutputStream os = new FileOutputStream( destFile );
                byte[] buf = new byte[1024];
                int len = 0;

                while( (len = is.read( buf )) > 0 )
                {
                    os.write( buf, 0, len );
                }

                is.close();
                os.close();
            } else
            {
                Log.e( "MainActivity", "Could not create cache directory: " + destFile.getParentFile() );
            }
        } else
        {
            Log.d( "MainActivity", "Skipping existing file in cache: " + assetPath + " to: " + destFile );
        }
    }

    @Override
    public void onDestroy()
    {
        if( m_logger != null )
        {
            m_logger.post( "ENGINE STOPPED" );
        }

        if( m_startEffectMediaPlayer != null )
        {
            m_startEffectMediaPlayer.release();
            m_startEffectMediaPlayer = null;
        }
        if( m_engine != null )
        {
            m_engine.dispose();
        }

        super.onDestroy();
    }

    @Override
    protected void onStart()
    {
        super.onStart();
    }

    @Override
    protected void onStop()
    {
        super.onStop();
    }

    @Override
    public boolean onCreateOptionsMenu( Menu menu )
    {
        super.onCreateOptionsMenu( menu );

        getMenuInflater().inflate( R.menu.menu_main, menu );

        // show icon
        if ( menu.getClass().getSimpleName().equals( "MenuBuilder" ) )
        {
            try
            {
                Field field = menu.getClass().getDeclaredField( "mOptionalIconsVisible" );
                field.setAccessible( true );
                field.setBoolean( menu, true );
            } catch ( Exception ignored )
            {
                ignored.printStackTrace();
            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected( MenuItem item )
    {
        switch( item.getItemId() )
        {
            case R.id.action_tap_to_talk:
                if( m_alexaClient.m_connectionStatus == AlexaClient.ConnectionStatus.CONNECTED )
                {
                    m_startEffectMediaPlayer.start();
                    m_speechRecognizer.tapToTalk();
                }
                else
                {
                    m_logger.post( "DISCONNECTED", Logger.Level.WARN, Color.YELLOW );
                }
                return true;

            case R.id.action_drawer:
                DrawerLayout drawerLayout = findViewById( R.id.drawer_layout );
                FrameLayout drawerView = findViewById( R.id.drawer_container );
                if( drawerLayout.isDrawerOpen( drawerView ) )
                {
                    drawerLayout.closeDrawer( drawerView );
                }
                else
                {
                    drawerLayout.openDrawer( drawerView );
                }
                return true;

            default:
                return super.onOptionsItemSelected( item );
        }
    }

    @Override
    public void onRequestPermissionsResult( int requestCode, @NonNull String[] permissions,
                                            @NonNull int[] grantResults )
    {
        if( requestCode == PERMISSION_REQUEST_CODE )
        {
            if( grantResults.length > 0 )
            {
                for( int grantResult : grantResults )
                {
                    if( grantResult == PackageManager.PERMISSION_DENIED )
                    {
                        // Permission request was denied. Exit app
                        finishAndRemoveTask();
                    }
                }
                // Permissions have been granted. Start app
                create();
            } else
            {
                // Permission request was denied. Exit app
                finishAndRemoveTask();
            }
        }
    }

    // updates Observables
    @Override
    public void update( Observable o, Object logModelObject )
    {
        // append to log list
        if( logModelObject instanceof LogEntry )
        {
            final LogEntry entry = ( LogEntry ) logModelObject;

            runOnUiThread( new Runnable()
            {
                @Override
                public void run()
                {
                    // recycler update
                    m_logList.add( entry );
                    m_recyclerAdapter.notifyItemInserted( m_logList.size() - 1 );
                    ( ( LogRecyclerViewAdapter ) m_recyclerAdapter ).filter();
                    int count = m_recyclerAdapter.getItemCount();
                    int position = count > 0 ? count - 1 : 0;
                    m_recyclerView.smoothScrollToPosition( position );
                }
            });
        } else Log.e( "ERROR", "TRYING TO ADD NON LOG ENTRY OBJECT" );
    }

    // for LWA cycle
    @Override
    protected void onResume()
    {
        super.onResume();
        if( mLWA != null )
        {
            mLWA.onResume();
        }
    }
}
