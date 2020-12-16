/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.design.widget.TabLayout;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
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

import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aace.alexa.SpeechRecognizer;
import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.aace.core.CoreProperties;
import com.amazon.aace.core.Engine;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.ConfigurationFile;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;
import com.amazon.aace.logger.Logger;
import com.amazon.aace.navigation.Navigation;
import com.amazon.aace.navigation.config.NavigationConfiguration;
import com.amazon.aace.propertyManager.PropertyManager;
import com.amazon.aace.storage.config.StorageConfiguration;
import com.amazon.aace.vehicle.config.VehicleConfiguration;
import com.amazon.sampleapp.core.AuthorizationHandlerFactoryInterface;
import com.amazon.sampleapp.core.AuthorizationHandlerObserverInterface;
import com.amazon.sampleapp.core.LoggerControllerInterface;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;
import com.amazon.sampleapp.impl.AddressBook.AddressBookHandler;
import com.amazon.sampleapp.impl.Alerts.AlertsHandler;
import com.amazon.sampleapp.impl.AlexaClient.AlexaClientHandler;
import com.amazon.sampleapp.impl.AlexaSpeaker.AlexaSpeakerHandler;
import com.amazon.sampleapp.impl.Audio.AudioInputProviderHandler;
import com.amazon.sampleapp.impl.Audio.AudioOutputProviderHandler;
import com.amazon.sampleapp.impl.AudioPlayer.AudioPlayerHandler;
import com.amazon.sampleapp.impl.Authorization.AuthorizationHandler;
import com.amazon.sampleapp.impl.Authorization.CBLAuthorizationHandler;
import com.amazon.sampleapp.impl.CarControl.CarControlDataProvider;
import com.amazon.sampleapp.impl.CarControl.CarControlHandler;
import com.amazon.sampleapp.impl.DoNotDisturb.DoNotDisturbHandler;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerConfiguration;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.ExternalMediaPlayer.MACCPlayer;
import com.amazon.sampleapp.impl.GlobalPreset.GlobalPresetHandler;
import com.amazon.sampleapp.impl.LocalMediaSource.AMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.BluetoothLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.CDLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.DABLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.FMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.LineInLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.SatelliteLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.SiriusXMLocalMediaSource;
import com.amazon.sampleapp.impl.LocalMediaSource.USBLocalMediaSource;
import com.amazon.sampleapp.impl.LocationProvider.LocationProviderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerFragment;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.Messaging.MessagingHandler;
import com.amazon.sampleapp.impl.Navigation.NavigationHandler;
import com.amazon.sampleapp.impl.NetworkInfoProvider.NetworkInfoProviderHandler;
import com.amazon.sampleapp.impl.Notifications.NotificationsHandler;
import com.amazon.sampleapp.impl.PhoneCallController.PhoneCallControllerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;
import com.amazon.sampleapp.impl.PropertyManager.PropertyManagerHandler;
import com.amazon.sampleapp.impl.SpeechRecognizer.SpeechRecognizerHandler;
import com.amazon.sampleapp.impl.SpeechSynthesizer.SpeechSynthesizerHandler;
import com.amazon.sampleapp.impl.TemplateRuntime.TemplateRuntimeHandler;
import com.amazon.sampleapp.impl.TextToSpeech.TextToSpeechHandler;
import com.amazon.sampleapp.logView.LogEntry;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.UUID;

public class MainActivity extends AppCompatActivity implements SampleAppContext, Observer {
    private static final String TAG = MainActivity.class.getSimpleName();

    private static final int sPermissionRequestCode = 0;
    private static final String[] sRequiredPermissions = {Manifest.permission.RECORD_AUDIO,
            Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.READ_EXTERNAL_STORAGE};

    // AVS-supported locales: https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/system.html#locales
    private static final String[] sSupportedLocales = {"de-DE", "en-AU", "en-CA", "en-GB", "en-IN", "en-US", "es-ES",
            "es-MX", "es-US", "fr-CA", "fr-FR", "hi-IN", "it-IT", "ja-JP", "pt-BR", "en-CA/fr-CA", "en-IN/hi-IN",
            "en-US/es-US", "es-US/en-US", "fr-CA/en-CA", "hi-IN/en-IN"};

    /* AACE Platform Interface Handlers */

    // Alexa
    private AlertsHandler mAlerts;
    private AlexaClientHandler mAlexaClient;
    private AudioPlayerHandler mAudioPlayer;
    private AuthorizationHandler mAuthorizationHandler;
    private EqualizerControllerHandler mEqualizerControllerHandler;
    private NotificationsHandler mNotifications;
    private PhoneCallControllerHandler mPhoneCallController;
    private AddressBookHandler mAddressBook;
    private PlaybackControllerHandler mPlaybackController;
    private SpeechRecognizerHandler mSpeechRecognizer;
    private SpeechSynthesizerHandler mSpeechSynthesizer;
    private TemplateRuntimeHandler mTemplateRuntime;
    private AlexaSpeakerHandler mAlexaSpeaker;
    private DoNotDisturbHandler mDoNotDisturb;

    // Core
    private Engine mEngine;
    private boolean mEngineStarted = false;

    // Audio
    private AudioInputProviderHandler mAudioInputProvider;
    private AudioOutputProviderHandler mAudioOutputProvider;

    // Car Control
    private CarControlHandler mCarControlHandler;

    // Location
    private LocationProviderHandler mLocationProvider;

    // Logger
    private LoggerHandler mLogger;

    // Messaging
    private MessagingHandler mMessaging;

    // Navigation
    private NavigationHandler mNavigation;

    // Network
    private NetworkInfoProviderHandler mNetworkInfoProvider;

    // Property Manager
    private PropertyManagerHandler mPropertyManager;

    // TextToSpeech
    private TextToSpeechHandler mTextToSpeech;

    /* Tab Fragment Adapter */
    private TabFragmentAdapter mAdapter;

    /* Log View Components */
    private LoggerFragment mLoggerFragment;

    /* Shared Preferences */
    private SharedPreferences mPreferences;

