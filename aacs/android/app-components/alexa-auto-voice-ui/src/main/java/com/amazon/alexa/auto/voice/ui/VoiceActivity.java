package com.amazon.alexa.auto.voice.ui;

import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.SpeechRecognizerMessages;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AnimationProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.EarconProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.session.SessionActivityController;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.NetworkUtil;
import com.amazon.alexa.auto.voice.ui.common.AutoVoiceUIMessage;
import com.amazon.alexa.auto.voice.ui.common.Constants;
import com.amazon.alexa.auto.voice.ui.earcon.EarconController;
import com.amazon.autovoicechrome.AutoVoiceChromeController;
import com.amazon.autovoicechrome.util.AutoVoiceChromeState;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;
import java.util.Optional;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import io.reactivex.rxjava3.disposables.Disposable;

import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.ModuleName.LVC;
import static com.amazon.alexa.auto.apps.common.util.NetworkUtil.TYPE_NOT_CONNECTED;

/**
 * Active Alexa Auto voice session activity, providing a facility for the implementation
 * to interact with the user in the voice interaction layer, such as showing voice chrome animation,
 * inflating voice fragment based on the capability.
 */
public class VoiceActivity extends AppCompatActivity {
    private static final String TAG = VoiceActivity.class.getCanonicalName();

    private Disposable mActivityControllerDisposable;
    private boolean mVoiceSessionInUse;
    private boolean mSessionEnded;
    private Disposable mViewControllerDisposable;
    private MediaPlayer mNetworkErrorTTSPrompt;
    private Fragment mVoiceFragment;

    @NonNull
    AutoVoiceChromeController mAutoVoiceChromeController;
    @NonNull
    EarconController mEarconController;
    @NonNull
    SpeechRecognizerMessages mSpeechRecognizerMessages;
    @NonNull
    private AACSSender mAACSSender;
    @NonNull
    private AACSMessageSender mMessageSender;

    @VisibleForTesting
    View mContentView;

    AnimationProvider mAnimationProvider;
    EarconProvider mEarconProvider;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setTheme(R.style.Theme_Alexa_VoiceInteractionUi_Activity);
        setContentView(R.layout.autovoiceinteraction_layout);

        mAutoVoiceChromeController = new AutoVoiceChromeController(getApplicationContext());
        mEarconController = new EarconController(getApplicationContext());
        mAACSSender = new AACSSender();
        mMessageSender = new AACSMessageSender(new WeakReference<>(getApplicationContext()), mAACSSender);
        mSpeechRecognizerMessages = new SpeechRecognizerMessages(mMessageSender);

        EventBus.getDefault().register(this);
        mEarconController.initEarcon();
        mNetworkErrorTTSPrompt = MediaPlayer.create(getApplicationContext(), R.raw.auto_error_offline);
        initializeProviders();

        // Getting voice chrome view group for auto voice chrome controller
        mContentView = findViewById(R.id.auto_voice_interaction_view_container);

        ViewGroup v = (ViewGroup) mContentView;

        // Getting voice chrome view group for auto voice chrome controller
        ViewGroup autoVoiceChromeBarView = (ViewGroup) v.getChildAt(1);

        mAutoVoiceChromeController.initialize(autoVoiceChromeBarView);

        // Initializing animation provider if it exists
        if (mAnimationProvider != null) {
            // Inflate custom animation layout onto the root view
            getLayoutInflater().inflate(mAnimationProvider.getCustomLayout(), v);
            ViewGroup autoCustomAnimationView = (ViewGroup) v.getChildAt(2);
            mAnimationProvider.initialize(getApplicationContext(), autoCustomAnimationView);
        }

