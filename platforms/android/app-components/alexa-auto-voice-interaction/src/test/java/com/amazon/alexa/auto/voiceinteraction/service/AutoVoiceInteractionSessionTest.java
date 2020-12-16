package com.amazon.alexa.auto.voiceinteraction.service;

import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.app.assist.AssistContent;
import android.app.assist.AssistStructure;
import android.content.Context;
import android.os.Bundle;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.SpeechRecognizerMessages;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.alexa.auto.voiceinteraction.earcon.EarconController;
import com.amazon.autovoicechrome.AutoVoiceChromeController;
import com.amazon.autovoicechrome.util.AutoVoiceChromeState;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class AutoVoiceInteractionSessionTest {
    @Mock
    Context context;
    private AutoVoiceInteractionSession voiceInteractionSession;
    private AutoVoiceChromeController mockAutoVoiceChromeController;
    private SpeechRecognizerMessages mockSpeechRecognizerMessages;
    private EarconController mockEarconController;
    private Bundle sessionArgs;

    @Before
    public void setup() {
        voiceInteractionSession = new AutoVoiceInteractionSession(context);
        voiceInteractionSession.onCreate();
        mockAutoVoiceChromeController = Mockito.mock(AutoVoiceChromeController.class);
        mockSpeechRecognizerMessages = Mockito.mock(SpeechRecognizerMessages.class);
        mockEarconController = Mockito.mock(EarconController.class);
        voiceInteractionSession.mAutoVoiceChromeController = mockAutoVoiceChromeController;
        voiceInteractionSession.mSpeechRecognizerMessages = mockSpeechRecognizerMessages;
        voiceInteractionSession.mEarconController = mockEarconController;
    }

    @Test
    public void test_handle_voice_assist() {
        Bundle testBundle = new Bundle();
        AssistStructure mockAssistStructure = Mockito.mock(AssistStructure.class);
        AssistContent mockAssistContent = Mockito.mock(AssistContent.class);
        voiceInteractionSession.onHandleAssist(testBundle, mockAssistStructure, mockAssistContent);
        verify(voiceInteractionSession.mSpeechRecognizerMessages, times(1))
                .sendStartCapture(AASBConstants.SpeechRecognizer.SPEECH_INITIATOR_TAP_TO_TALK);
    }

    @Test
    public void test_show_listening_voice_chrome_over_speech_recognizer() {
        sessionArgs = createSessionArgs(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.START_CAPTURE);
        voiceInteractionSession.onShow(sessionArgs, 1);
        verify(voiceInteractionSession.mEarconController, times(1)).playAudioCueStartTouch();
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1))
                .onStateChanged(AutoVoiceChromeState.LISTENING);
    }

    @Test
    public void test_show_error_voice_chrome() {
        sessionArgs = createSessionArgs(Constants.TOPIC_ALEXA_CONNECTION, Constants.ACTION_ALEXA_NOT_CONNECTED);
        voiceInteractionSession.onShow(sessionArgs, 1);
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1))
                .onStateChanged(AutoVoiceChromeState.IN_ERROR);
    }

    @Test
    public void test_show_listening_voice_chrome() {
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(Constants.TOPIC_VOICE_CHROME, "LISTENING", ""));
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1))
                .onStateChanged(AutoVoiceChromeState.LISTENING);
    }

    @Test
    public void test_show_thinking_voice_chrome() {
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(Constants.TOPIC_VOICE_CHROME, "THINKING", ""));
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1))
                .onStateChanged(AutoVoiceChromeState.THINKING);
    }

    @Test
    public void test_show_speaking_voice_chrome() {
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(Constants.TOPIC_VOICE_CHROME, "SPEAKING", ""));
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1))
                .onStateChanged(AutoVoiceChromeState.SPEAKING);
    }

    @Test
    public void test_show_idle_voice_chrome() {
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(Constants.TOPIC_VOICE_CHROME, "IDLE", ""));
        verify(voiceInteractionSession.mAutoVoiceChromeController, times(1)).onStateChanged(AutoVoiceChromeState.IDLE);
    }

    @Test
    public void test_wake_word_detected_audio_cue() {
        sessionArgs = createSessionArgs(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.WAKEWORD_DETECTED);
        voiceInteractionSession.onShow(sessionArgs, 1);
        verify(voiceInteractionSession.mEarconController, times(1)).playAudioCueStartVoice();
    }

    @Test
    public void test_end_of_speech_detected_audio_cue() {
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(
                Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.END_OF_SPEECH_DETECTED, ""));
        verify(voiceInteractionSession.mEarconController, times(1)).playAudioCueEnd();
    }

    private Bundle createSessionArgs(String topic, String action) {
        Bundle args = new Bundle();
        args.putString(topic, action);
        return args;
    }
}
