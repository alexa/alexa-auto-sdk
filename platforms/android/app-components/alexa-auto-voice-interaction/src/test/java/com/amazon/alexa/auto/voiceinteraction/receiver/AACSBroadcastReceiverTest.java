package com.amazon.alexa.auto.voiceinteraction.receiver;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.voiceinteraction.TestResourceFileReader;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.autovoicechrome.util.AutoVoiceChromeState;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class AACSBroadcastReceiverTest {
    @Mock
    private Context context;

    private AACSBroadcastReceiver aacsBroadcastReceiver;
    private EventBus eventBus;
    private Intent mIntent;

    private String receiveMessageTopic;
    private String receiveMessageAction;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);
        aacsBroadcastReceiver = spy(new AACSBroadcastReceiver());
        EventBus.getDefault().register(this);
        eventBus = spy(EventBus.getDefault());
    }

    @Test
    public void handle_aacs_dialog_state_listening_intent() {
        generateIntent("aacs/DialogStateChangedListening.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_VOICE_CHROME, AutoVoiceChromeState.LISTENING.toString(), "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_VOICE_CHROME);
        Assert.assertEquals(receiveMessageAction, AutoVoiceChromeState.LISTENING.toString());
    }

    @Test
    public void handle_aacs_dialog_state_thinking_intent() {
        generateIntent("aacs/DialogStateChangedThinking.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_VOICE_CHROME, AutoVoiceChromeState.THINKING.toString(), "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_VOICE_CHROME);
        Assert.assertEquals(receiveMessageAction, AutoVoiceChromeState.THINKING.toString());
    }

    @Test
    public void handle_aacs_dialog_state_speaking_intent() {
        generateIntent("aacs/DialogStateChangedSpeaking.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_VOICE_CHROME, AutoVoiceChromeState.SPEAKING.toString(), "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_VOICE_CHROME);
        Assert.assertEquals(receiveMessageAction, AutoVoiceChromeState.SPEAKING.toString());
    }

    @Test
    public void handle_aacs_dialog_state_idle_intent() {
        generateIntent("aacs/DialogStateChangedIdle.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_VOICE_CHROME, AutoVoiceChromeState.IDLE.toString(), "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_VOICE_CHROME);
        Assert.assertEquals(receiveMessageAction, AutoVoiceChromeState.IDLE.toString());
    }

    @Test
    public void handle_aacs_connection_status_connected_intent() {
        generateIntent("aacs/ConnectionStatusChangedConnected.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_ALEXA_CONNECTION, Constants.CONNECTION_STATUS_CONNECTED, "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_ALEXA_CONNECTION);
        Assert.assertEquals(receiveMessageAction, Constants.CONNECTION_STATUS_CONNECTED);
    }

    @Test
    public void handle_aacs_connection_status_disconnected_intent() {
        generateIntent("aacs/ConnectionStatusChangedDisconnected.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(
                        Constants.TOPIC_ALEXA_CONNECTION, Constants.CONNECTION_STATUS_DISCONNECTED, "");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_ALEXA_CONNECTION);
        Assert.assertEquals(receiveMessageAction, Constants.CONNECTION_STATUS_DISCONNECTED);
    }

    @Test
    public void handle_aacs_wakeword_detected_intent() {
        generateIntent("aacs/WakewordDetected.json");
        aacsBroadcastReceiver.onReceive(context, mIntent);
        Assert.assertEquals(receiveMessageTopic, Topic.SPEECH_RECOGNIZER);
        Assert.assertEquals(receiveMessageAction, Action.SpeechRecognizer.WAKEWORD_DETECTED);
    }

    @Test
    public void ignore_invalid_intent_action() {
        mIntent = new Intent("");
        verify(aacsBroadcastReceiver, times(0)).handleDialogStateChanged(any());
        verify(aacsBroadcastReceiver, times(0)).handleConnectionStatusChanged(any());
        verify(aacsBroadcastReceiver, times(0)).sendAutoVoiceInteractionMessage(any(), any(), any());
    }

    private void generateIntent(String resPath) {
        mIntent = new Intent(Constants.DIALOG_STATE_CHANGE);
        Optional<String> sampleAACSListeningMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSListeningMessage.get());
        mIntent.putExtra("payload", sampleListeningPayload);
    }

    @Subscribe
    public void testOnReceiveEvent(AutoVoiceInteractionMessage message) {
        receiveMessageTopic = message.getTopic();
        receiveMessageAction = message.getAction();
    }
}
