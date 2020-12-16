package com.amazon.alexa.auto.voiceinteraction.service;

import android.app.assist.AssistContent;
import android.app.assist.AssistStructure;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.service.voice.VoiceInteractionSession;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.SpeechRecognizerMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.alexa.auto.voiceinteraction.earcon.EarconController;
import com.amazon.autovoicechrome.AutoVoiceChromeController;
import com.amazon.autovoicechrome.util.AutoVoiceChromeState;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.jetbrains.annotations.NotNull;

import java.lang.ref.WeakReference;
import java.util.Objects;
import java.util.Optional;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * Active Alexa Auto voice interaction session, providing a facility for the implementation
 * to interact with the user in the voice interaction layer.
 */
public class AutoVoiceInteractionSession extends VoiceInteractionSession {
    private static final String TAG = AutoVoiceInteractionSession.class.getSimpleName();

    @NonNull
    private final WeakReference<Context> mApplicationContext;
    @NonNull
    private final AACSSender mAACSSender;
    @NonNull
    private final AACSMessageSender mMessageSender;
    @VisibleForTesting
    @NonNull
    AutoVoiceChromeController mAutoVoiceChromeController;
    @VisibleForTesting
    @NonNull
    SpeechRecognizerMessages mSpeechRecognizerMessages;
    @NonNull
    EarconController mEarconController;

    @VisibleForTesting
    View mContentView;
    private boolean mVoiceSessionInUse;
    private boolean mSessionEnded;
    private Disposable mViewControllerDisposable;

    public AutoVoiceInteractionSession(@NotNull Context context) {
        super(context);
        mApplicationContext = new WeakReference<>(context);
        mAACSSender = new AACSSender();
        mMessageSender = new AACSMessageSender(mApplicationContext, mAACSSender);
        mAutoVoiceChromeController = new AutoVoiceChromeController(context);
        mSpeechRecognizerMessages = new SpeechRecognizerMessages(mMessageSender);
        mEarconController = new EarconController(context);
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        setTheme(R.style.Theme_Alexa_VoiceInteractionUi_Activity);
        super.onCreate();
        EventBus.getDefault().register(this);
        mEarconController.initEarcon();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        EventBus.getDefault().unregister(this);
        mEarconController.uninitEarcon();
        mAutoVoiceChromeController.onDestroy();
        AlexaApp.from(getContext())
                .getRootComponent()
                .getScopedComponents()
                .getComponent(SessionViewController.class)
                .ifPresent(sessionViewController -> sessionViewController.setSessionView(null));
        if (mViewControllerDisposable != null) {
            mViewControllerDisposable.dispose();
            mViewControllerDisposable = null;
        }
    }

    @Override
    public void onShow(Bundle args, int showFlags) {
        Log.d(TAG, "onShow");
        super.onShow(args, showFlags);

        if (args.containsKey(Topic.SPEECH_RECOGNIZER)) {
            if (Objects.equals(args.getString(Topic.SPEECH_RECOGNIZER), Action.SpeechRecognizer.START_CAPTURE)) {
                mEarconController.playAudioCueStartTouch();
            } else if (Objects.equals(
                               args.getString(Topic.SPEECH_RECOGNIZER), Action.SpeechRecognizer.WAKEWORD_DETECTED)) {
                mEarconController.playAudioCueStartVoice();
            }
            mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.LISTENING);
        } else if (args.containsKey(Constants.TOPIC_ALEXA_CONNECTION)) {
            if (Objects.equals(
                        args.getString(Constants.TOPIC_ALEXA_CONNECTION), Constants.ACTION_ALEXA_NOT_CONNECTED)) {
                mAutoVoiceChromeController.onStateChanged(AutoVoiceChromeState.IN_ERROR);
            }
        }
    }

    @Override
    public View onCreateContentView() {
        Log.d(TAG, "onCreateContentView");
        mContentView = getLayoutInflater().inflate(R.layout.autovoiceinteraction_layout, null);
        setContentView(mContentView);

        ViewGroup v = (ViewGroup) mContentView;

        // Getting voice chrome view group for auto voice chrome controller
        ViewGroup child = (ViewGroup) v.getChildAt(1);
        mAutoVoiceChromeController.initialize(child);

        // TODO: we provide user a way to cancel Alexa voice request anytime needed, now it can be anywhere on the
        // screen,
        // the UX could be replace with "X" button, need UX confirm
        mContentView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Cancel current Alexa request");
                AACSMessageBuilder.buildMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "")
                        .ifPresent(message -> {
                            mMessageSender.sendMessage(
                                    Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "");
                        });
                finish();
            }
        });

        setVISViewForSessionController();

        return mContentView;
    }

    /**
     * Set VIS view for session controller. This view will be used by other components to inflate
     * visual elements onto.
     */
    private void setVISViewForSessionController() {
        ViewGroup view = mContentView.findViewById(R.id.auto_voice_interaction_view);

        AlexaApp app = AlexaApp.from(getContext());
        Optional<SessionViewController> viewController =
                app.getRootComponent().getScopedComponents().getComponent(SessionViewController.class);

        viewController.ifPresent(sessionViewController -> {
            sessionViewController.setSessionView(view);
            mViewControllerDisposable =
                    sessionViewController.getTemplateDisplayedObservable().subscribe(templateDisplayed -> {
                        Log.d(TAG, "TemplateDisplayedObservable | templateDisplayed | " + templateDisplayed);
                        mVoiceSessionInUse = templateDisplayed;

                        View layout = mContentView.findViewById(R.id.auto_voice_chrome_bar_view);
                        if (templateDisplayed) {
                            layout.setBackgroundColor(getContext().getResources().getColor(R.color.transparent_black));
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

    @Override
    public void onHandleAssist(
            @Nullable Bundle data, @Nullable AssistStructure structure, @Nullable AssistContent content) {
        Log.d(TAG, "onHandleAssist");
        super.onHandleAssist(data, structure, content);

        mSpeechRecognizerMessages.sendStartCapture(AASBConstants.SpeechRecognizer.SPEECH_INITIATOR_TAP_TO_TALK);
    }

    @Subscribe
    public void onVoiceInteractionStateChange(AutoVoiceInteractionMessage message) {
        Log.d(TAG,
                "Receiving voice interaction message, topic: " + message.getTopic()
                        + " action: " + message.getAction());
        if (message.getTopic().equals(Constants.TOPIC_VOICE_CHROME)) {
            mAutoVoiceChromeController.onStateChanged(convertToAutoVoiceChromeDialogState(message.getAction()));

            if (message.getAction().equals(AutoVoiceChromeState.IDLE.toString())) {
                mSessionEnded = true;
                if (!mVoiceSessionInUse) {
                    Log.d(TAG, "voice session idle, so finishing voice session.");
                    finish();
                }
            } else if (message.getAction().equals(AutoVoiceChromeState.LISTENING.toString())) {
                mSessionEnded = false;
            }
        } else if (message.getAction().equals(Action.SpeechRecognizer.END_OF_SPEECH_DETECTED)) {
            mEarconController.playAudioCueEnd();
        }
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
