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
import android.os.Environment;
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
import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.core.CoreProperties;
import com.amazon.aace.core.Engine;
import com.amazon.aace.core.config.ConfigurationFile;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.logger.Logger;
import com.amazon.aace.navigation.NavigationProperties;
import com.amazon.aace.storage.config.StorageConfiguration;
import com.amazon.aace.vehicle.config.VehicleConfiguration;
import com.amazon.aace.navigation.Navigation;
import com.amazon.sampleapp.impl.Alerts.AlertsHandler;
import com.amazon.sampleapp.impl.AlexaClient.AlexaClientHandler;
import com.amazon.sampleapp.impl.AlexaSpeaker.AlexaSpeakerHandler;
import com.amazon.sampleapp.impl.Audio.AudioInputProviderHandler;
import com.amazon.sampleapp.impl.Audio.AudioOutputProviderHandler;
import com.amazon.sampleapp.impl.AudioPlayer.AudioPlayerHandler;
import com.amazon.sampleapp.impl.AuthProvider.AuthProviderHandler;
import com.amazon.sampleapp.impl.AuthProvider.LoginWithAmazonCBL;

import com.amazon.sampleapp.impl.AddressBook.AddressBookHandler;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerConfiguration;
import com.amazon.sampleapp.impl.ExternalMediaPlayer.MACCPlayer;
import com.amazon.sampleapp.impl.GlobalPreset.GlobalPresetHandler;
import com.amazon.sampleapp.impl.LocalMediaSource.AMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.BluetoothLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.CDLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.DABLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.LineInLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.SiriusXMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.FMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.SatelliteLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.USBLocalMediaSource;

import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.LocationProvider.LocationProviderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
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
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Observable;
import java.util.Observer;
import java.util.UUID;

// AlexaComms Imports

// AmazonLite Imports

// LVC Imports

// AutoVoiceChrome Imports

// DCM Imports

public class MainActivity extends AppCompatActivity implements Observer {
    private static final String TAG = MainActivity.class.getSimpleName();

    private static final String sDeviceConfigFile = "app_config.json";
    private static final int sPermissionRequestCode = 0;
    private static final String[] sRequiredPermissions = { Manifest.permission.RECORD_AUDIO,
            Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.READ_EXTERNAL_STORAGE };

    /* AACE Platform Interface Handlers */

    // Alexa
    private AlertsHandler mAlerts;
    private AlexaClientHandler mAlexaClient;
    private AudioPlayerHandler mAudioPlayer;
    private AuthProviderHandler mAuthProvider;
    private EqualizerControllerHandler mEqualizerControllerHandler;
    private NotificationsHandler mNotifications;
    private PhoneCallControllerHandler mPhoneCallController;
    private AddressBookHandler mAddressBook;
    private PlaybackControllerHandler mPlaybackController;
    private SpeechRecognizerHandler mSpeechRecognizer;
    private SpeechSynthesizerHandler mSpeechSynthesizer;
    private TemplateRuntimeHandler mTemplateRuntime;
    private AlexaSpeakerHandler mAlexaSpeaker;

    // Alexa Comms Handler

    // LVC Handlers

    // Core
    private Engine mEngine;
    private boolean mEngineStarted = false;

    // Audio
    private AudioInputProviderHandler mAudioInputProvider;
    private AudioOutputProviderHandler mAudioOutputProvider;

    // Location
    private LocationProviderHandler mLocationProvider;

    // Logger
    private LoggerHandler mLogger;

    // Navigation
    private NavigationHandler mNavigation;

    // Network
    private NetworkInfoProviderHandler mNetworkInfoProvider;

    /* Log View Components */
    private RecyclerView mRecyclerView;
    private LogRecyclerViewAdapter mRecyclerAdapter;

    /* Shared Preferences */
    private SharedPreferences mPreferences;

    /* Speech Recognition Components */
    private boolean mIsTalkButtonLongPressed = false;

