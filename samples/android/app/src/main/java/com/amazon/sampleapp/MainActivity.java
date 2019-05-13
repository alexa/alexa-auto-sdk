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

package com.amazon.sampleapp;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.SwitchCompat;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.amazon.aace.alexa.AlexaClient;
import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aace.alexa.Speaker;
import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.amazonLite.config.AmazonLiteConfiguration;
import com.amazon.aace.carControl.CarControlConfiguration;
import com.amazon.aace.carControl.ClimateControlInterface;
import com.amazon.aace.communication.config.AlexaCommsConfiguration;
import com.amazon.aace.core.CoreProperties;
import com.amazon.aace.core.Engine;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.localSkillService.config.LocalSkillServiceConfiguration;
import com.amazon.aace.localVoiceControl.config.LocalVoiceControlConfiguration;
import com.amazon.aace.logger.Logger;
import com.amazon.aace.logger.config.LoggerConfiguration;
import com.amazon.aace.modules.ExtraModules;
import com.amazon.aace.storage.config.StorageConfiguration;
import com.amazon.aace.vehicle.config.VehicleConfiguration;
import com.amazon.sampleapp.impl.Alerts.AlertsHandler;
import com.amazon.sampleapp.impl.AlexaClient.AlexaClientHandler;
import com.amazon.sampleapp.impl.AudioPlayer.AudioPlayerHandler;
import com.amazon.sampleapp.impl.AuthProvider.AuthProviderHandler;
import com.amazon.sampleapp.impl.AuthProvider.LoginWithAmazonCBL;
/* LWA Browser Sample Implementation */
//import com.amazon.sampleapp.impl.AuthProvider.LoginWithAmazonBrowser;
import com.amazon.sampleapp.impl.CarControl.ClimateControlHandler;
import com.amazon.sampleapp.impl.Common.AudioInputManager;
import com.amazon.sampleapp.impl.Communication.AlexaCommsHandler;
import com.amazon.sampleapp.impl.Communication.AlexaCommsView;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerConfiguration;
import com.amazon.sampleapp.impl.ExternalMediaPlayer.MACCPlayer;
import com.amazon.sampleapp.impl.LocalMediaSource.CDLocalMediaSource;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.LocationProvider.LocationProviderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.MediaPlayer.MediaPlayerHandler;
import com.amazon.sampleapp.impl.MediaPlayer.RawAudioMediaPlayerHandler;
import com.amazon.sampleapp.impl.Navigation.NavigationHandler;
import com.amazon.sampleapp.impl.NetworkInfoProvider.NetworkInfoProviderHandler;
import com.amazon.sampleapp.impl.Notifications.NotificationsHandler;
import com.amazon.sampleapp.impl.PhoneCallController.PhoneCallControllerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;
import com.amazon.sampleapp.impl.SpeechRecognizer.SpeechRecognizerHandler;
import com.amazon.sampleapp.impl.SpeechSynthesizer.SpeechSynthesizerHandler;
import com.amazon.sampleapp.impl.TemplateRuntime.TemplateRuntimeHandler;
import com.amazon.sampleapp.logView.LogEntry;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Observable;
import java.util.Observer;
import java.util.UUID;

/* Sample Metrics Code */
//import com.amazon.metricuploadservice.MetricsUploadService;
//import com.amazon.metricuploadservice.MetricUploadConfiguration;

public class MainActivity extends AppCompatActivity implements Observer {

    /* AACE Platform Interface Handlers */

    // Common
    private AudioInputManager mAudioInputManager;

    // Alexa
    private AlertsHandler mAlerts;
    private AlexaClientHandler mAlexaClient;
    private AudioPlayerHandler mAudioPlayer;
    private AuthProviderHandler mAuthProvider;
    private AlexaCommsHandler mAlexaCommsHandler;
    private EqualizerControllerHandler mEqualizerControllerHandler;
    private NotificationsHandler mNotifications;
    private PhoneCallControllerHandler mPhoneCallController;
    private ContactUploaderHandler mContactUploader;
    private PlaybackControllerHandler mPlaybackController;
    private SpeechRecognizerHandler mSpeechRecognizer;
    private SpeechSynthesizerHandler mSpeechSynthesizer;
    private TemplateRuntimeHandler mTemplateRuntime;

    // Core
    private Engine mEngine;

    // Location
    private LocationProviderHandler mLocationProvider;

    // Logger
    private LoggerHandler mLogger;

    // Navigation
    private NavigationHandler mNavigation;

    // Network
    private NetworkInfoProviderHandler mNetworkInfoProvider;

    // Climate control
    private ClimateControlHandler mClimateControl;

    /* Log View Components */
    private RecyclerView mRecyclerView;
    private LogRecyclerViewAdapter mRecyclerAdapter;

    /* Shared Preferences */
    private SharedPreferences mPreferences;

    /* Speech Recognition Components */
    private MediaPlayer mAudioCueStartVoice; // Voice-initiated listening audio cue
    private MediaPlayer mAudioCueStartTouch; // Touch-initiated listening audio cue
    private MediaPlayer mAudioCueEnd; // End of listening audio cue
    private boolean mIsTalkButtonLongPressed = false;

    private static final String sTag = "CLI";
    private static final String sDeviceConfigFile = "app_config.json";
    private static final int sPermissionRequestCode = 0;
    private static final String[] sRequiredPermissions = { Manifest.permission.RECORD_AUDIO,
            Manifest.permission.ACCESS_FINE_LOCATION };
    private MACCPlayer mMACCPlayer;

