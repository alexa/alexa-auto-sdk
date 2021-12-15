package com.amazon.alexa.auto.voiceinteraction.service;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.service.voice.VoiceInteractionService;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.UiThemeManager;
import com.amazon.alexa.auto.settings.config.AACSConfigurationPreferences;
import com.amazon.alexa.auto.settings.config.AACSConfigurator;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.alexa.auto.voice.ui.session.SessionActivityControllerImpl;
import com.amazon.alexa.auto.voice.ui.session.SessionViewControllerImpl;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONObject;

import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.reactivex.rxjava3.core.Single;

/**
 * Alexa Auto Voice Interaction Service, extends top-level service of the current global voice interactor,
 * which is providing support for start/stop AACS, handle Alexa wakeword and the back-end of a VoiceInteractor.
 */
public class AutoVoiceInteractionService extends VoiceInteractionService {
    private static final String TAG = AutoVoiceInteractionService.class.getCanonicalName();
    private static final int FLAG_SHOW_WITH_ASSIST = 1;

    AuthController mAuthController;
    AACSConfigurator mAACSConfigurator;
    UiThemeManager mUiThemeManager;
    AACSSender mAACSSender;
    AACSMessageSender mMessageSender;

    private ExecutorService mExecutorService;
    private Handler mMainThreadHandler;

    private boolean isAlexaConnected;

    private static String[] AMAZONLITE_MODEL_FILES;

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();
        EventBus.getDefault().register(this);

        mExecutorService = Executors.newSingleThreadExecutor();
        mMainThreadHandler = new Handler(Looper.getMainLooper());

        AlexaApp mApp = AlexaApp.from(this);
        mApp.getRootComponent().activateScope(new SessionViewControllerImpl());
        mApp.getRootComponent().activateScope(new SessionActivityControllerImpl());

        // Initializing extra modules
        getModuleAsync().filter(Optional::isPresent).map(Optional::get).subscribe((modules) -> {
            for (ModuleInterface moduleInterface : modules) {
                moduleInterface.initialize(this.getApplicationContext());
            }
        });

        mAuthController = mApp.getRootComponent().getAuthController();
        mAACSSender = new AACSSender();

        WeakReference<Context> ContextWk = new WeakReference<>(this.getApplicationContext());
        mAACSConfigurator =
                new AACSConfigurator(ContextWk, mAACSSender, new AACSConfigurationPreferences(ContextWk));
        mMessageSender = new AACSMessageSender(ContextWk, mAACSSender);
        mUiThemeManager = new UiThemeManager(getApplicationContext(), mMessageSender);
    }

    @Override
    public void onReady() {
        Log.i(TAG, "OnReady");
        super.onReady();

        // Temporary fix to start AACS 30 seconds after startup of VIS after recent
        // device boot. This allows system to settle down and deliver intents in
        // regular time window instead of rather large time window (up to 15 seconds
        // in delivery on emulator).
        final int aacs_start_delay_ms = 30 * 1000;
        final long device_boot_margin_ms = 5 * 1000 * 60;
        final long ms_since_device_bootup = SystemClock.elapsedRealtime();
        if (ms_since_device_bootup < device_boot_margin_ms) {
            Log.d(TAG, "Will start AACS after 30 seconds. Milliseconds since device bootup " + ms_since_device_bootup);
            new Handler().postDelayed(() -> {
                Log.d(TAG, "Starting AACS after delay. Milliseconds since device bootup " + ms_since_device_bootup);
                AACSServiceController.startAACS(this, true);
            }, aacs_start_delay_ms);
        } else {
            Log.d(TAG, "Starting AACS right away. Milliseconds since device bootup " + ms_since_device_bootup);
            AACSServiceController.startAACS(this, true);
        }

        // The order is important! Share files before configuring AACS.
        mAACSConfigurator.shareFilesWithAACS(this.getApplicationContext());
        // Configure AACS immediately without waiting for delay start to avoid consistent ANRs in AACS.
        mAACSConfigurator.configureAACSWithPreferenceOverrides();

        if (mAuthController.isAuthenticated()) {
            isAlexaConnected = true;
        }

        mUiThemeManager.init();
    }

    @Override
    public void onShutdown() {
        Log.i(TAG, "onShutdown");
        super.onShutdown();

        AACSServiceController.stopAACS(this);

        // Un-initializing extra modules
        getModuleAsync().filter(Optional::isPresent).map(Optional::get).subscribe((modules) -> {
            for (ModuleInterface moduleInterface : modules) {
                moduleInterface.uninitialize(this.getApplicationContext());
            }
        });

        mUiThemeManager.destroy();
    }

    @Subscribe
    public void onVoiceInteractionStateChange(AutoVoiceInteractionMessage message) {
        if (message.getTopic().equals(Constants.TOPIC_ALEXA_CONNECTION)) {
            isAlexaConnected = message.getAction().equals(Constants.CONNECTION_STATUS_CONNECTED);
        }

        if (message.getAction().equals(Action.SpeechRecognizer.WAKEWORD_DETECTED)
                || message.getAction().equals(Action.SpeechRecognizer.START_CAPTURE)) {
            final Bundle args = new Bundle();
            if (isAlexaConnected) {
                if (message.getAction().equals(Action.SpeechRecognizer.START_CAPTURE)) {
                    Log.d(TAG, "SpeechRecognizer: Start capture...");
                    args.putString(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.START_CAPTURE);
                } else if (message.getAction().equals(Action.SpeechRecognizer.WAKEWORD_DETECTED)) {
                    Log.d(TAG, "SpeechRecognizer: Wake word is detected...");
                    args.putString(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.WAKEWORD_DETECTED);
                    args.putString(Action.SpeechRecognizer.WAKEWORD_DETECTED, message.getPayload());
                }
                showSession(args, FLAG_SHOW_WITH_ASSIST);
            } else {
                Log.d(TAG, "Alexa is not connected!");
                args.putString(Constants.TOPIC_ALEXA_CONNECTION, Constants.ACTION_ALEXA_NOT_CONNECTED);
                showSession(args, FLAG_SHOW_WITH_ASSIST);
            }
        }
    }

    private Single<Optional<List<ModuleInterface>>> getModuleAsync() {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                Optional<List<ModuleInterface>> handlerModuleOptional = getModuleSync(this.getApplicationContext());
                mMainThreadHandler.post(() -> emitter.onSuccess(handlerModuleOptional));
            });
        });
    }

    private Optional<List<ModuleInterface>> getModuleSync(Context context) {
        List<ModuleInterface> modules = new ArrayList<>();
        try {
            String folderName = "aacs-sample-app";
            String moduleKey = "module";
            String category = "name";
            String[] fileList = context.getAssets().list(folderName);
            for (String f : fileList) {
                InputStream is = context.getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONObject moduleKeyObj = obj.optJSONObject(moduleKey);
                    if (moduleKeyObj == null) {
                        Log.w(TAG, "module key is missing");
                        continue;
                    }
                    String moduleName = moduleKeyObj.getString(category);
                    ModuleInterface instance = (ModuleInterface) Class.forName(moduleName).newInstance();
                    modules.add(instance);
                    Log.i(TAG, "getModule: load extra module:" + moduleName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "getModule: " + e.getMessage());
            return Optional.empty();
        }
        return Optional.of(modules);
    }
}