    private MediaPlayer mAudioCueStartVoice; // Voice-initiated listening audio cue
    private MediaPlayer mAudioCueStartTouch; // Touch-initiated listening audio cue
    private MediaPlayer mAudioCueEnd; // End of listening audio cue
    private MACCPlayer mMACCPlayer;

    private CDLocalMediaSource mCDLocalMediaSource;
    private DABLocalMediaSource mDABLocalMediaSource;
    private SiriusXMLocalMediaSource mSIRUSXMLocalMediaSource;
    private AMLocalMediaSource mAMLocalMediaSource;
    private FMLocalMediaSource mFMLocalMediaSource;
    private BluetoothLocalMediaSource mBTLocalMediaSource;
    private LineInLocalMediaSource mLILocalMediaSource;
    private SatelliteLocalMediaSource mSATRADLocalMediaSource;
    private USBLocalMediaSource mUSBLocalMediaSource;

    private GlobalPresetHandler mGlobalPresetHandler;

    private LVCConfigReceiver mLVCConfigReceiver;

    private MenuItem mTapToTalkIcon;
    // Earcon Settings
    private boolean mDisableStartOfRequestEarcon;
    private boolean mDisableEndOfRequestEarcon;

    // Lock for Earcons
    private Object mDisableStartOfRequestEarconLock = new Object();
    private Object mDisableEndOfRequestEarconLock = new Object();

    /* AutoVoiceChrome Controller */

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

        // Initialize LVCInteractionService to start LVC, if supported
        initLVC();

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
        String clientId = "", productId = "", productDsn = "";
        JSONObject config = FileUtils.getConfigFromFile(getAssets(), sDeviceConfigFile, "config");
        if ( config != null ) {
            try {
                clientId = config.getString( "clientId" );
                productId = config.getString( "productId" );
            } catch ( JSONException e ) {
                Log.w( TAG, "Missing device info in app_config.json" );
            }
            try {
                productDsn = config.getString( "productDsn" );
            } catch ( JSONException e ) {
                try {
                    // set Android ID as product DSN
                    productDsn = Settings.Secure.getString( getContentResolver(),
                            Settings.Secure.ANDROID_ID );
                    Log.i( TAG, "android id for DSN: " + productDsn );
                } catch ( Error error ) {
                    productDsn = UUID.randomUUID().toString();
                    Log.w( TAG, "android id not found, generating random DSN: " + productDsn );
                }
            }
        }
        updateDevicePreferences( clientId, productId, productDsn );