        // TODO: we provide user a way to cancel Alexa voice request anytime needed, now it can be anywhere on the
        // screen, the UX could be replace with "X" button, need UX confirm
        mContentView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Cancel current Alexa request");
                AACSMessageBuilder.buildMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "")
                        .ifPresent(message -> {
                            mMessageSender.sendMessage(
                                    Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "");
                        });
                AlexaApp app = AlexaApp.from(getApplicationContext());
                Optional<SessionViewController> viewController =
                        app.getRootComponent().getComponent(SessionViewController.class);
                viewController.ifPresent(SessionViewController::clearTemplate);
                finish();
            }
        });

        Bundle args = getIntent().getExtras();

        if (args.containsKey(Topic.SPEECH_RECOGNIZER)) {
            String action = args.getString(Topic.SPEECH_RECOGNIZER);
            if (mEarconProvider != null) {
                if (Action.SpeechRecognizer.WAKEWORD_DETECTED.equals(action)) {
                    mEarconController.playAudioCueStartVoice(
                            mEarconProvider.shouldUseAudioCueStartVoice(args.getString(action)));
                }
            } else {
                if (Action.SpeechRecognizer.WAKEWORD_DETECTED.equals(action)) {
                    mEarconController.playAudioCueStartVoice();
                }
            }

            if (mAnimationProvider != null) {
                switchAnimation(AutoVoiceChromeState.LISTENING.toString());
            } else {
                mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.LISTENING);
            }
        } else if (args.containsKey(Constants.TOPIC_ALEXA_CONNECTION)) {
            if (Constants.ACTION_ALEXA_NOT_CONNECTED.equals(args.getString(Constants.TOPIC_ALEXA_CONNECTION))) {
                //Error chrome is only shown if authenticated
                if (AlexaApp.from(getApplicationContext()).getRootComponent().getAuthController().isAuthenticated()) {
                    mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
                    mNetworkErrorTTSPrompt.start();
                }

            }
        } else {
            // User presses PTT
            if (AlexaApp.from(getApplicationContext()).getRootComponent().getAuthController().isAuthenticated()) {
                int connectivityStatus = NetworkUtil.getConnectivityStatus(getApplicationContext());
                if ( connectivityStatus == NetworkUtil.TYPE_NOT_CONNECTED
                        && !ModuleProvider.containsModule(getApplicationContext(), LVC)) {
                    mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
                    mNetworkErrorTTSPrompt.start();
                } else {
                    //Notifying AACS to start capture
                    Log.d(TAG, "PTT: Start capture...");
                    mSpeechRecognizerMessages.sendStartCapture(AASBConstants.SpeechRecognizer.SPEECH_INITIATOR_TAP_TO_TALK);
                    mEarconController.playAudioCueStartTouch();

                    if (mAnimationProvider != null) {
                        // Prepare animation for PTT action
                        mAnimationProvider.prepareAnimationForPTT();
                        switchAnimation(AutoVoiceChromeState.LISTENING.toString());
                    } else {
                        mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.LISTENING);
                    }
                }
            } else {
                Log.d(TAG, "PTT: User not authenticated, ignoring ...");
            }
        }

        setVISViewForSessionController();
    }

    public void onStart() {
        Log.d(TAG, "onStart");
        super.onStart();

        if (!isFinishing()) {
            // Clear previous added fragment if voice session is not in used
            if (!mVoiceSessionInUse) {
                AlexaApp.from(getApplicationContext())
                        .getRootComponent()
                        .getComponent(SessionActivityController.class)
                        .ifPresent(SessionActivityController::removeFragment);
            }

            // Subscribe fragment added observable to add/remove fragment in voice activity
            AlexaApp app = AlexaApp.from(getApplicationContext());
            Optional<SessionActivityController> viewController =
                    app.getRootComponent().getComponent(SessionActivityController.class);
            viewController.ifPresent(sessionActivityController -> {
                mActivityControllerDisposable =
                        sessionActivityController.getFragmentAddedObservable().subscribe(fragmentAdded -> {
                            Log.d(TAG, "ActivityControllerDisposable | fragmentAdded | " + fragmentAdded);
                            mVoiceSessionInUse = fragmentAdded;

                            mVoiceFragment = sessionActivityController.getFragment();
                            if (mVoiceFragment != null) {
                                if (fragmentAdded) {
                                    getSupportFragmentManager().beginTransaction().add(R.id.auto_voice_interaction_view_container,
                                            mVoiceFragment, mVoiceFragment.getClass().getSimpleName()).commit();

                                } else {
                                    getSupportFragmentManager().beginTransaction().remove(mVoiceFragment).commit();
                                    mVoiceFragment = null;

                                    if (mSessionEnded) {
                                        Log.d(TAG,
                                                "FragmentAddedObservable | fragment removed and session is "
                                                        + "finished so finishing activity");
                                        finish();
                                    }
                                }
                            }
                        });
            });
        }
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();

        mVoiceSessionInUse = false;
        finish();
    }

    @Override
    public void finish() {
        Log.d(TAG, "finish");
        super.finish();

        // Clean up visuals and eventbus in finish() callback instead of onDestroy() to prevent
        // leaking IntentReceiver and crashing the app when session is finished.
        mAutoVoiceChromeController.onDestroy();
        if (mAnimationProvider != null) {
            mAnimationProvider.uninitialize();
        }
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        mEarconController.uninitEarcon();
        if (mNetworkErrorTTSPrompt != null) {
            mNetworkErrorTTSPrompt.release();
            mNetworkErrorTTSPrompt = null;
        }

        uninitializeProviders();

        AlexaApp.from(getApplicationContext())
                .getRootComponent()
                .getComponent(SessionViewController.class)
                .ifPresent(sessionViewController -> sessionViewController.setSessionView(null));
        if (mViewControllerDisposable != null) {
            mViewControllerDisposable.dispose();
            mViewControllerDisposable = null;
        }
        if (mActivityControllerDisposable != null) {
            mActivityControllerDisposable.dispose();
            mActivityControllerDisposable = null;
        }
    }

    @Subscribe
    public void onVoiceUiStateChange(AutoVoiceUIMessage message) {
        Log.d(TAG,
                "Receiving voice interaction message, topic: " + message.getTopic()
                        + " action: " + message.getAction());

        if (!isFinishing()) {
            if (message.getTopic().equals(Constants.TOPIC_VOICE_ANIMATION)) {
                switch (message.getAction()) {
                    case Action.AlexaClient.DIALOG_STATE_CHANGED:
                        if (mAnimationProvider != null) {
                            // Switching animation for dialog state changed
                            switchAnimation(message.getPayload());
                        } else {
                            mAutoVoiceChromeController.onStateChanged(
                                    convertToAutoVoiceChromeDialogState(message.getPayload()));
                        }

                        if (message.getPayload().equals(AutoVoiceChromeState.IDLE.toString())) {
                            mSessionEnded = true;
                            if (!mVoiceSessionInUse) {
                                Log.d(TAG, "voice session idle, so finishing voice session.");
                                finish();
                            }
                        } else if (message.getPayload().equals(AutoVoiceChromeState.LISTENING.toString())) {
                            mSessionEnded = false;
                        }
                        break;
                    case Action.Animation.ANIMATION_SWITCH:
                        if (mAnimationProvider != null) {
                            switchAnimation(AutoVoiceChromeState.SPEAKING.toString());
                        }
                }
            } else if (message.getAction().equals(Action.SpeechRecognizer.END_OF_SPEECH_DETECTED)) {
                if (mEarconProvider != null) {
                    mEarconController.playAudioCueEnd(mEarconProvider.shouldUseAudioCueEnd(""));
                } else {
                    mEarconController.playAudioCueEnd();
                }
            }
        }
    }

    private void initializeProviders() {
        if (getApplicationContext() != null) {
            AlexaApp app = AlexaApp.from(getApplicationContext());
            AlexaAppRootComponent alexaAppRootComponent = app.getRootComponent();
            if (alexaAppRootComponent != null
                    && alexaAppRootComponent.getComponent(AnimationProvider.class).isPresent()) {
                mAnimationProvider = alexaAppRootComponent.getComponent(AnimationProvider.class).get();
            }
            if (alexaAppRootComponent != null && alexaAppRootComponent.getComponent(EarconProvider.class).isPresent()) {
                mEarconProvider = alexaAppRootComponent.getComponent(EarconProvider.class).get();
            }
        }
    }

    private void uninitializeProviders() {
        if (mAnimationProvider != null) {
            mAnimationProvider = null;
        }

        if (mEarconProvider != null) {
            mEarconProvider = null;
        }
    }

    private void switchAnimation(String dialogState) {
        if (mAnimationProvider.shouldTakeOver()) {
            // Clear alexa chrome before switching to custom animation provider
            mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IDLE);
            mAnimationProvider.doTakeOver(dialogState);
        } else {
            // Clear custom animation provider before switching to alexa chrome
            mAnimationProvider.doTakeOver(AutoVoiceChromeState.IDLE.toString());
            mAutoVoiceChromeController.onStateChanged(convertToAutoVoiceChromeDialogState(dialogState));
        }
    }

    /**
     * Set VIS view for session controller. This view will be used by other components to inflate
     * visual elements onto.
     */
    private void setVISViewForSessionController() {
        ViewGroup view = findViewById(R.id.auto_voice_interaction_view);

        AlexaApp app = AlexaApp.from(getApplicationContext());
        Optional<SessionViewController> viewController =
                app.getRootComponent().getComponent(SessionViewController.class);

        viewController.ifPresent(sessionViewController -> {
            sessionViewController.setSessionView(view);
            mViewControllerDisposable =
                    sessionViewController.getTemplateDisplayedObservable().subscribe(templateDisplayed -> {
                        Log.d(TAG, "TemplateDisplayedObservable | templateDisplayed | " + templateDisplayed);
                        mVoiceSessionInUse = templateDisplayed;

                        View layout = findViewById(R.id.auto_voice_chrome_bar_view);
                        if (templateDisplayed) {
                            layout.setBackgroundColor(getApplicationContext().getResources().getColor(R.color.transparent_black));
                        } else {
                            layout.setBackgroundColor(Color.TRANSPARENT);
                        }

                        if (!templateDisplayed && mSessionEnded) {
                            Log.d(TAG,
                                    "TemplateDisplayedObservable | template cleared and session is "
                                            + "finished so finishing activity");
                            finish();
                        }
                    });
        });
    }

    /**
     * Convert AlexaClient Dialog state to AutoVoiceChrome dialog state
     *
     * @param state DialogState
     * @return AutoVoiceChromeState
     */
    @VisibleForTesting
    AutoVoiceChromeState convertToAutoVoiceChromeDialogState(String state) {
        AutoVoiceChromeState autoVoiceChromeState = AutoVoiceChromeState.UNKNOWN;
        switch (state) {
            case "IDLE":
            case "EXPECTING":
                autoVoiceChromeState = AutoVoiceChromeState.IDLE;
                break;
            case "SPEAKING":
                autoVoiceChromeState = AutoVoiceChromeState.SPEAKING;
                break;
            case "THINKING":
                autoVoiceChromeState = AutoVoiceChromeState.THINKING;
                break;
            case "LISTENING":
                autoVoiceChromeState = AutoVoiceChromeState.LISTENING;
                break;
        }
        return autoVoiceChromeState;
    }
}