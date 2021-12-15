package com.amazon.alexa.auto.voiceinteraction.receiver;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.voiceinteraction.TestResourceFileReader;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;

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
    private String receiveMessagePayload;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);
        aacsBroadcastReceiver = spy(new AACSBroadcastReceiver());
        EventBus.getDefault().register(this);
        eventBus = spy(EventBus.getDefault());
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
        verify(aacsBroadcastReceiver, times(1))
                .sendAutoVoiceInteractionMessage(Constants.TOPIC_VOICE_ANIMATION,
                        Action.SpeechRecognizer.WAKEWORD_DETECTED, "wakeword-SampleText");
        Assert.assertEquals(receiveMessageTopic, Constants.TOPIC_VOICE_ANIMATION);
        Assert.assertEquals(receiveMessageAction, Action.SpeechRecognizer.WAKEWORD_DETECTED);
        Assert.assertEquals(receiveMessagePayload, "wakeword-SampleText");
    }

    @Test
    public void ignore_invalid_intent_action() {
        mIntent = new Intent("");
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
        receiveMessagePayload = message.getPayload();
    }
}
