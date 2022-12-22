/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.voice.ui;

import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.ModuleName.LVC;

import android.content.Intent;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

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

import io.reactivex.rxjava3.disposables.Disposable;

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
    private String mCurrentAlexaDialogState;
    private Disposable mViewControllerDisposable;
    private MediaPlayer mNetworkErrorTTSPrompt;
    private Fragment mVoiceFragment;
    private static final Integer TRANSITION_TO_IDLE_WAIT_TIME = 500;
    private Runnable mTransitionToIdleRunnable;
    private Handler mHandler;

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
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.autovoiceinteraction_layout);

        mAutoVoiceChromeController = new AutoVoiceChromeController(getApplicationContext());
        mEarconController = new EarconController(getApplicationContext());
        mAACSSender = new AACSSender();
        mMessageSender = new AACSMessageSender(new WeakReference<>(getApplicationContext()), mAACSSender);
        mSpeechRecognizerMessages = new SpeechRecognizerMessages(mMessageSender);

        EventBus.getDefault().register(this);
        mEarconController.initEarcon();
        initializeProviders();

        // Getting voice chrome view group for auto voice chrome controller
        mContentView = findViewById(R.id.auto_voice_interaction_view_container);

        ViewGroup v = (ViewGroup) mContentView;

        // Getting voice chrome view group for auto voice chrome controller
        ViewGroup autoVoiceChromeBarView = v.findViewById(R.id.auto_voice_chrome_bar_view);
        mAutoVoiceChromeController.initialize(autoVoiceChromeBarView);

        // Initializing animation provider if it exists
        if (mAnimationProvider != null) {
            // Inflate custom animation layout onto the root view
            ViewGroup autoCustomAnimationView = (ViewGroup) getLayoutInflater()
                    .inflate(mAnimationProvider.getCustomLayout(), null);
            v.addView(autoCustomAnimationView);
            mAnimationProvider.initialize(getApplicationContext(), autoCustomAnimationView);
        }

        // TODO: we provide user a way to cancel Alexa voice request anytime needed, now it can be anywhere on the
        // screen, the UX could be replace with "X" button, need UX confirm
        mContentView.setOnClickListener(view -> cancelAlexaDialog());

        mTransitionToIdleRunnable = new Runnable() {
            @Override
            public void run() {
                if (!mVoiceSessionInUse) {
                    finish();
                }
            }
        };
        mHandler = new Handler(Looper.getMainLooper());

        mNetworkErrorTTSPrompt = MediaPlayer.create(getApplicationContext(), R.raw.auto_error_offline);
        mNetworkErrorTTSPrompt.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mPlayer) {
                mHandler.postDelayed(mTransitionToIdleRunnable, TRANSITION_TO_IDLE_WAIT_TIME);
            }
        });

        Bundle args = getIntent().getExtras();
        mCurrentAlexaDialogState = AASBConstants.AlexaClient.DIALOG_STATE_IDLE;
        if (args != null && args.getString(AASBConstants.ACTION) != null && args.containsKey(AASBConstants.TOPIC))
            sendAutoVoiceUIMessage(args);

        setVISViewForSessionController();
    }

    public void onStart() {
        Log.d(TAG, "onStart");
        super.onStart();
        if (!isFinishing()) {
            AlexaApp app = AlexaApp.from(getApplicationContext());
            Optional<SessionActivityController> viewController =
                    app.getRootComponent().getComponent(SessionActivityController.class);

            viewController.ifPresent(sessionActivityController -> {
                // Clear previously consumed fragment. Non-consumed fragments were added while the session was closed
                // and launched a new session to be shown
                if (sessionActivityController.isFragmentConsumed()) {
                    Log.i(TAG, "Removing the previously consumed fragment, if one exists");
                    sessionActivityController.removeFragment();
                }

                // Subscribe fragment added observable to add/remove fragment in voice activity
                mActivityControllerDisposable =
                        sessionActivityController.getFragmentAddedObservable().subscribe(fragmentAdded -> {
                            Log.d(TAG, "ActivityControllerDisposable | fragmentAdded | " + fragmentAdded);
                            mVoiceSessionInUse = fragmentAdded;

                            mVoiceFragment = sessionActivityController.getFragment();
                            if (mVoiceFragment != null) {
                                sessionActivityController.consumeFragment();
                                if (fragmentAdded) {
                                    getSupportFragmentManager()
                                            .beginTransaction()
                                            .add(R.id.auto_voice_interaction_view_container, mVoiceFragment,
                                                    mVoiceFragment.getClass().getSimpleName())
                                            .commit();

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
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(TAG, "onNewIntent");
        Bundle args = intent.getExtras();
        if (args != null && args.containsKey(AASBConstants.TOPIC))
            sendAutoVoiceUIMessage(args);
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();

        mVoiceSessionInUse = false;
        cancelAlexaDialog();
    }

    @Override
    public void finish() {
        Log.d(TAG, "finish");
        super.finish();
        overridePendingTransition(0,0);

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
        mHandler.removeCallbacksAndMessages(null);
    }

    @Subscribe
    public void onVoiceUiStateChange(AutoVoiceUIMessage message) {
        Log.d(TAG,
                "Receiving voice interaction message, topic: " + message.getTopic()
                        + " | action: " + message.getAction() + " | payload: " + message.getPayload());
        final String messageTopic = message.getTopic();
        final String messageAction = message.getAction();
        final String messagePayload = message.getPayload();
        if (!isFinishing()) {
            removeHandlerCallbacks(mTransitionToIdleRunnable);
            if(messageAction.equals(Action.AlexaClient.CLOSE_DISPLAY_CARD)) {
                finish();
            }
            if (Topic.SPEECH_RECOGNIZER.equals(messageTopic)) {
                switch (messageAction) {
                    case Action.SpeechRecognizer.WAKEWORD_DETECTED:
                        if (!AutoVoiceChromeState.LISTENING.equals(mCurrentAlexaDialogState))
                            wakewordDetected(messagePayload);
                        break;
                    case Action.SpeechRecognizer.START_CAPTURE:
                        if (AutoVoiceChromeState.LISTENING.equals(mCurrentAlexaDialogState))
                            cancelAlexaDialog();
                        else
                            pttPressed();
                        break;
                    case Action.SpeechRecognizer.END_OF_SPEECH_DETECTED:
                        if (mEarconProvider != null) {
                            mEarconController.playAudioCueEnd(mEarconProvider.shouldUseAudioCueEnd(""));
                        } else {
                            mEarconController.playAudioCueEnd();
                        }
                        break;
                }
            } else if (Constants.TOPIC_ALEXA_CONNECTION.equals(messageTopic)) {
                if (Constants.ACTION_ALEXA_NOT_CONNECTED.equals(messageAction) && !ModuleProvider.isAlexaCustomAssistantEnabled(getApplicationContext())) {
                    // Error chrome is only shown if authenticated
                    if (AlexaApp.from(getApplicationContext())
                                    .getRootComponent()
                                    .getAuthController()
                                    .isAuthenticated()) {
                        mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
                        mNetworkErrorTTSPrompt.start();
                    }
                }
            } else if (Constants.TOPIC_VOICE_ANIMATION.equals(messageTopic)) {
                switch (messageAction) {
                    case Action.AlexaClient.DIALOG_STATE_CHANGED:
                        mCurrentAlexaDialogState = messagePayload;
                        if (AASBConstants.AlexaClient.DIALOG_STATE_IDLE.equals(mCurrentAlexaDialogState)) {
                            mSessionEnded = true;
                            //Schedule  finishing the activity after the idle transition animation completes.
                            if (!mVoiceSessionInUse)
                                mHandler.postDelayed(mTransitionToIdleRunnable, TRANSITION_TO_IDLE_WAIT_TIME);
                        }
                        if (AASBConstants.AlexaClient.DIALOG_STATE_LISTENING.equals(mCurrentAlexaDialogState)) {
                            mSessionEnded = false;
                        }
                        if (AASBConstants.AlexaClient.DIALOG_STATE_EXPECTING.equals(
                                           message.getPayload())) {
                            if (mEarconProvider != null) {
                                mEarconController.playAudioCueStartVoice(
                                        mEarconProvider.shouldUseAudioCueStartVoice(""));
                            } else {
                                mEarconController.playAudioCueStartVoice();
                            }
                            Log.d(TAG,
                                    "Ignoring EXPECTING state for animation. Will transition to LISTENING automatically");
                            return;
                        }
                        if (mAnimationProvider != null) {
                            // Switching animation for dialog state changed
                            switchAnimation(convertToAutoVoiceChromeDialogState(mCurrentAlexaDialogState).toString());
                        } else {
                            mAutoVoiceChromeController.onStateChanged(
                                    convertToAutoVoiceChromeDialogState(mCurrentAlexaDialogState));
                        }
                        break;
                    case Action.Animation.ANIMATION_SWITCH:
                        if (mAnimationProvider != null) {
                            switchAnimation(AutoVoiceChromeState.SPEAKING.toString());
                        }
                }
            }
        }
    }

    private void removeHandlerCallbacks(Runnable runnable) {
        if (runnable != null)
            mHandler.removeCallbacks(runnable);
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
        Log.d(TAG, "setVISViewForSessionController");
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
                            layout.getRootView().setBackgroundColor(
                                    getApplicationContext().getResources().getColor(R.color.transparent_black));
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
            case AASBConstants.AlexaClient.DIALOG_STATE_IDLE:
                autoVoiceChromeState = AutoVoiceChromeState.IDLE;
                break;
            case AASBConstants.AlexaClient.DIALOG_STATE_SPEAKING:
                autoVoiceChromeState = AutoVoiceChromeState.SPEAKING;
                break;
            case AASBConstants.AlexaClient.DIALOG_STATE_THINKING:
                autoVoiceChromeState = AutoVoiceChromeState.THINKING;
                break;
            case AASBConstants.AlexaClient.DIALOG_STATE_LISTENING:
                autoVoiceChromeState = AutoVoiceChromeState.LISTENING;
                break;
        }
        Log.d(TAG, "Returning: " + autoVoiceChromeState.toString());
        return autoVoiceChromeState;
    }

    private void cancelAlexaDialog() {
        Log.d(TAG, "Cancel current Alexa request");
        if (!mSessionEnded) {
            AACSMessageBuilder.buildMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "")
                    .ifPresent(message -> {
                        mMessageSender.sendMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "");
                    });
            mSessionEnded = true;
        }

        AlexaApp app = AlexaApp.from(getApplicationContext());
        Optional<SessionViewController> viewController =
                app.getRootComponent().getComponent(SessionViewController.class);
        viewController.ifPresent(SessionViewController::clearTemplate);
        Log.d(TAG, "Finishing Voice Activity...");
        finish();
    }

    private void wakewordDetected(String action) {
        if (isFinishing())
            return;
        if (AlexaApp.from(getApplicationContext()).getRootComponent().getAuthController().isAuthenticated()) {
            int connectivityStatus = NetworkUtil.getConnectivityStatus(getApplicationContext());
            if (connectivityStatus == NetworkUtil.TYPE_NOT_CONNECTED
                    && !ModuleProvider.containsModule(getApplicationContext(), LVC)) {
                mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
                mNetworkErrorTTSPrompt.start();
            } else {
                if (mEarconProvider != null)
                    mEarconController.playAudioCueStartVoice(mEarconProvider.shouldUseAudioCueStartVoice(action));
                else
                    mEarconController.playAudioCueStartVoice();
                if (mAnimationProvider != null)
                    switchAnimation(AutoVoiceChromeState.LISTENING.toString());
                else
                    mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.LISTENING);
            }
        } else {
            Log.d(TAG, "WW: User has not complete setup. Ignoring...");
        }
    }

    private void sendAutoVoiceUIMessage(Bundle args) {
        final String msgTopic = args.getString(AASBConstants.TOPIC, null);
        final String msgAction = args.getString(AASBConstants.ACTION, null);
        final String msgPayload = args.getString(AASBConstants.PAYLOAD, null);

        onVoiceUiStateChange(new AutoVoiceUIMessage(msgTopic, msgAction, msgPayload));
    }

    private void pttPressed() {
        if (isFinishing())
            return;
        if (AlexaApp.from(getApplicationContext()).getRootComponent().getAuthController().isAuthenticated()) {
            int connectivityStatus = NetworkUtil.getConnectivityStatus(getApplicationContext());
            if (connectivityStatus == NetworkUtil.TYPE_NOT_CONNECTED
                    && !ModuleProvider.containsModule(getApplicationContext(), LVC)) {
                mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
                mNetworkErrorTTSPrompt.start();
            } else {
                // Notifying AACS to start capture
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
            Log.d(TAG, "PTT User has not complete setup. Ignoring...");
        }
    }
}