        // Display device config settings in GUI
        updateDeviceConfigGUI( clientId, productId, productDsn );

    }

    /**
      * Start {@link LVCInteractionService}, the service that initializes and communicates with LVC,
      * and register a broadcast receiver to receive the configuration from LVC provided through the
      * {@link LVCInteractionService}
     */
    private void initLVC() {
        // Register broadcast receiver for configuration from the LVCInteractionService
        mLVCConfigReceiver = new LVCConfigReceiver();
        IntentFilter filter = new IntentFilter(LVCInteractionService.LVC_RECEIVER_INTENT);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLVCConfigReceiver, filter);

        // Start LVCInteractionService to communicate with LVC
        startService(new Intent(this, LVCInteractionService.class));
    }

    /**
     * Continue starting the Engine with the config received from LVC Service.
     * @param config  json string with LVC config if LVC is supported, null otherwise
     */
    private void onLVCConfigReceived(String config) {
        // Initialize AAC engine and register platform interfaces
        try {
            if (!mEngineStarted) {
                startEngine(config);
            }
        } catch ( RuntimeException e ) {
            Log.e( TAG, "Could not start engine. Reason: " + e.getMessage() );
            return;
        }

        // Observe log event changes to update the log view
        mLogger.addLogObserver( this );
        mSpeechRecognizer.addObserver( this );
    }

    /**
     * Configure the Engine and register platform interface instances
     * @param json JSON string with LVC config if LVC is supported, null otherwise.
     * @throws RuntimeException
     */
    private void startEngine(String json) throws RuntimeException {

        // Create an "appdata" subdirectory in the cache directory for storing application data
        File cacheDir = getCacheDir();
        File appDataDir = new File( cacheDir, "appdata" );
        File sampleDataDir = new File( cacheDir, "sampledata" );

        // Copy certs from assets to certs subdirectory of cache directory
        File certsDir = new File( appDataDir, "certs" );
        FileUtils.copyAllAssets(getAssets(), "certs", certsDir, false);

        // Copy models from assets to certs subdirectory of cache directory.
        // Force copy the models on every start so that the models on device cache are always the latest
        // from the APK
        File modelsDir = new File( appDataDir, "models" );
        FileUtils.copyAllAssets(getAssets(), "models", modelsDir, true);

        copyAsset("Contacts.json", new File(sampleDataDir, "Contacts.json"), false);
        copyAsset("NavigationFavorites.json", new File(sampleDataDir, "NavigationFavorites.json"), false);

        // Create AAC engine
        mEngine = Engine.create(this);
        ArrayList<EngineConfiguration> configuration = getEngineConfigurations( json, appDataDir, certsDir, modelsDir );

        EngineConfiguration[] configurationArray = configuration.toArray( new EngineConfiguration[configuration.size()] );
        boolean configureSucceeded = mEngine.configure( configurationArray );
        if ( !configureSucceeded ) throw new RuntimeException( "Engine configuration failed" );

        // Create the platform implementation handlers and register them with the engine
        // Logger
        if ( !mEngine.registerPlatformInterface(
                mLogger = new LoggerHandler()
            )
        ) throw new RuntimeException( "Could not register Logger platform interface" );

        // AudioInputProvider
        if ( !mEngine.registerPlatformInterface(
                mAudioInputProvider = new AudioInputProviderHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register AudioInputProvider platform interface" );

        // AudioInputProvider
        if ( !mEngine.registerPlatformInterface(
                mAudioOutputProvider = new AudioOutputProviderHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register AudioOutputProvider platform interface" );

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

        // SpeechRecognizer
        boolean wakeWordSupported = false;
        if ( !mEngine.registerPlatformInterface(
                mSpeechRecognizer = new SpeechRecognizerHandler(
                        this,
                        mLogger,
                        wakeWordSupported,
                        true
                )
            )
        ) throw new RuntimeException( "Could not register SpeechRecognizer platform interface" );

        // AudioPlayer
        if ( !mEngine.registerPlatformInterface(
                mAudioPlayer = new AudioPlayerHandler( mLogger, mAudioOutputProvider, mPlaybackController )
            )
        ) throw new RuntimeException( "Could not register AudioPlayer platform interface" );

        // SpeechSynthesizer
        if ( !mEngine.registerPlatformInterface(
                mSpeechSynthesizer = new SpeechSynthesizerHandler()
            )
        ) throw new RuntimeException( "Could not register SpeechSynthesizer platform interface" );

        // TemplateRuntime
        if ( !mEngine.registerPlatformInterface(
                mTemplateRuntime = new TemplateRuntimeHandler( mLogger, mPlaybackController )
            )
        ) throw new RuntimeException( "Could not register TemplateRuntime platform interface" );

        // AlexaSpeaker
        if ( !mEngine.registerPlatformInterface(
            mAlexaSpeaker = new AlexaSpeakerHandler( this,  mLogger )
        )
        ) throw new RuntimeException( "Could not register AlexaSpeaker platform interface" );

        // Alerts
        if ( !mEngine.registerPlatformInterface(
                mAlerts = new AlertsHandler(
                        this,
                        mLogger )
            )
        ) throw new RuntimeException( "Could not register Alerts platform interface" );

        // NetworkInfoProvider
        if ( !mEngine.registerPlatformInterface(
                mNetworkInfoProvider = new NetworkInfoProviderHandler( this, mLogger, mEngine )
            )
        ) throw new RuntimeException( "Could not register NetworkInfoProvider platform interface" );

        // CBL Auth Handler
        LoginWithAmazonCBL LoginHandler = new LoginWithAmazonCBL( this, mLogger );

        // AuthProvider
        if ( !mEngine.registerPlatformInterface(
                mAuthProvider = new AuthProviderHandler( this, mLogger, LoginHandler)
            )
        ) throw new RuntimeException( "Could not register AuthProvider platform interface" );

        // Set auth handler as connection observer
        mNetworkInfoProvider.registerNetworkConnectionObserver( LoginHandler );

        // Navigation
        if ( !mEngine.registerPlatformInterface(
                mNavigation = new NavigationHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register Navigation platform interface" );
        else mEngine.setProperty( NavigationProperties.NAVIGATION_PROVIDER_NAME, "HERE" ); //set default provider name

        // Notifications
        if ( !mEngine.registerPlatformInterface(
                mNotifications = new NotificationsHandler(
                        this,
                        mLogger
                )
            )
        ) {
            throw new RuntimeException( "Could not register Notifications platform interface" );
        }

        // Contacts/NavigationFavorites
        String sampleContactsDataPath = sampleDataDir.getPath() + "/Contacts.json";;
        String sampleNavigationFavoritesDataPath = sampleDataDir.getPath() + "/NavigationFavorites.json";

        // Always use sample data from external storage if available
        File sampleContactsFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/Contacts.json");
        File sampleNavigationFavoritesFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/NavigationFavorites.json");

        if ( sampleContactsFile.exists() ) {
            sampleContactsDataPath = sampleContactsFile.getPath();
        }

        if ( sampleNavigationFavoritesFile.exists() ) {
            sampleNavigationFavoritesDataPath = sampleNavigationFavoritesFile.getPath();
        }

        if (!mEngine.registerPlatformInterface(
                mAddressBook = new AddressBookHandler(this, mLogger, sampleContactsDataPath, sampleNavigationFavoritesDataPath )
            )
        ) throw new RuntimeException("Could not register AddressBook platform interface");

        // EqualizerController
        if ( !mEngine.registerPlatformInterface(
                mEqualizerControllerHandler = new EqualizerControllerHandler( this, mLogger )
            )
        ) throw new RuntimeException( "Could not register EqualizerController platform interface" );

        // AlexaComms Handler

        // LVC Handlers

        mMACCPlayer = new MACCPlayer(this,  mLogger, mPlaybackController);
        if ( !mEngine.registerPlatformInterface( mMACCPlayer ) )  {
            Log.i("MACC", "registration failed");
            throw new RuntimeException( "Could not register external media player platform interface" );
        } else {
            Log.i("MACC", "registration succeeded");
        }
        mMACCPlayer.runDiscovery();

        // Mock CD platform handler
        if ( !mEngine.registerPlatformInterface(
                mCDLocalMediaSource = new CDLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock CD player Local Media Source platform interface" );

        // Mock DAB platform handler
        if ( !mEngine.registerPlatformInterface(
                mDABLocalMediaSource = new DABLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock DAB player Local Media Source platform interface" );

        // Mock AM platform handler
        if ( !mEngine.registerPlatformInterface(
            mAMLocalMediaSource = new AMLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock AM radio player Local Media Source platform interface" );

        // Mock SIRIUSXM platform handler
        if ( !mEngine.registerPlatformInterface(
                mSIRUSXMLocalMediaSource = new SiriusXMLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock SIRIUSXM player Local Media Source platform interface" );

        // Mock FM platform handler
        if ( !mEngine.registerPlatformInterface(
                mFMLocalMediaSource = new FMLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock FM radio player Local Media Source platform interface" );

        // Mock Bluetooth platform handler
        if ( !mEngine.registerPlatformInterface(
                mBTLocalMediaSource = new BluetoothLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock Bluetooth player Local Media Source platform interface" );

        // Mock Line In platform handler
        if ( !mEngine.registerPlatformInterface(
                mLILocalMediaSource = new LineInLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock Line In player Local Media Source platform interface" );

        // Mock Satellite Radio platform handler
        if ( !mEngine.registerPlatformInterface(
                mSATRADLocalMediaSource = new SatelliteLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock Satellite radio player Local Media Source platform interface" );

        // Mock USB platform handler
        if ( !mEngine.registerPlatformInterface(
                mUSBLocalMediaSource = new USBLocalMediaSource(this, mLogger)
        ) ) throw new RuntimeException( "Could not register Mock USB player Local Media Source platform interface" );

        // Mock global preset
        if ( !mEngine.registerPlatformInterface(
                mGlobalPresetHandler = new GlobalPresetHandler(this, mLogger )
        ) ) throw new RuntimeException( "Could not register Mock Global Preset platform interface" );

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
            Log.e( TAG, defaultLocale + " is not in the Supported Locales" );
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
        mEngineStarted = true;
        //log whether LocationProvider gave a supported country
        mLogger.postInfo( "Country Supported: ",  mEngine.getProperty( AlexaProperties.COUNTRY_SUPPORTED ));

        // Initialize AutoVoiceChrome

        mAddressBook.onInitialize();
        mAuthProvider.onInitialize();
        initTapToTalk();
        initEarconsSettings();
    }

    /**
     * Get the configurations to start the Engine
     * @param json JSON string with LVC config if LVC is supported, null otherwise.
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
        ));

        String endpointConfigPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/aace.json";
        if ( new File( endpointConfigPath ).exists() ) {
            EngineConfiguration alexaEndpointsConfig = ConfigurationFile.create( Environment.getExternalStorageDirectory().getAbsolutePath() + "/aace.json" );
            configuration.add(alexaEndpointsConfig);
            Log.i("getEngineConfigurations", "Overriding endpoints");
        }

        // AlexaComms Config

        // AmazonLite Config

        // LVC Config

        // DCM Config

        return configuration;
    }

    @Override
    public void onDestroy() {
        if ( mLogger != null ) mLogger.postInfo( TAG, "Engine stopped" );
        else Log.i( TAG, "Engine stopped" );

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

        if ( mLVCConfigReceiver != null ) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mLVCConfigReceiver);
        }

        if ( mNetworkInfoProvider != null ) { mNetworkInfoProvider.unregister(); }

        if ( mEngine != null ) { mEngine.dispose(); }

        // AutoVoiceChrome cleanup

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
                        // Notify Error state to AutoVoiceChrome

                        String message = "AlexaClient not connected. ConnectionStatus: "
                                + mAlexaClient.getConnectionStatus();
                        if ( mLogger != null )  mLogger.postWarn( TAG, message );
                        else Log.w( TAG, message );
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
                        // Notify Error state to AutoVoiceChrome

                        if ( mLogger != null ) {
                            mLogger.postWarn( TAG,
                                    "ConnectionStatus: DISCONNECTED" );
                        } else Log.w( TAG, "ConnectionStatus: DISCONNECTED" );
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

    private void initEarconsSettings() {
        mDisableStartOfRequestEarcon = false;
        mDisableEndOfRequestEarcon = false;

        // Switch to toggle start of request
        final View toggleStartOfRequestItem = findViewById( R.id.toggleEarconsStartOfRequest );
        final SwitchCompat startOfRequestSwitch = toggleStartOfRequestItem.findViewById( R.id.drawerSwitch );
        startOfRequestSwitch.setChecked( true );
        startOfRequestSwitch.setOnCheckedChangeListener(
            new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                    toggleStartOfRequestState( isChecked );
                }
            }
        );

        // Switch to toggle end of request
        final View toggleEndtOfRequestItem = findViewById( R.id.toggleEarconsEndOfRequest );
        final SwitchCompat endOfRequestSwitch = toggleEndtOfRequestItem.findViewById( R.id.drawerSwitch );
        endOfRequestSwitch.setChecked( true );
        endOfRequestSwitch.setOnCheckedChangeListener(
            new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                    toggleEndOfRequestState( isChecked );
                }
            }
        );
    }

    private void toggleEndOfRequestState( boolean isChecked ) {
        synchronized ( mDisableEndOfRequestEarconLock ) {
            if ( isChecked )
                mDisableEndOfRequestEarcon = false;
            else
                mDisableEndOfRequestEarcon = true;
        }
    }

    private void toggleStartOfRequestState( boolean isChecked ) {
        synchronized ( mDisableStartOfRequestEarconLock ) {
            if ( isChecked )
                mDisableStartOfRequestEarcon = false;
            else
                mDisableStartOfRequestEarcon = true;
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
                synchronized ( mDisableStartOfRequestEarconLock ) {
                    if( !mDisableStartOfRequestEarcon ) {
                        // Play touch-initiated listening audio cue
                        mAudioCueStartTouch.start();
                    }
                }
            } else if ( object.equals( SpeechRecognizerHandler.AudioCueState.START_VOICE ) ) {
                synchronized ( mDisableStartOfRequestEarconLock ) {
                    if ( !mDisableStartOfRequestEarcon ) {
                        // Play voice-initiated listening audio cue
                        mAudioCueStartVoice.start();
                    }
                }
            } else if ( object.equals( SpeechRecognizerHandler.AudioCueState.END ) ) {
                synchronized ( mDisableEndOfRequestEarconLock ) {
                    if ( !mDisableEndOfRequestEarcon ) {
                        // Play stop listening audio cue
                        mAudioCueEnd.start();
                    }
                }
            }
        }
    }

    private void copyAsset(String assetPath, File destFile, boolean force) {
        FileUtils.copyAsset(getAssets(), assetPath, destFile, force);
    }

    private void updateDevicePreferences( String clientId,
                                          String productId,
                                          String productDsn ) {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString( getString( R.string.preference_client_id ), clientId );
        editor.putString( getString( R.string.preference_product_id ), productId );
        editor.putString( getString( R.string.preference_product_dsn ), productDsn );
        editor.apply();
    }

    private void updateDeviceConfigGUI( final String clientId,
                                        final String productId,
                                        final String productDsn ) {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                ( ( TextView ) findViewById( R.id.clientId ) ).setText( clientId );
                ( ( TextView ) findViewById( R.id.productId ) ).setText( productId );
                ( ( TextView ) findViewById( R.id.productDsn ) ).setText( productDsn );
            }
        });
    }

    // Auto Voice Chrome initialize function

    // Set up log view filtering options
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
     * Broadcast receiver to receive configuration from LVC provided through the
     * {@link LVCInteractionService}
     */
    class LVCConfigReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (LVCInteractionService.LVC_RECEIVER_INTENT.equals(intent.getAction())) {
                if (intent.hasExtra(LVCInteractionService.LVC_RECEIVER_FAILURE_REASON)) {
                    // LVCInteractionService was unable to provide config from LVC
                    String reason = intent.getStringExtra(LVCInteractionService.LVC_RECEIVER_FAILURE_REASON);
                    onLVCConfigReceived(null);
                    Log.e( TAG, "Failed to init LVC: " + reason );
                } else if (intent.hasExtra(LVCInteractionService.LVC_RECEIVER_CONFIGURATION)) {
                    // LVCInteractionService received config from LVC
                    Log.i( TAG, "Received config from LVC, starting engine now" );
                    String config = intent.getStringExtra(LVCInteractionService.LVC_RECEIVER_CONFIGURATION);
                    onLVCConfigReceived(config);
                }
            }
        }
    }
}