    private CDLocalMediaSource mCDLocalMediaSource;
    private LVEConfigReceiver mLVEConfigReceiver;
    private MenuItem mTapToTalkIcon;

    /* Sample Metrics Code */
//    private MetricsUploadService mMetricsUploadService;

    @Override
    protected void onCreate( Bundle savedInstanceState ) {
        super.onCreate( savedInstanceState );

        // Check if permissions are missing and must be requested
        ArrayList<String> requests = new ArrayList<>();

        for ( String permission : sRequiredPermissions) {
            if ( ActivityCompat.checkSelfPermission( this, permission )
                    == PackageManager.PERMISSION_DENIED ) {
                requests.add( permission );
            }
        }

        // Request necessary permissions if not already granted, else start app
        if ( requests.size() > 0 ) {
            ActivityCompat.requestPermissions( this,
                    requests.toArray( new String[requests.size()] ), sPermissionRequestCode );
        } else create();
    }

    @Override
    public void onRequestPermissionsResult( int requestCode, @NonNull String[] permissions,
                                            @NonNull int[] grantResults ) {
        if ( requestCode == sPermissionRequestCode) {
            if ( grantResults.length > 0 ) {
                for ( int grantResult : grantResults ) {
                    if ( grantResult == PackageManager.PERMISSION_DENIED ) {
                        // Permission request was denied
                        Toast.makeText( this, "Permissions required",
                                Toast.LENGTH_LONG ).show();
                    }
                }
                // Permissions have been granted. Start app
                create();
            } else {
                // Permission request was denied
                Toast.makeText( this, "Permissions required", Toast.LENGTH_LONG ).show();
            }
        }
    }

    private void create() {

        // Set the main view content
        setContentView( R.layout.activity_main );

        // Initialize LVE Interaction service which will handle the initialization of LVE
        initLVE();

        // Add support action toolbar for action buttons
        setSupportActionBar( ( Toolbar ) findViewById( R.id.actionToolbar ) );

        // Initialize RecyclerView list for log view
        mRecyclerView = findViewById( R.id.rvLog );
        mRecyclerView.setHasFixedSize( true );
        mRecyclerView.setLayoutManager( new LinearLayoutManager( this ) );
        mRecyclerAdapter = new LogRecyclerViewAdapter( getApplicationContext() );
        mRecyclerView.setAdapter( mRecyclerAdapter );
        setUpLogViewOptions();

        // Initialize sound effects for speech recognition
        mAudioCueStartVoice = MediaPlayer.create( this, R.raw.med_ui_wakesound );
        mAudioCueStartTouch = MediaPlayer.create( this, R.raw.med_ui_wakesound_touch );
        mAudioCueEnd = MediaPlayer.create( this, R.raw.med_ui_endpointing_touch );

        // Get shared preferences
        mPreferences = getSharedPreferences( getString( R.string.preference_file_key ),
                Context.MODE_PRIVATE );

        // Retrieve device config from config file and update preferences
        String clientId = "", clientSecret = "", productId = "", productDsn = "";
        JSONObject config = getConfigFromFile("config");
        if ( config != null ) {
            try {
                clientId = config.getString( "clientId" );
                clientSecret = config.getString( "clientSecret" );
                productId = config.getString( "productId" );
            } catch ( JSONException e ) {
                Log.w( sTag, "Missing device info in app_config.json" );
            }
            try {
                productDsn = config.getString( "productDsn" );
            } catch ( JSONException e ) {
                try {
                    // set Android ID as product DSN
                    productDsn = Settings.Secure.getString(getContentResolver(),
                            Settings.Secure.ANDROID_ID);
                    Log.i(sTag, "android id for DSN: " + productDsn);
                } catch ( Error error ) {
                    productDsn = UUID.randomUUID().toString();
                    Log.w(sTag, "android id not found, generating random DSN: " + productDsn);
                }
            }
        }
        updateDevicePreferences( clientId, clientSecret, productId, productDsn );

        // Display device config settings in GUI
        updateDeviceConfigGUI( clientId, clientSecret, productId, productDsn );

    }

    /**
     * Continue starting the Engine with the config received from LVE Service.
     * @param config  json string with LVE config if LVE is supported. null otherwise.
     */
    private void onLVEConfigReceived(String config) {
        // Initialize AAC engine and register platform interfaces
        try {
            startEngine(config);
        } catch ( RuntimeException e ) {
            Log.e( sTag, "Could not start engine. Reason: " + e.getMessage() );
            return;
        }

        // Observe log event changes to update the log view
        mLogger.addLogObserver( this );
        mSpeechRecognizer.addObserver( this );
    }

    /**
     * Registers Broadcast receiver callbacks and starts LVEInteractionService that will initialize LVE
     */
    private void initLVE() {
        //Register Broadcast receiver that will receive configuration back from the LVEInteractionService
        mLVEConfigReceiver = new LVEConfigReceiver();
        IntentFilter filter = new IntentFilter(LVEInteractionService.LVE_RECEIVER_INTENT);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLVEConfigReceiver, filter);