    /* View Pager */
    private ViewPager mViewPager;

    private Map<String, String> mContextData = new HashMap<>();

    /* Speech Recognition Components */
    private boolean mIsTalkButtonLongPressed = false;

    private MediaPlayer mAudioCueStartVoice; // Voice-initiated listening audio cue
    private MediaPlayer mAudioCueStartTouch; // Touch-initiated listening audio cue
    private MediaPlayer mAudioCueEnd; // End of listening audio cue
    private MACCPlayer mMACCPlayer;

    private CDLocalMediaSource mCDLocalMediaSource;
    private DABLocalMediaSource mDABLocalMediaSource;
    private SiriusXMLocalMediaSource mSIRIUSXMLocalMediaSource;
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

    // Timezone
    private final List<String> timezoneArray = new ArrayList<String>(Arrays.asList("America/Vancouver",
            "America/Edmonton", "America/Winnipeg", "America/Toronto", "America/Halifax", "America/St_Johns"));
    ArrayAdapter<String> timezoneAdapter;
    /* AutoVoiceChrome Controller */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Check if permissions are missing and must be requested
        ArrayList<String> requests = new ArrayList<>();

        for (String permission : sRequiredPermissions) {
            if (ActivityCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_DENIED) {
                requests.add(permission);
            }
        }

        // Request necessary permissions if not already granted, else start app
        if (requests.size() > 0) {
            ActivityCompat.requestPermissions(
                    this, requests.toArray(new String[requests.size()]), sPermissionRequestCode);
        } else
            create();
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == sPermissionRequestCode) {
            if (grantResults.length > 0) {
                for (int grantResult : grantResults) {
                    if (grantResult == PackageManager.PERMISSION_DENIED) {
                        // Permission request was denied
                        Toast.makeText(this, "Permissions required", Toast.LENGTH_LONG).show();
                    }
                }
                // Permissions have been granted. Start app
                create();
            } else {
                // Permission request was denied
                Toast.makeText(this, "Permissions required", Toast.LENGTH_LONG).show();
            }
        }
    }

    // SampleAppContext

    @Override
    public Activity getActivity() {
        return this;
    }

    @Override
    public AudioOutputProvider getAudioOutputProvider() {
        return mAudioOutputProvider;
    }

    @Override
    public LoggerControllerInterface getLoggerController() {
        return mLogger;
    }

    @Override
    public SpeechRecognizer getSpeechRecognizer() {
        return mSpeechRecognizer;
    }

    @Override
    public PropertyManager getPropertyManager() {
        return (PropertyManager) mPropertyManager;
    }

    @Override
    public ViewPager getViewPager() {
        return mViewPager;
    }

    @Override
    public String getData(String key) {
        return mContextData.get(key);
    }

    @Override
    public JSONObject getConfigFromFile(String configAssetName, String configRootKey) {
        JSONObject config = FileUtils.getOptionalConfigFromSDCard(configAssetName, configRootKey);
        if (config != null) {
            Log.i(TAG, "Got " + configRootKey + " from config file on the SD Card");
            return config;
        }
        return FileUtils.getConfigFromFile(getAssets(), configAssetName, configRootKey);
    }

    public static class TabFragmentAdapter extends FragmentPagerAdapter {
        ArrayList<Fragment> mFragments;

        public TabFragmentAdapter(FragmentManager fm) {
            super(fm);
            mFragments = new ArrayList<Fragment>();
        }

        @Override
        public int getCount() {
            return mFragments.size();
        }

        @Override
        public Fragment getItem(int position) {
            return mFragments.get(position);
        }

        public void addFragment(Fragment fragment) {
            mFragments.add(fragment);
        }
    }

    private void create() {
        // Set the main view content
        setContentView(R.layout.activity_main);

        // Initialize LVCInteractionService to start LVC, if supported
        initLVC();

        // Add support action toolbar for action buttons
        setSupportActionBar((Toolbar) findViewById(R.id.actionToolbar));

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.viewpager);

        mAdapter = new TabFragmentAdapter(getSupportFragmentManager());
        mLoggerFragment = new LoggerFragment(this);

        mAdapter.addFragment(mLoggerFragment);
        mViewPager.setAdapter(mAdapter);

        TabLayout tabLayout = (TabLayout) findViewById(R.id.tablayout);
        tabLayout.setupWithViewPager(mViewPager);

        // Initialize sound effects for speech recognition
        mAudioCueStartVoice = MediaPlayer.create(this, R.raw.med_ui_wakesound);
        mAudioCueStartTouch = MediaPlayer.create(this, R.raw.med_ui_wakesound_touch);
        mAudioCueEnd = MediaPlayer.create(this, R.raw.med_ui_endpointing_touch);

        // Get shared preferences
        mPreferences = getSharedPreferences(getString(R.string.preference_file_key), Context.MODE_PRIVATE);

        // Retrieve device config from config file and update preferences
        String clientId = "", productId = "", productDsn = "";

        JSONObject config = SampleApplication.getConfig(this, "config");
        if (config != null) {
            try {
                clientId = config.getString("clientId");
                productId = config.getString("productId");
            } catch (JSONException e) {
                Log.w(TAG, "Missing device info in app_config.json");
            }
            try {
                productDsn = config.getString("productDsn");
            } catch (JSONException e) {
                try {
                    // set Android ID as product DSN
                    productDsn = Settings.Secure.getString(getContentResolver(), Settings.Secure.ANDROID_ID);
                    Log.i(TAG, "android id for DSN: " + productDsn);
                } catch (Error error) {
                    productDsn = UUID.randomUUID().toString();
                    Log.w(TAG, "android id not found, generating random DSN: " + productDsn);
                }
            }
        }
        updateDevicePreferences(clientId, productId, productDsn);

        // Display device config settings in GUI
        updateDeviceConfigGUI(clientId, productId, productDsn);

        setupCallOnDestroyButton();
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
        } catch (RuntimeException e) {
            Log.e(TAG, "Could not start engine. Reason: " + e.getMessage());
            return;
        }

        // Observe log event changes to update the log view
        mLogger.addLogObserver(this);
        mSpeechRecognizer.addObserver(this);
    }

    /**
     * Configure the Engine and register platform interface instances
     * @param json JSON string with LVC config if LVC is supported, null otherwise.
     * @throws RuntimeException
     */
    private void startEngine(String json) throws RuntimeException {
        // Create an "appdata" subdirectory in the cache directory for storing application data
        File cacheDir = getCacheDir();
        File appDataDir = new File(cacheDir, "appdata");
        File sampleDataDir = new File(cacheDir, "sampledata");

        // Copy certs from assets to certs subdirectory of cache directory
        File certsDir = new File(appDataDir, "certs");
        FileUtils.copyAllAssets(getAssets(), "certs", certsDir, false);

        // Copy models from assets to certs subdirectory of cache directory.
        // Force copy the models on every start so that the models on device cache are always the latest
        // from the APK
        File modelsDir = new File(appDataDir, "models");
        FileUtils.copyAllAssets(getAssets(), "models", modelsDir, true);

        copyAsset("Contacts.json", new File(sampleDataDir, "Contacts.json"), false);
        copyAsset("NavigationFavorites.json", new File(sampleDataDir, "NavigationFavorites.json"), false);
        copyAsset("ConversationsReport.json", new File(sampleDataDir, "ConversationsReport.json"), false);

        // Create AAC engine
        mEngine = Engine.create(this);
        ArrayList<EngineConfiguration> configuration = getEngineConfigurations(json, appDataDir, certsDir, modelsDir);

        // Get extra module factories and add configurations

        mContextData.put(SampleAppContext.CERTS_DIR, certsDir.getPath());
        mContextData.put(SampleAppContext.MODEL_DIR, modelsDir.getPath());
        mContextData.put(
                SampleAppContext.PRODUCT_DSN, mPreferences.getString(getString(R.string.preference_product_dsn), ""));
        mContextData.put(SampleAppContext.APPDATA_DIR, appDataDir.getPath());
        mContextData.put(SampleAppContext.JSON, json);

        List<ModuleFactoryInterface> extraFactories = getExtraModuleFactory();
        configExtraModules(this, extraFactories, configuration);

        EngineConfiguration[] configurationArray = configuration.toArray(new EngineConfiguration[configuration.size()]);
        boolean configureSucceeded = mEngine.configure(configurationArray);
        if (!configureSucceeded)
            throw new RuntimeException("Engine configuration failed");

        // Create the platform implementation handlers and register them with the engine
        // Logger
        if (!mEngine.registerPlatformInterface(mLogger = new LoggerHandler()))
            throw new RuntimeException("Could not register Logger platform interface");

        // AlexaClient
        if (!mEngine.registerPlatformInterface(mAlexaClient = new AlexaClientHandler(this, mLogger)))
            throw new RuntimeException("Could not register AlexaClient platform interface");

        // AudioInputProvider
        if (!mEngine.registerPlatformInterface(mAudioInputProvider = new AudioInputProviderHandler(this, mLogger)))
            throw new RuntimeException("Could not register AudioInputProvider platform interface");

        // EqualizerController
        if (!mEngine.registerPlatformInterface(
                    mEqualizerControllerHandler = new EqualizerControllerHandler(this, mLogger)))
            throw new RuntimeException("Could not register EqualizerController platform interface");

        // AudioOutputProvider
        if (!mEngine.registerPlatformInterface(mAudioOutputProvider = new AudioOutputProviderHandler(
                                                       this, mLogger, mAlexaClient, mEqualizerControllerHandler)))
            throw new RuntimeException("Could not register AudioOutputProvider platform interface");

        // LocationProvider
        if (!mEngine.registerPlatformInterface(mLocationProvider = new LocationProviderHandler(this, mLogger)))
            throw new RuntimeException("Could not register LocationProvider platform interface");

        // Messaging sample datafile
        String conversationsDataPath = sampleDataDir.getPath() + "/ConversationsReport.json";

        // Always use sample data from external storage if available
        File sampleConversationsFile =
                new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/ConversationsReport.json");

        if (sampleConversationsFile.exists()) {
            conversationsDataPath = sampleConversationsFile.getPath();
        }

        // Messaging
        if (!mEngine.registerPlatformInterface(mMessaging = new MessagingHandler(this, mLogger, conversationsDataPath)))
            throw new RuntimeException("Could not register MessagingController platform interface");

        // PhoneCallController
        if (!mEngine.registerPlatformInterface(mPhoneCallController = new PhoneCallControllerHandler(this, mLogger)))
            throw new RuntimeException("Could not register PhoneCallController platform interface");

        // PlaybackController
        if (!mEngine.registerPlatformInterface(mPlaybackController = new PlaybackControllerHandler(this, mLogger)))
            throw new RuntimeException("Could not register PlaybackController platform interface");

        // PropertyManager
        if (!mEngine.registerPlatformInterface(mPropertyManager = new PropertyManagerHandler(this, mLogger)))
            throw new RuntimeException("Could not register PropertyManager platform interface");

        // SpeechRecognizer
        // Note : Expects PropertyManager to be not null.
        if (!mEngine.registerPlatformInterface(
                    mSpeechRecognizer = new SpeechRecognizerHandler(this, mLogger, mPropertyManager)))
            throw new RuntimeException("Could not register SpeechRecognizer platform interface");

        // AudioPlayer
        if (!mEngine.registerPlatformInterface(mAudioPlayer = new AudioPlayerHandler(mLogger)))
            throw new RuntimeException("Could not register AudioPlayer platform interface");

        // SpeechSynthesizer
        if (!mEngine.registerPlatformInterface(mSpeechSynthesizer = new SpeechSynthesizerHandler()))
            throw new RuntimeException("Could not register SpeechSynthesizer platform interface");

        // TemplateRuntime
        if (!mEngine.registerPlatformInterface(
                    mTemplateRuntime = new TemplateRuntimeHandler(mLogger, mPlaybackController)))
            throw new RuntimeException("Could not register TemplateRuntime platform interface");

        // AlexaSpeaker
        if (!mEngine.registerPlatformInterface(mAlexaSpeaker = new AlexaSpeakerHandler(this, mLogger)))
            throw new RuntimeException("Could not register AlexaSpeaker platform interface");

        // Alerts
        if (!mEngine.registerPlatformInterface(mAlerts = new AlertsHandler(this, mLogger)))
            throw new RuntimeException("Could not register Alerts platform interface");

        // NetworkInfoProvider
        if (!mEngine.registerPlatformInterface(
                    mNetworkInfoProvider = new NetworkInfoProviderHandler(this, mLogger, mPropertyManager)))
            throw new RuntimeException("Could not register NetworkInfoProvider platform interface");

        // Authorization
        if (!mEngine.registerPlatformInterface(mAuthorizationHandler = new AuthorizationHandler(this, mLogger)))
            throw new RuntimeException("Could not register Authorization platform interface");

        JSONObject deviceConfig = new JSONObject();
        try {
            deviceConfig.put("productId", mPreferences.getString(getString(R.string.preference_product_id), ""));
            deviceConfig.put("productDsn", mPreferences.getString(getString(R.string.preference_product_dsn), ""));
        } catch (JSONException e) {
            throw new RuntimeException("Cloud not create a device json");
        }

        AuthorizationHandlerObserverInterface cblAuth = new CBLAuthorizationHandler(this, mLogger);
        cblAuth.initialize(mAuthorizationHandler, deviceConfig);

        List<AuthorizationHandlerFactoryInterface> extraAuthorizationModuleFactories =
                getExtraAuthorizationHandlerFactory();
        loadAuthorizationHandlerUIAndRegister(extraAuthorizationModuleFactories, this);

        // Navigation
        if (!mEngine.registerPlatformInterface(mNavigation = new NavigationHandler(this, mLogger)))
            throw new RuntimeException("Could not register Navigation platform interface");

        // Notifications
        if (!mEngine.registerPlatformInterface(mNotifications = new NotificationsHandler(this, mLogger))) {
            throw new RuntimeException("Could not register Notifications platform interface");
        }

        // DoNotDisturb
        if (!mEngine.registerPlatformInterface(mDoNotDisturb = new DoNotDisturbHandler(this, mLogger)))
            throw new RuntimeException("Could not register DoNotDisturb platform interface");
        else
            mAlexaClient.registerAuthStateObserver(mDoNotDisturb);

        // TextToSpeech
        if (!mEngine.registerPlatformInterface(mTextToSpeech = new TextToSpeechHandler(this, mLogger)))
            throw new RuntimeException("Could not register TextToSpeech platform interface");

        // Contacts/NavigationFavorites
        String sampleContactsDataPath = sampleDataDir.getPath() + "/Contacts.json";
        String sampleNavigationFavoritesDataPath = sampleDataDir.getPath() + "/NavigationFavorites.json";

        // Always use sample data from external storage if available
        File sampleContactsFile =
                new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/Contacts.json");
        File sampleNavigationFavoritesFile =
                new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/NavigationFavorites.json");

        if (sampleContactsFile.exists()) {
            sampleContactsDataPath = sampleContactsFile.getPath();
        }

        if (sampleNavigationFavoritesFile.exists()) {
            sampleNavigationFavoritesDataPath = sampleNavigationFavoritesFile.getPath();
        }

        if (!mEngine.registerPlatformInterface(mAddressBook = new AddressBookHandler(this, mLogger,
                                                       sampleContactsDataPath, sampleNavigationFavoritesDataPath)))
            throw new RuntimeException("Could not register AddressBook platform interface");

        // AlexaComms Handler

        // LVC Handlers
        if (!mEngine.registerPlatformInterface(mCarControlHandler = new CarControlHandler(this, mLogger))) {
            throw new RuntimeException("Could not register Car Control platform interface");
        }

        mMACCPlayer = new MACCPlayer(this, mLogger);
        if (!mEngine.registerPlatformInterface(mMACCPlayer)) {
            Log.i("MACC", "registration failed");
            throw new RuntimeException("Could not register external media player platform interface");
        } else {
            Log.i("MACC", "registration succeeded");
        }
        mMACCPlayer.runDiscovery();

        // SiriusXM is disabled by default and enabled by setting mockSiriusXM to true in the
        // config file. For example:
        // {
        //   "lms": {
        //     "mockSiriusXM": true
        //   }
        // }
        boolean mockSiriusXM = false; // SiriusXM disabled by default
        JSONObject lmsConfig = SampleApplication.getConfig(this, "lms");
        if (lmsConfig != null) {
            Log.i(TAG, "Using LMS config from file on SD card");
            mockSiriusXM = lmsConfig.optBoolean("mockSiriusXM", mockSiriusXM);
        }

        // Mock CD platform handler
        if (!mEngine.registerPlatformInterface(mCDLocalMediaSource = new CDLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock CD player Local Media Source platform interface");

        // Mock DAB platform handler
        if (!mEngine.registerPlatformInterface(mDABLocalMediaSource = new DABLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock DAB player Local Media Source platform interface");

        // Mock AM platform handler
        if (!mEngine.registerPlatformInterface(mAMLocalMediaSource = new AMLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock AM radio player Local Media Source platform interface");

        // Mock SIRIUSXM platform handler
        if (mockSiriusXM
                && !mEngine.registerPlatformInterface(
                        mSIRIUSXMLocalMediaSource = new SiriusXMLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock SIRIUSXM player Local Media Source platform interface");

        // Mock FM platform handler
        if (!mEngine.registerPlatformInterface(mFMLocalMediaSource = new FMLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock FM radio player Local Media Source platform interface");

        // Mock Bluetooth platform handler
        if (!mEngine.registerPlatformInterface(mBTLocalMediaSource = new BluetoothLocalMediaSource(mLogger)))
            throw new RuntimeException(
                    "Could not register Mock Bluetooth player Local Media Source platform interface");

        // Mock Line In platform handler
        if (!mEngine.registerPlatformInterface(mLILocalMediaSource = new LineInLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock Line In player Local Media Source platform interface");

        // Mock Satellite Radio platform handler
        if (!mEngine.registerPlatformInterface(mSATRADLocalMediaSource = new SatelliteLocalMediaSource(mLogger)))
            throw new RuntimeException(
                    "Could not register Mock Satellite radio player Local Media Source platform interface");

        // Mock USB platform handler
        if (!mEngine.registerPlatformInterface(mUSBLocalMediaSource = new USBLocalMediaSource(mLogger)))
            throw new RuntimeException("Could not register Mock USB player Local Media Source platform interface");

        // Mock global preset
        if (!mEngine.registerPlatformInterface(mGlobalPresetHandler = new GlobalPresetHandler(this, mLogger)))
            throw new RuntimeException("Could not register Mock Global Preset platform interface");

        // Register extra modules
        loadPlatformInterfacesAndLoadUI(mEngine, extraFactories, this);

        // Alexa Locale
        ArrayAdapter<String> localeAdapter =
                new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sSupportedLocales);

        Spinner spinnerView = (Spinner) findViewById(R.id.locale_spinner);
        spinnerView.setAdapter(localeAdapter);

        final String defaultLocale = mPropertyManager.getProperty(AlexaProperties.LOCALE);

        int localePosition = localeAdapter.getPosition(defaultLocale);
        if (localePosition < 0) {
            Log.e(TAG, defaultLocale + " is not in the Supported Locales");
            localePosition = 0;
        }
        spinnerView.setSelection(localePosition);

        spinnerView.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1, int position, long arg3) {
                String s = sSupportedLocales[position];
                if (!mPropertyManager.getProperty(AlexaProperties.LOCALE).equals(s)) {
                    Toast.makeText(MainActivity.this, "Switching Alexa locale to " + s, Toast.LENGTH_SHORT).show();

                    mPropertyManager.setProperty(AlexaProperties.LOCALE, s);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub
            }
        });

        // Timezone
        setUpTimeZoneUI();

        // Start the engine
        if (!mEngine.start())
            throw new RuntimeException("Could not start engine");
        mEngineStarted = true;

        // Check if Amazonlite is supported
        if (mPropertyManager.getProperty(AlexaProperties.WAKEWORD_SUPPORTED).equals("true")) {
            mSpeechRecognizer.enableWakeWordUI();
        }
        mLogger.postInfo("Wakeword supported: ", mPropertyManager.getProperty(AlexaProperties.WAKEWORD_SUPPORTED));

        // log whether LocationProvider gave a supported country
        mLogger.postInfo("Country Supported: ", mPropertyManager.getProperty(AlexaProperties.COUNTRY_SUPPORTED));

        // Initialize AutoVoiceChrome

        mAddressBook.onInitialize();
        initTapToTalk();
        initEarconsSettings();
    }

    public void updateTimezoneSpinner(String timezone) {
        runOnUiThread(new Runnable() {
            public void run() {
                if (!timezoneArray.contains(timezone)) {
                    timezoneArray.add(timezone);
                    final String[] timezones = new String[timezoneArray.size()];
                    Spinner spinnerView = (Spinner) findViewById(R.id.timezone_spinner);
                    timezoneAdapter = new ArrayAdapter<String>(
                            MainActivity.this, android.R.layout.simple_spinner_item, timezoneArray.toArray(timezones));
                    spinnerView.setAdapter(timezoneAdapter);
                    int timezonePosition = timezoneAdapter.getPosition(timezone);
                    spinnerView.setSelection(timezonePosition);
                }
            }
        });
    }

    private void setupCallOnDestroyButton() {
        TextView callOnDestroyButton = findViewById(R.id.callOnDestroy_button);
        callOnDestroyButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }

    /**
     * Get the configurations to start the Engine
     * @param json JSON string with LVC config if LVC is supported, null otherwise.
     * @param appDataDir path to app's data directory
     * @param certsDir path to certificates directory
     * @return List of Engine configurations
     */
    private ArrayList<EngineConfiguration> getEngineConfigurations(
            String json, File appDataDir, File certsDir, File modelsDir) {
        // Configure the engine
        String productDsn = mPreferences.getString(getString(R.string.preference_product_dsn), "");
        String clientId = mPreferences.getString(getString(R.string.preference_client_id), "");
        String productId = mPreferences.getString(getString(R.string.preference_product_id), "");

        AlexaConfiguration.TemplateRuntimeTimeout[] timeoutList = new AlexaConfiguration.TemplateRuntimeTimeout[] {
                new AlexaConfiguration.TemplateRuntimeTimeout(
                        AlexaConfiguration.TemplateRuntimeTimeoutType.DISPLAY_CARD_TTS_FINISHED_TIMEOUT, 8000),
                new AlexaConfiguration.TemplateRuntimeTimeout(
                        AlexaConfiguration.TemplateRuntimeTimeoutType.DISPLAY_CARD_AUDIO_PLAYBACK_FINISHED_TIMEOUT,
                        8000),
                new AlexaConfiguration.TemplateRuntimeTimeout(
                        AlexaConfiguration.TemplateRuntimeTimeoutType
                                .DISPLAY_CARD_AUDIO_PLAYBACK_STOPPED_PAUSED_TIMEOUT,
                        1800000)};

        JSONObject config = null;
        ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
                // AlexaConfiguration.createCurlConfig( certsDir.getPath(), "wlan0" ), Uncomment this line to specify
                // the interface name to use by AVS.
                AlexaConfiguration.createCurlConfig(certsDir.getPath()),
                AlexaConfiguration.createDeviceInfoConfig(
                        productDsn, clientId, productId, "Alexa Auto SDK", "Android Sample App"),
                AlexaConfiguration.createMiscStorageConfig(appDataDir.getPath() + "/miscStorage.sqlite"),
                AlexaConfiguration.createCertifiedSenderConfig(appDataDir.getPath() + "/certifiedSender.sqlite"),
                AlexaConfiguration.createCapabilitiesDelegateConfig(
                        appDataDir.getPath() + "/capabilitiesDelegate.sqlite"),
                AlexaConfiguration.createAlertsConfig(appDataDir.getPath() + "/alerts.sqlite"),
                AlexaConfiguration.createNotificationsConfig(appDataDir.getPath() + "/notifications.sqlite"),
                AlexaConfiguration.createDeviceSettingsConfig(appDataDir.getPath() + "/deviceSettings.sqlite"),
                AlexaConfiguration.createEqualizerControllerConfig(EqualizerConfiguration.getSupportedBands(),
                        EqualizerConfiguration.getMinBandLevel(), EqualizerConfiguration.getMaxBandLevel(),
                        EqualizerConfiguration.getDefaultBandLevels()),
                // Uncomment the below line to specify the speaker manager values
                // AlexaConfiguration.createSpeakerManagerConfig( true ),
                // Uncomment the below line to specify the template runtime values
                // AlexaConfiguration.createTemplateRuntimeTimeoutConfig( timeoutList ),
                StorageConfiguration.createLocalStorageConfig(appDataDir.getPath() + "/localStorage.sqlite"),

                // Create the optional navigation provider config name
                NavigationConfiguration.createNavigationConfig("HERE"),

                // Example Vehicle Config
                VehicleConfiguration.createVehicleInfoConfig(new VehicleConfiguration.VehicleProperty[] {
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.MAKE, "Amazon"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.MODEL, "AmazonCarOne"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.TRIM, "Advance"),
                        new VehicleConfiguration.VehicleProperty(VehicleConfiguration.VehiclePropertyType.YEAR, "2025"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.GEOGRAPHY, "US"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.VERSION, "1.0.0"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.OPERATING_SYSTEM,
                                "Android 8.1 Oreo API Level 26"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.HARDWARE_ARCH, "Armv8a"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.LANGUAGE, "en-US"),
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.MICROPHONE, "Single, roof mounted"),
                        // If this list is left blank, it will be fetched by the engine using amazon default endpoint
                        new VehicleConfiguration.VehicleProperty(
                                VehicleConfiguration.VehiclePropertyType.VEHICLE_IDENTIFIER, "123456789a")})));

        String endpointConfigPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/aace.json";
        if (new File(endpointConfigPath).exists()) {
            EngineConfiguration alexaEndpointsConfig = ConfigurationFile.create(
                    Environment.getExternalStorageDirectory().getAbsolutePath() + "/aace.json");
            configuration.add(alexaEndpointsConfig);
            Log.i("getEngineConfigurations", "Overriding endpoints");
        }

        // Provide a car control configuration to the Engine.
        //
        // We check if a car control configuration file (CarControlConfig.json) is on the SD
        // card to override the default configuration generated by
        // CarControlDataProvider.generateCarControlConfig(), else use the default. This logic to
        // conditionally generate a config or use a file would not ordinarily be required in a
        // typical application since the config that an application uses will be known and stable.
        // However, since this sample application allows facilitating testing by overriding the
        // default car control configuration generated by the application itself, we check if such
        // an override is being used.
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String externalCarControlConfigPath = sdCardPath + "/CarControlConfig.json";
        File carControlConfigFile = new File(externalCarControlConfigPath);
        if (carControlConfigFile.exists()) {
            Log.i(TAG, "Using car control config from file on SD card");
            EngineConfiguration carControlConfig = ConfigurationFile.create(carControlConfigFile.getPath());
            configuration.add(carControlConfig);

            // If application is using an external car control configuration file, then the CarControlDataProvider
            // needs to generate initial values by scanning the file and building a model of the power, toggle, range
            // and mode controllers defined.
            try {
                CarControlDataProvider.initialize(carControlConfigFile.getPath());
            } catch (Exception e) {
                Context context = getApplicationContext();
                int duration = Toast.LENGTH_LONG;
                Toast toast = Toast.makeText(context, e.getMessage(), duration);
                toast.show();
            }
        } else {
            // Use programmatic generation of car control configuration. The corresponding custom
            // assets file that complements the default generated car control config is in the
            // assets directory, and LVCInteractionService will take care of ensuring it is used.
            EngineConfiguration ccConfig = CarControlDataProvider.generateCarControlConfig();
            configuration.add(ccConfig);
        }

        return configuration;
    }

    @Override
    public void onDestroy() {
        if (mLogger != null)
            mLogger.postInfo(TAG, "Engine stopped");
        else
            Log.i(TAG, "Engine stopped");

        if (mAudioCueStartVoice != null) {
            mAudioCueStartVoice.release();
            mAudioCueStartVoice = null;
        }
        if (mAudioCueStartTouch != null) {
            mAudioCueStartTouch.release();
            mAudioCueStartTouch = null;
        }
        if (mAudioCueEnd != null) {
            mAudioCueEnd.release();
            mAudioCueEnd = null;
        }

        if (mLVCConfigReceiver != null) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mLVCConfigReceiver);
        }

        if (mNetworkInfoProvider != null) {
            mNetworkInfoProvider.unregister();
        }

        if (mMACCPlayer != null) {
            mMACCPlayer.cleanupMACCClient();
        }

        // cleanup for restarting sample app while using LVE
        if (mAddressBook != null) {
            mAddressBook.removeAllAddressBooks();
        }

        if (mEngine != null) {
            mEngine.dispose();
        }

        // AutoVoiceChrome cleanup

        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.menu_main, menu);

        // Set tap-to-talk and hold-to-talk actions
        mTapToTalkIcon = menu.findItem(R.id.action_talk);
        initTapToTalk();
        return true;
    }

    private void initTapToTalk() {
        if (mTapToTalkIcon != null && mAlexaClient != null && mSpeechRecognizer != null) {
            mTapToTalkIcon.setActionView(R.layout.menu_item_talk);

            // Set hold-to-talk action
            mTapToTalkIcon.getActionView().setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mSpeechRecognizer.onTapToTalk();
                }
            });

            // Start hold-to-talk button action
            mTapToTalkIcon.getActionView().setOnLongClickListener(new View.OnLongClickListener() {
                @Override
                public boolean onLongClick(View v) {
                    mIsTalkButtonLongPressed = true;
                    mSpeechRecognizer.onHoldToTalk();
                    return true;
                }
            });

            // Release hold-to-talk button action
            mTapToTalkIcon.getActionView().setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent m) {
                    // Talk button released
                    if (m.getAction() == MotionEvent.ACTION_UP) {
                        if (mIsTalkButtonLongPressed) {
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
        final View toggleStartOfRequestItem = findViewById(R.id.toggleEarconsStartOfRequest);
        final SwitchCompat startOfRequestSwitch = toggleStartOfRequestItem.findViewById(R.id.drawerSwitch);
        startOfRequestSwitch.setChecked(true);
        startOfRequestSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                toggleStartOfRequestState(isChecked);
            }
        });

        // Switch to toggle end of request
        final View toggleEndtOfRequestItem = findViewById(R.id.toggleEarconsEndOfRequest);
        final SwitchCompat endOfRequestSwitch = toggleEndtOfRequestItem.findViewById(R.id.drawerSwitch);
        endOfRequestSwitch.setChecked(true);
        endOfRequestSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                toggleEndOfRequestState(isChecked);
            }
        });
    }

    private void toggleEndOfRequestState(boolean isChecked) {
        synchronized (mDisableEndOfRequestEarconLock) {
            if (isChecked)
                mDisableEndOfRequestEarcon = false;
            else
                mDisableEndOfRequestEarcon = true;
        }
    }

    private void toggleStartOfRequestState(boolean isChecked) {
        synchronized (mDisableStartOfRequestEarconLock) {
            if (isChecked)
                mDisableStartOfRequestEarcon = false;
            else
                mDisableStartOfRequestEarcon = true;
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_drawer:
                toggleDrawer();
                return true;
            case R.id.action_stop:
                mAlexaClient.stopForegroundActivity();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void toggleDrawer() {
        final View drawer = findViewById(R.id.drawer);
        if (drawer.getVisibility() == View.VISIBLE) {
            drawer.setVisibility(View.GONE);
        } else {
            drawer.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void update(Observable observable, Object object) {
        if (observable instanceof LoggerHandler.LoggerObservable) {
            if (object instanceof LogEntry) {
                final LogEntry entry = (LogEntry) object;
                runOnUiThread(new Runnable() {
                    public void run() {
                        // Insert log entry into log view
                        mLoggerFragment.insertItem(entry);
                    }
                });
            }
        } else if (observable instanceof SpeechRecognizerHandler.AudioCueObservable) {
            if (object.equals(SpeechRecognizerHandler.AudioCueState.START_TOUCH)) {
                synchronized (mDisableStartOfRequestEarconLock) {
                    if (!mDisableStartOfRequestEarcon) {
                        // Play touch-initiated listening audio cue
                        mAudioCueStartTouch.start();
                    }
                }
            } else if (object.equals(SpeechRecognizerHandler.AudioCueState.START_VOICE)) {
                synchronized (mDisableStartOfRequestEarconLock) {
                    if (!mDisableStartOfRequestEarcon) {
                        // Play voice-initiated listening audio cue
                        mAudioCueStartVoice.start();
                    }
                }
            } else if (object.equals(SpeechRecognizerHandler.AudioCueState.END)) {
                synchronized (mDisableEndOfRequestEarconLock) {
                    if (!mDisableEndOfRequestEarcon) {
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

    private void updateDevicePreferences(String clientId, String productId, String productDsn) {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putString(getString(R.string.preference_client_id), clientId);
        editor.putString(getString(R.string.preference_product_id), productId);
        editor.putString(getString(R.string.preference_product_dsn), productDsn);
        editor.apply();
    }

    private void updateDeviceConfigGUI(final String clientId, final String productId, final String productDsn) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((TextView) findViewById(R.id.clientId)).setText(clientId);
                ((TextView) findViewById(R.id.productId)).setText(productId);
                ((TextView) findViewById(R.id.productDsn)).setText(productDsn);
            }
        });
    }

    // Auto Voice Chrome initialize function

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
                    Log.e(TAG, "Failed to init LVC: " + reason);
                } else if (intent.hasExtra(LVCInteractionService.LVC_RECEIVER_CONFIGURATION)) {
                    // LVCInteractionService received config from LVC
                    Log.i(TAG, "Received config from LVC, starting engine now");
                    String config = intent.getStringExtra(LVCInteractionService.LVC_RECEIVER_CONFIGURATION);
                    onLVCConfigReceived(config);
                }
            }
        }
    }

    /**
     * Retrieves a list of factory classes implementing @c ModuleFactoryInterface by
     * iterating through the json files that are located under the sample-app folder of assets
     * directory. The json contains the fully qualified class name of the factory class.
     * The expected json format:
     *
     * @code{.json}
     * {
     *
     *   "factory": {
     *     "name": "fully-qualified-class-name-implementing-ModuleFactoryInterface"
     *   }
     *
     * }
     * @endcode
     */
    private List<ModuleFactoryInterface> getExtraModuleFactory() {
        List<ModuleFactoryInterface> extraModuleFactories = new ArrayList<>();
        try {
            String folderName = "sample-app";
            String factoryKey = "factory";
            String category = "name";
            String[] fileList = getAssets().list(folderName);
            Log.i(TAG, "getExtraModuleFactory: begin loading");
            for (String f : fileList) {
                InputStream is = getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONObject factoryKeyObj = obj.optJSONObject(factoryKey);
                    if (factoryKeyObj == null) {
                        continue;
                    }
                    String factoryName = factoryKeyObj.getString(category);
                    ModuleFactoryInterface instance = (ModuleFactoryInterface) Class.forName(factoryName).newInstance();
                    extraModuleFactories.add(instance);
                    Log.i(TAG, "load extra module: " + factoryName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "getExtraModuleFactory: " + e.getMessage());
        }
        return extraModuleFactories;
    }

    /**
     * Retrieves a list of factory classes implementing @c AuthorizationHandlerFactoryInterface by
     * iterating through the json files that are located under the sample-app folder of assets
     * directory. The json contains the fully qualified class name of the factory class.
     * The expected json format:
     *
     * @code{.json}
     * {
     *
     *   "authorizationhandlerfactory": {
     *     "name": "fully-qualified-class-name-implementing-AuthorizationHandlerFactoryInterface"
     *   }
     *
     * }
     * @endcode
     */
    private List<AuthorizationHandlerFactoryInterface> getExtraAuthorizationHandlerFactory() {
        List<AuthorizationHandlerFactoryInterface> extraAuthorizationHandlerFactories = new ArrayList<>();
        try {
            String folderName = "sample-app";
            String factoryKey = "authorizationhandlerfactory";
            String category = "name";
            String[] fileList = getAssets().list(folderName);
            Log.i(TAG, "getExtraAuthorizationHandlerFactory: begin loading");
            for (String f : fileList) {
                InputStream is = getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONObject factoryKeyObj = obj.optJSONObject(factoryKey);
                    if (factoryKeyObj == null) {
                        continue;
                    }
                    String factoryName = factoryKeyObj.getString(category);
                    AuthorizationHandlerFactoryInterface instance =
                            (AuthorizationHandlerFactoryInterface) Class.forName(factoryName).newInstance();
                    extraAuthorizationHandlerFactories.add(instance);
                    Log.i(TAG, "getExtraAuthorizationHandlerFactory: load extra module:" + factoryName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "getExtraAuthorizationHandlerFactory: " + e.getMessage());
        }
        return extraAuthorizationHandlerFactories;
    }

    // Load UI of extra authorization handlers and set authorization handler
    private void loadAuthorizationHandlerUIAndRegister(
            List<AuthorizationHandlerFactoryInterface> extraAuthorizationHandlerFactories,
            SampleAppContext sampleAppContext) {
        JSONObject deviceConfig = new JSONObject();
        try {
            deviceConfig.put("productId", mPreferences.getString(getString(R.string.preference_product_id), ""));
            deviceConfig.put("productDsn", mPreferences.getString(getString(R.string.preference_product_dsn), ""));
        } catch (JSONException e) {
            Log.e(TAG, "loadAuthorizationHandlerUIAndRegister: Cloud not create a device json");
            return;
        }

        for (AuthorizationHandlerFactoryInterface handlerFactory : extraAuthorizationHandlerFactories) {
            // Inflate any drawer view UIs
            LinearLayout llDisplayData = findViewById(R.id.drawer_linear_layout);
            LayoutInflater linflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            for (int layoutResourceNumber : handlerFactory.getLayoutResourceNums()) {
                View tempView = linflater.inflate(layoutResourceNumber, null);
                llDisplayData.addView(tempView);
            }

            List<AuthorizationHandlerObserverInterface> handlers =
                    handlerFactory.getModuleAuthorizationHandlerInterfaces(this);
            for (AuthorizationHandlerObserverInterface handler : handlers) {
                handler.initialize(mAuthorizationHandler, deviceConfig);
            }
        }
    }

    //     Add configuration of extra modules
    private void configExtraModules(SampleAppContext sampleAppContext,
            List<ModuleFactoryInterface> extraModuleFactories, List<EngineConfiguration> configuration) {
        for (ModuleFactoryInterface moduleFactory : extraModuleFactories) {
            List<EngineConfiguration> moduleConfigs = moduleFactory.getConfiguration(sampleAppContext);
            for (EngineConfiguration moduleConfig : moduleConfigs) {
                Log.e(TAG, moduleConfig.toString());
                configuration.add(moduleConfig);
            }
        }
    }

    //     Register platform interfaces and load UI of extra modules
    private void loadPlatformInterfacesAndLoadUI(
            Engine engine, List<ModuleFactoryInterface> extraModuleFactories, SampleAppContext sampleAppContext) {
        for (ModuleFactoryInterface moduleFactory : extraModuleFactories) {
            //     Inflate any drawer view UIs. Must inflate UI before registering platform interfaces
            LinearLayout llDisplayData = findViewById(R.id.drawer_linear_layout);
            LayoutInflater linflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            for (int layoutResourceNumber : moduleFactory.getLayoutResourceNums()) {
                View tempView = linflater.inflate(layoutResourceNumber, null);
                llDisplayData.addView(tempView);
            }
            //     Add adapter fragments
            List<Fragment> fragments = moduleFactory.getFragments(sampleAppContext);
            for (Fragment fragment : fragments) {
                mAdapter.addFragment(fragment);
                mAdapter.notifyDataSetChanged();
            }
            //     Register platform interfaces
            List<PlatformInterface> platformInterfaces = moduleFactory.getModulePlatformInterfaces(sampleAppContext);
            for (PlatformInterface platformInterface : platformInterfaces) {
                if (!engine.registerPlatformInterface(platformInterface))
                    throw new RuntimeException("Could not register extra module interface");
            }
        }
    }

    private void setUpTimeZoneUI() {
        final String[] timezones = new String[timezoneArray.size()];
        timezoneAdapter =
                new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, timezoneArray.toArray(timezones));

        Spinner spinnerView = (Spinner) findViewById(R.id.timezone_spinner);
        spinnerView.setAdapter(timezoneAdapter);

        final String defaultTimeZone = mPropertyManager.getProperty(AlexaProperties.TIMEZONE);

        int timezonePosition = timezoneAdapter.getPosition(defaultTimeZone);
        spinnerView.setSelection(timezonePosition);
        spinnerView.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1, int position, long arg3) {
                String s = timezoneArray.get(position);
                if (!mPropertyManager.getProperty(AlexaProperties.TIMEZONE).equals(s)) {
                    Toast.makeText(MainActivity.this, "Switching timezone to " + s, Toast.LENGTH_SHORT).show();

                    mPropertyManager.setProperty(AlexaProperties.TIMEZONE, s);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub
            }
        });
    }
}