        // Start LVEInteractionService. This service is responsible for communicating with LVE
        startService(new Intent(this, LVEInteractionService.class));
    }

    /**
     * Configure the Engine and register platform interface instances
     * @param json json string with LVE config if LVE is supported. null otherwise.
     * @throws RuntimeException
     */
    private void startEngine(String json) throws RuntimeException {

        // Copy certs to the cache directory
        File cacheDir = getCacheDir();
        File appDataDir = new File( cacheDir, "appdata" );
        File certsDir = new File( appDataDir, "certs" );
        File modelsDir = new File( appDataDir, "models" );
        try {
            String[] certAssets = getAssets().list("certs");
            for ( String next : certAssets ) {
                copyAsset("certs/" + next, new File( certsDir, next ), false);
            }
        } catch ( IOException e ) {
            Log.w( sTag, "Cannot copy certs to cache directory. Error: " + e.getMessage() );
        }

        try {
            String[] modelsAssert = getAssets().list("models");
            // copy models forcefully on every start, so that during an app update (for instance an update
            // where models in assets folder are updated) the models are also updated on the device.
            for (String next : modelsAssert) {
                copyAsset("models/" + next, new File(modelsDir, next), true);
            }
        } catch (IOException e) {
            Log.w(sTag, "Cannot copy models to cache directory. Error: " + e.getMessage());
        }

        // Create AAC engine
        mEngine = Engine.create();
        ArrayList<EngineConfiguration> configuration = getEngineConfigurations(json, appDataDir, certsDir, modelsDir);

        EngineConfiguration[] configurationArray = configuration.toArray(new EngineConfiguration[configuration.size()]);
        boolean configureSucceeded = mEngine.configure( configurationArray );
        if ( !configureSucceeded ) throw new RuntimeException( "Engine configuration failed" );

        // Create the platform implementation handlers and register them with the engine
        // Logger
        if ( !mEngine.registerPlatformInterface(
                mLogger = new LoggerHandler()
            )
        ) throw new RuntimeException( "Could not register Logger platform interface" );

        mAudioInputManager = new AudioInputManager(mLogger);

        // LocationProvider
        if ( !mEngine.registerPlatformInterface(
                mLocationProvider = new LocationProviderHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register LocationProvider platform interface" );

        // AlexaClient
        if ( !mEngine.registerPlatformInterface(
                mAlexaClient = new AlexaClientHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register AlexaClient platform interface" );

        // PhoneCallController
        if ( !mEngine.registerPlatformInterface(
                mPhoneCallController = new PhoneCallControllerHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register PhoneCallController platform interface" );

        // PlaybackController
        if ( !mEngine.registerPlatformInterface(
                mPlaybackController = new PlaybackControllerHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register PlaybackController platform interface" );

        ExtraModules extraModules = new ExtraModules(getApplicationContext());

        // SpeechRecognizer
        boolean wakeWordSupported = extraModules.isAmazonLiteEnabled();
        if ( !mEngine.registerPlatformInterface(
                mSpeechRecognizer = new SpeechRecognizerHandler(
                        mAudioInputManager,
                        this,
                        mLogger,
                        wakeWordSupported,
                        true
                )
            )
        ) throw new RuntimeException( "Could not register SpeechRecognizer platform interface" );

        // AudioPlayer
        if ( !mEngine.registerPlatformInterface(
                mAudioPlayer = new AudioPlayerHandler(
                        new MediaPlayerHandler(
                                this,
                                mLogger, "Audio Player",
                                Speaker.Type.AVS_SPEAKER,
                                mPlaybackController )
                )
            )
        ) throw new RuntimeException( "Could not register AudioPlayer platform interface" );


        // SpeechSynthesizer
        if ( !mEngine.registerPlatformInterface(
                mSpeechSynthesizer = new SpeechSynthesizerHandler(
                        new MediaPlayerHandler(
                                this,
                                mLogger,
                                "Speech Synthesizer",
                                Speaker.Type.AVS_SPEAKER,
                                null )
                )
            )
        ) throw new RuntimeException( "Could not register SpeechSynthesizer platform interface" );

        // TemplateRuntime
        if ( !mEngine.registerPlatformInterface(
                mTemplateRuntime = new TemplateRuntimeHandler( mLogger, mPlaybackController )
            )
        ) throw new RuntimeException( "Could not register TemplateRuntime platform interface" );

        // Alerts
        if ( !mEngine.registerPlatformInterface(
                mAlerts = new AlertsHandler(
                        this,
                        mLogger,
                        new MediaPlayerHandler(
                                this,
                                mLogger,
                                "Alerts",
                                Speaker.Type.AVS_ALERTS,
                                null
                        )
                )
            )
        ) throw new RuntimeException( "Could not register Alerts platform interface" );

        // NetworkInfoProvider
        if ( !mEngine.registerPlatformInterface(
                mNetworkInfoProvider = new NetworkInfoProviderHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register NetworkInfoProvider platform interface" );

        // CBL Auth Handler
        LoginWithAmazonCBL LoginHandler = new LoginWithAmazonCBL( this, mLogger );

        // LWA Auth Handler - replace CBL to use LWA browser instead
        //LoginWithAmazonBrowser LoginHandler = new LoginWithAmazonBrowser( this, mLogger, this.getLifecycle() );

        // AuthProvider
        if ( !mEngine.registerPlatformInterface(
                mAuthProvider = new AuthProviderHandler( this, mLogger, LoginHandler)
            )
        ) throw new RuntimeException( "Could not register AuthProvider platform interface" );

        // Set auth handler as connection observer
        mNetworkInfoProvider.registerNetworkConnectionObserver( LoginHandler );

        // Navigation
        if ( !mEngine.registerPlatformInterface(
                mNavigation = new NavigationHandler( mLogger )
            )
        ) throw new RuntimeException( "Could not register Navigation platform interface" );

        // Notifications
        if ( !mEngine.registerPlatformInterface(
                mNotifications = new NotificationsHandler(
                        this,
                        mLogger,
                        new MediaPlayerHandler(
                                this,
                                mLogger,
                                "Notifications",
                                Speaker.Type.AVS_ALERTS,
                                null
                        )
                )
            )
        ) throw new RuntimeException( "Could not register Notifications platform interface" );

        // Contact-Uploader
        if (!mEngine.registerPlatformInterface(
            mContactUploader = new ContactUploaderHandler(this, mLogger)
            )
        ) throw new RuntimeException("Could not register ContactUploader platform interface");

        // EqualizerController
        if ( !mEngine.registerPlatformInterface(
                mEqualizerControllerHandler = new EqualizerControllerHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register EqualizerController platform interface" );

        // Alexa Comms Handler
        if (extraModules.isAlexaCommsEnabled()) {
            MediaPlayerHandler ringtoneMediaPlayerHandler = new MediaPlayerHandler(
                    this,
                    mLogger,
                    "CommunicationRingtone",
                    Speaker.Type.AVS_SPEAKER,
                    null);
            // Call audio player is required to play PCM 16 raw data @ 16 KHZ.
            // RawAudioMediaPlayerHandler can play from such a data stream.
            RawAudioMediaPlayerHandler callAudioMediaPlayerHandler = new RawAudioMediaPlayerHandler(
                    this,
                    mLogger,
                    "CommunicationCallAudio",
                    Speaker.Type.AVS_SPEAKER);
            if (!mEngine.registerPlatformInterface(
                    mAlexaCommsHandler = new AlexaCommsHandler(
                            mAudioInputManager,
                            ringtoneMediaPlayerHandler,
                            ringtoneMediaPlayerHandler.getSpeaker(),
                            callAudioMediaPlayerHandler,
                            callAudioMediaPlayerHandler.getSpeaker()
                    )
                )
            ) throw new RuntimeException("Could not register AlexaCommsHandler platform interface");
            AlexaCommsView commsView = findViewById(R.id.alexa_comms);
            commsView.setupUI(mAlexaCommsHandler);
        }

        // Climate Control Handler
        if (extraModules.isLocalVoiceControlEnabled()) {
            if (!mEngine.registerPlatformInterface(
                    mClimateControl = new ClimateControlHandler( this, mLogger )
                )
            ) throw new RuntimeException( "Could not register ClimateControl platform interface" );
            mClimateControl.addClimateControlSwitch();
            mClimateControl.addAirConditioningSwitch();
            ClimateControlInterface.AirConditioningMode[] modes = { ClimateControlInterface.AirConditioningMode.MANUAL, ClimateControlInterface.AirConditioningMode.AUTO };
            mClimateControl.addAirConditioningModeSelector( modes );
            mClimateControl.addFanSwitch( ClimateControlInterface.FanZone.ALL );
            mClimateControl.addFanSpeedControl( ClimateControlInterface.FanZone.ALL, 0, 100, 1 );
            mClimateControl.addTemperatureControl( ClimateControlInterface.TemperatureZone.ALL, 60, 80, 1, ClimateControlInterface.TemperatureUnit.FAHRENHEIT );
        }

        mMACCPlayer = new MACCPlayer(this,  mPlaybackController);
        if ( !mEngine.registerPlatformInterface( mMACCPlayer ) )  {
            Log.i("MACC", "registered not fine");
            throw new RuntimeException( "Could not register Notifications platform interface" );
        } else {
            Log.i("MACC", "registered fine");
        }
        mMACCPlayer.runDiscovery();

        // Mock CD platform handler
        if ( !mEngine.registerPlatformInterface(
                mCDLocalMediaSource = new CDLocalMediaSource(this, mLogger, CDLocalMediaSource.Source.COMPACT_DISC )
        ) ) throw new RuntimeException( "Could not register Mock CD player Local Media Source platform interface" );

        /* Sample Metrics Code */
//        // Create and configure MetricsUploadService
//        MetricUploadConfiguration metricUploadConfiguration = initializeMetricsConfiguration();
//        mMetricsUploadService = new MetricsUploadService( this.getApplicationContext(), metricUploadConfiguration );
//        if ( !mEngine.registerPlatformInterface( mMetricsUploadService) ) {
//            throw new RuntimeException( "Could not register MetricsUploader platform interface" );
//        }
        // Alexa Locale
        final String supportedLocales = mEngine.getProperty(AlexaProperties.SUPPORTED_LOCALES);
        final String[] localesArray = supportedLocales.split(",");

        ArrayAdapter<String> localeAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, localesArray);

        Spinner spinnerView = (Spinner) findViewById(R.id.locale_spinner);
        spinnerView.setAdapter(localeAdapter);

        final String defaultLocale = mEngine.getProperty(AlexaProperties.LOCALE);

        int localePosition = localeAdapter.getPosition(defaultLocale);
        if (localePosition < 0) {
            Log.e( sTag, defaultLocale + " is not in the Supported Locales" );
            localePosition = 0;
        }
        spinnerView.setSelection ( localePosition );

        spinnerView.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected (AdapterView<?> arg0, View arg1,
                                        int position, long arg3) {
                String s = localesArray[position];
                if( !mEngine.getProperty(AlexaProperties.LOCALE).equals(s) ) {

                    Toast.makeText(MainActivity.this, "Switching Alexa locale to " + s,
                            Toast.LENGTH_SHORT).show();

                    mEngine.setProperty(AlexaProperties.LOCALE, s);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub
            }
        });

        // Start the engine
        if ( !mEngine.start() ) throw new RuntimeException( "Could not start engine" );
        //log whether LocationProvider gave a supported country
        mLogger.postInfo( "Country Supported: ",  mEngine.getProperty( AlexaProperties.COUNTRY_SUPPORTED ));

        mContactUploader.onInitialize();
        mAuthProvider.onInitialize();
        initTapToTalk();
    }

    /**
     * Get the configurations to start the Engine
     * @param json json string with LVE config if LVE is supported. null otherwise.
     * @param appDataDir path to app's data directory
     * @param certsDir path to certificates directory
     * @return List of Engine configurations
     */
    private ArrayList<EngineConfiguration> getEngineConfigurations(String json, File appDataDir, File certsDir, File modelsDir) {
        // Configure the engine
        String productDsn = mPreferences.getString( getString( R.string.preference_product_dsn ), "" );
        String clientId = mPreferences.getString( getString( R.string.preference_client_id ), "" );
        String productId = mPreferences.getString( getString( R.string.preference_product_id ), "" );

        AlexaConfiguration.TemplateRuntimeTimeout [] timeoutList = new AlexaConfiguration.TemplateRuntimeTimeout[ ]{
                new AlexaConfiguration.TemplateRuntimeTimeout ( AlexaConfiguration.TemplateRuntimeTimeoutType.DISPLAY_CARD_TTS_FINISHED_TIMEOUT, 8000 ),
                new AlexaConfiguration.TemplateRuntimeTimeout ( AlexaConfiguration.TemplateRuntimeTimeoutType.DISPLAY_CARD_AUDIO_PLAYBACK_FINISHED_TIMEOUT, 8000),
                new AlexaConfiguration.TemplateRuntimeTimeout ( AlexaConfiguration.TemplateRuntimeTimeoutType.DISPLAY_CARD_AUDIO_PLAYBACK_STOPPED_PAUSED_TIMEOUT, 1800000)
        };

        AmazonLiteConfiguration.AmazonLiteModelConfig[] modelsInfoList = new AmazonLiteConfiguration.AmazonLiteModelConfig[]{
                new AmazonLiteConfiguration.AmazonLiteModelConfig("de-DE", "D.de-DE.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("en-IN", "D.en-IN.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("en-US", "D.en-US.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("es-ES", "D.es-ES.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("fr-FR", "D.fr-FR.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("it-IT", "D.it-IT.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("ja-JP", "D.ja-JP.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("en-GB", "U_250k.en-GB.alexa.bin"),
                new AmazonLiteConfiguration.AmazonLiteModelConfig("en-AU", "D.en-US.alexa.bin"), // Mapped to en_US
                new AmazonLiteConfiguration.AmazonLiteModelConfig("en-CA", "D.en-US.alexa.bin"), // Mapped to en_US
                new AmazonLiteConfiguration.AmazonLiteModelConfig("fr-CA", "D.fr-FR.alexa.bin"), // Mapped to fr_FR
                new AmazonLiteConfiguration.AmazonLiteModelConfig("es-MX", "D.es-ES.alexa.bin")  // Mapped to es_ES
        };

        JSONObject config = null;
        ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
                //AlexaConfiguration.createCurlConfig( certsDir.getPath(), "wlan0" ), Uncomment this line to specify the interface name to use by AVS.
                AlexaConfiguration.createCurlConfig(certsDir.getPath()),
                AlexaConfiguration.createDeviceInfoConfig(productDsn, clientId, productId),
                AlexaConfiguration.createMiscStorageConfig(appDataDir.getPath() + "/miscStorage.sqlite"),
                AlexaConfiguration.createCertifiedSenderConfig(appDataDir.getPath() + "/certifiedSender.sqlite"),
                AlexaConfiguration.createAlertsConfig(appDataDir.getPath() + "/alerts.sqlite"),
                AlexaConfiguration.createSettingsConfig(appDataDir.getPath() + "/settings.sqlite"),
                AlexaConfiguration.createNotificationsConfig(appDataDir.getPath() + "/notifications.sqlite"),
                AlexaConfiguration.createEqualizerControllerConfig(
                        EqualizerConfiguration.getSupportedBands(),
                        EqualizerConfiguration.getMinBandLevel(),
                        EqualizerConfiguration.getMaxBandLevel(),
                        EqualizerConfiguration.getDefaultBandLevels() ),
                // Uncomment the below line to specify the template runtime values
                //AlexaConfiguration.createTemplateRuntimeTimeoutConfig( timeoutList ),
                StorageConfiguration.createLocalStorageConfig(appDataDir.getPath() + "/localStorage.sqlite"),
                AlexaCommsConfiguration.createCommsConfig(certsDir.getPath()),

                // Example Vehicle Config
                VehicleConfiguration.createVehicleInfoConfig(new VehicleConfiguration.VehicleProperty[]{
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.MAKE, "Amazon"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.MODEL, "AmazonCarOne"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.TRIM, "Advance"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.YEAR, "2025"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.GEOGRAPHY, "US"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.VERSION, String.format(
                                "Vehicle Software Version 1.0 (Auto SDK Version %s)", mEngine.getProperty(CoreProperties.VERSION))),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.OPERATING_SYSTEM, "Android 8.1 Oreo API Level 26"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.HARDWARE_ARCH, "Armv8a"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.LANGUAGE, "en-US"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.MICROPHONE, "Single, roof mounted"),
                        // If this list is left blank, it will be fetched by the engine using amazon default endpoint
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.COUNTRY_LIST, "US,GB,IE,CA,DE,AT,IN,JP,AU,NZ,FR"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.VEHICLE_IDENTIFIER, "123456789a")
                })
                // Uncomment the below line to enable the runtime switching of the wakeword model.
                // Also, ensure that the wakeword models are copied from the package to the assets/models folder.
                // AmazonLiteConfiguration.createAmazonLiteConfig(modelsDir.getPath(), modelsInfoList)
        ));

        // If LVE is supported and available, we get the LVE config from LVE apk.
        // Use this config to initialize the local skills.
        // If LVE is not supported or available, initialize with default config
        if (json != null) {
            try {
                config = new JSONObject(json);
                configuration.add(LocalVoiceControlConfiguration.createIPCConfig( config.getString("AlexaHybrid.SDK.ExecutionController.SocketDirectory"), LocalVoiceControlConfiguration.SocketPermission.ALL, config.getString("AlexaHybrid.SDK.PlatformServices.SocketDirectory"), LocalVoiceControlConfiguration.SocketPermission.ALL, "127.0.0.1", config.getString("AlexaHybrid.SDK.ExecutionController.UnixDomainSocketDirectory") ));
                configuration.add(LocalSkillServiceConfiguration.createLocalSkillServiceConfig( config.getString("LocalSkillService.Server.Endpoint"), config.getString("AlexaHybrid.SDK.ArtifactManager.IngestionEP") ));
                configuration.add(CarControlConfiguration.createCarControlConfig( config.getString("Skill.SmartHomeSkillId.CHRDatabaseFile") ));
            }
            catch( Throwable ex ) {
                ex.printStackTrace();
            }
        } else {
            configuration.add(LocalVoiceControlConfiguration.createIPCConfig( appDataDir.getPath(), LocalVoiceControlConfiguration.SocketPermission.ALL, appDataDir.getPath(), LocalVoiceControlConfiguration.SocketPermission.ALL, "127.0.0.1", appDataDir.getPath() ));
            configuration.add(LocalSkillServiceConfiguration.createLocalSkillServiceConfig( appDataDir.getPath() + "/LSS.socket", appDataDir.getPath() + "/ER.socket" ));
            configuration.add(CarControlConfiguration.createCarControlConfig( appDataDir.getPath() + "/ApplianceDB.sqlite" ));
        }
        return configuration;
    }

    @Override
    public void onDestroy() {
        if ( mLogger != null ) mLogger.postInfo( sTag, "Engine stopped" );
        else Log.i( sTag, "Engine stopped" );

        if ( mAudioCueStartVoice != null ) {
            mAudioCueStartVoice.release();
            mAudioCueStartVoice = null;
        }
        if ( mAudioCueStartTouch != null ) {
            mAudioCueStartTouch.release();
            mAudioCueStartTouch = null;
        }
        if ( mAudioCueEnd != null ) {
            mAudioCueEnd.release();
            mAudioCueEnd = null;
        }

        if ( mLVEConfigReceiver != null ) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mLVEConfigReceiver);
        }

        if ( mNetworkInfoProvider != null ) { mNetworkInfoProvider.unregister(); }

        if ( mEngine != null ) { mEngine.dispose(); }

        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu( Menu menu ) {
        super.onCreateOptionsMenu( menu );
        getMenuInflater().inflate( R.menu.menu_main, menu );

        // Set tap-to-talk and hold-to-talk actions
        mTapToTalkIcon = menu.findItem( R.id.action_talk );
        initTapToTalk();
        return true;
    }

    private void initTapToTalk() {
        if ( mTapToTalkIcon != null && mAlexaClient != null && mSpeechRecognizer != null ) {
            mTapToTalkIcon.setActionView( R.layout.menu_item_talk );

            // Set hold-to-talk action
            mTapToTalkIcon.getActionView().setOnClickListener( new View.OnClickListener() {
                @Override
                public void onClick( View v ) {
                    if ( mAlexaClient.getConnectionStatus()
                            == AlexaClient.ConnectionStatus.CONNECTED ) {
                        mSpeechRecognizer.onTapToTalk();
                    } else {
                        String message = "AlexaClient not connected. ConnectionStatus: "
                                + mAlexaClient.getConnectionStatus();
                        if ( mLogger != null )  mLogger.postWarn( sTag, message );
                        else Log.w( sTag, message );
                    }
                }
            });

            // Start hold-to-talk button action
            mTapToTalkIcon.getActionView().setOnLongClickListener( new View.OnLongClickListener() {
                @Override
                public boolean onLongClick( View v ) {
                    if ( mAlexaClient.getConnectionStatus()
                            == AlexaClient.ConnectionStatus.CONNECTED ) {
                        mIsTalkButtonLongPressed = true;
                        mSpeechRecognizer.onHoldToTalk();
                    } else {
                        if ( mLogger != null ) {
                            mLogger.postWarn( sTag,
                                    "ConnectionStatus: DISCONNECTED" );
                        } else Log.w( sTag, "ConnectionStatus: DISCONNECTED" );
                    }
                    return true;
                }
            });

            // Release hold-to-talk button action
            mTapToTalkIcon.getActionView().setOnTouchListener( new View.OnTouchListener() {
                @Override
                public boolean onTouch( View v, MotionEvent m ) {
                    // Talk button released
                    if ( m.getAction() == MotionEvent.ACTION_UP ) {
                        if ( mIsTalkButtonLongPressed ) {
                            mIsTalkButtonLongPressed = false;
                            mSpeechRecognizer.onReleaseHoldToTalk();
                        }
                    }
                    return false;
                }
            });
        }
    }

    @Override
    public boolean onOptionsItemSelected( MenuItem item ) {
        switch ( item.getItemId() ) {
            case R.id.action_drawer:
                toggleDrawer();
                return true;
            default:
                return super.onOptionsItemSelected( item );
        }
    }

    public void toggleDrawer() {
        final View drawer = findViewById( R.id.drawer );
        if ( drawer.getVisibility() == View.VISIBLE ) {
            drawer.setVisibility( View.GONE );
        } else {
            drawer.setVisibility( View.VISIBLE );
        }
    }

    @Override
    public void update( Observable observable, Object object ) {
        if ( observable instanceof LoggerHandler.LoggerObservable ) {
            if ( object instanceof LogEntry ) {
                final LogEntry entry = ( LogEntry ) object;
                runOnUiThread( new Runnable() {
                    public void run() {
                        // Insert log entry into log view
                        mRecyclerAdapter.insertItem( entry );
                        // Scroll to bottom of log view
                        int count = mRecyclerAdapter.getItemCount();
                        int position = count > 0 ? count - 1 : 0;
                        mRecyclerView.scrollToPosition( position );
                    }
                } );
            }
        } else if ( observable instanceof SpeechRecognizerHandler.AudioCueObservable ) {
            if ( object.equals( SpeechRecognizerHandler.AudioCueState.START_TOUCH ) ) {
                // Play touch-initiated listening audio cue
                mAudioCueStartTouch.start();
            } else if ( object.equals( SpeechRecognizerHandler.AudioCueState.START_VOICE ) ) {
                // Play voice-initiated listening audio cue
                mAudioCueStartVoice.start();
            } else if ( object.equals( SpeechRecognizerHandler.AudioCueState.END ) ) {
                // Play stop listening audio cue
                mAudioCueEnd.start();
            }
        }
    }

    private void copyAsset( String assetPath, File destFile, boolean force ) {
        if ( !destFile.exists() || force ) {
            if ( destFile.getParentFile().exists() || destFile.getParentFile().mkdirs() ) {
                // Copy the asset to the dest path
                try (
                    InputStream is = getAssets().open( assetPath );
                    OutputStream os = new FileOutputStream( destFile )
                ) {
                    byte[] buf = new byte[ 1024 ];
                    int len;
                    while ( ( len = is.read( buf ) ) > 0 ) {
                        os.write( buf, 0, len );
                    }
                } catch ( IOException e ) {
                    Log.w( sTag, e.getMessage() );
                }
            } else {
                Log.w( sTag, "Could not create cache directory: "
                        + destFile.getParentFile() );
            }
        } else {
            Log.w( sTag, String.format( "Skipping existing file in cache: %s to: %s",
                    assetPath, destFile ) );
        }
    }

    private JSONObject getConfigFromFile(String key) {
        JSONObject obj = null;
        try (
            InputStream is = getAssets().open(sDeviceConfigFile)
        ) {
            byte[] buffer = new byte[ is.available() ];
            is.read( buffer );
            String json = new String( buffer, "UTF-8" );
            obj = new JSONObject( json );
        } catch ( Exception e ) {
            Log.w( sTag, String.format( "Cannot read %s from assets directory. Error: %s",
                    sDeviceConfigFile, e.getMessage() ) );
        }

        JSONObject config = null;
        if ( obj != null ) {
            try {
                config = obj.getJSONObject( key );
            } catch ( JSONException e ) {
                Log.w( sTag, "No device config specified in " + sDeviceConfigFile );
            }
        }
        return config;
    }

    private void updateDevicePreferences( String clientId,
                                          String clientSecret,
                                          String productId,
                                          String productDsn ) {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( getString( R.string.preference_client_id ), clientId );
        editor.putString( getString( R.string.preference_client_secret ), clientSecret );
        editor.putString( getString( R.string.preference_product_id ), productId );
        editor.putString( getString( R.string.preference_product_dsn ), productDsn );
        editor.apply();
    }

    private void updateDeviceConfigGUI( final String clientId,
                                        final String clientSecret,
                                        final String productId,
                                        final String productDsn ) {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                ( ( TextView ) findViewById( R.id.clientId ) ).setText( clientId );
                ( ( TextView ) findViewById( R.id.clientSecret ) ).setText( clientSecret );
                ( ( TextView ) findViewById( R.id.productId ) ).setText( productId );
                ( ( TextView ) findViewById( R.id.productDsn ) ).setText( productDsn );
            }
        });
    }

    /* Sample Metrics Code */
//    // Retrieve metrics config from config file and create configuration object
//    private MetricUploadConfiguration initializeMetricsConfiguration() {
//        String amazonId = "", stageStr = "", productDsn = "";
//        boolean metricsEnabled = false;
//
//        JSONObject config = getConfigFromFile("metrics");
//        if ( config != null ) {
//            try {
//                amazonId = config.getString( "amazonId" );
//                metricsEnabled = config.getBoolean("metricsEnabled");
//                stageStr = config.getString( "stage" );
//            } catch ( JSONException e ) {
//                Log.w( sTag, "Missing metrics config info in app_config.json" );
//            }
//        }
//        productDsn = mPreferences.getString( this.getString( R.string.preference_product_dsn ), "" );
//        try {
//            MetricUploadConfiguration.Stage stage = null;
//            if (metricsEnabled) {
//                stage = MetricUploadConfiguration.Stage.valueOf(stageStr);
//            }
//            return MetricUploadConfiguration.createMetricUploadConfiguration(amazonId, productDsn, stage, metricsEnabled);
//        } catch (IllegalArgumentException| NullPointerException ex ) {
//            Log.e(sTag, "", ex);
//            throw ex;
//        }
//    }

    /// Set up log view filtering options
    private void setUpLogViewOptions() {
        LayoutInflater inf = getLayoutInflater();

        // Add switch for each log source type
        String[] sources = { "CLI", "AAC", "AVS" };
        LinearLayout sourceContainer = findViewById( R.id.sourceSwitchContainer );
        for ( final String source : sources ) {
            View switchItem = ( inf.inflate( R.layout.drawer_switch, sourceContainer, false ) );
            ( (TextView) switchItem.findViewById( R.id.text ) ).setText( source );
            SwitchCompat drawerSwitch = switchItem.findViewById( R.id.drawerSwitch );
            drawerSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener() {
                public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                    mRecyclerAdapter.setSourceDisplayMode( source, isChecked );
                }
            });
            sourceContainer.addView( switchItem );
        }

        // Add option in dropdown selector for each log level
        Spinner spinner = findViewById( R.id.levelSpinner );
        ArrayAdapter<Logger.Level> adapter = new ArrayAdapter<>( this, android.R.layout.simple_spinner_item );
        adapter.setDropDownViewResource( android.R.layout.simple_spinner_dropdown_item );

        for ( Logger.Level level : Logger.Level.values() ) {
            if ( level == Logger.Level.METRIC ) {
                continue;
            }
            adapter.add( level );
        }

        spinner.setAdapter( adapter );
        spinner.setOnItemSelectedListener( new AdapterView.OnItemSelectedListener() {
            public void onItemSelected( AdapterView<?> parent, View view, int position, long id ) {
                Logger.Level level = ( Logger.Level ) parent.getItemAtPosition(position);
                mRecyclerAdapter.setDisplayLevel( level );
                mRecyclerView.scrollToPosition( mRecyclerAdapter.getItemCount() - 1 );
            }
            public void onNothingSelected( AdapterView<?> parent ) {}
        });

        // Add switch to display or hide card logs
        View cardItem = findViewById( R.id.toggleCards );
        ( ( TextView ) cardItem.findViewById( R.id.text ) ).setText( R.string.log_switch_cards );
        SwitchCompat cardSwitch = cardItem.findViewById( R.id.drawerSwitch );
        cardSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                mRecyclerAdapter.setCardDisplayMode( isChecked );
            }
        });

        // Add switch to display or hide pretty-printed JSON template logs
        View tempItem = findViewById( R.id.toggleTemplates );
        ( ( TextView ) tempItem.findViewById( R.id.text ) ).setText( R.string.log_switch_template );
        SwitchCompat tempSwitch = tempItem.findViewById( R.id.drawerSwitch );
        tempSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                mRecyclerAdapter.setJsonDisplayMode( isChecked );
            }
        });

        // Clear log button
        findViewById( R.id.clearLogButton ).setOnClickListener(
            new View.OnClickListener() {
                public void onClick( View v ) { mRecyclerAdapter.clear(); }
            }
        );

        // Set initial level selection to INFO
        spinner.setSelection( Logger.Level.INFO.ordinal() );
        mRecyclerAdapter.setDisplayLevel( Logger.Level.INFO );
    }

    /**
     * Broadcast receiver to receive Configurations from LVEInteractionService
     */
    class LVEConfigReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (LVEInteractionService.LVE_RECEIVER_INTENT.equals(intent.getAction())) {
                if (intent.hasExtra(LVEInteractionService.LVE_RECEIVER_FAILURE_REASON)) {
                    String reason = intent.getStringExtra(LVEInteractionService.LVE_RECEIVER_FAILURE_REASON);
                    onLVEConfigReceived(null);
                    Log.e( sTag, "Failed to init AHE : " + reason);
                } else if (intent.hasExtra(LVEInteractionService.LVE_RECEIVER_CONFIGURATION)) {
                    Log.i( sTag, "Received config from LVE starting engine now : ");
                    String config = intent.getStringExtra(LVEInteractionService.LVE_RECEIVER_CONFIGURATION);
                    onLVEConfigReceived(config);
                }
            }
        }
    }
}
