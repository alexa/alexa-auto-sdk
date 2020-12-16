package com.amazon.alexa.auto.voiceinteraction.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.ConnectionStatusChangedMessages;
import com.amazon.alexa.auto.aacs.common.DialogStateChangedMessages;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;

import org.greenrobot.eventbus.EventBus;

public class AACSBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = AACSBroadcastReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            switch (message.action) {
                case Action.AlexaClient.CONNECTION_STATUS_CHANGED:
                    handleConnectionStatusChanged(message);
                    break;
                case Action.AlexaClient.DIALOG_STATE_CHANGED:
                    handleDialogStateChanged(message);
                    break;
                case Action.SpeechRecognizer.WAKEWORD_DETECTED:
                    sendAutoVoiceInteractionMessage(
                            Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.WAKEWORD_DETECTED, "");
                    break;
                case Action.SpeechRecognizer.END_OF_SPEECH_DETECTED:
                    sendAutoVoiceInteractionMessage(
                            Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.END_OF_SPEECH_DETECTED, "");
                    break;
            }
        });
    }

    @VisibleForTesting
    void handleDialogStateChanged(@NonNull AACSMessage aacsMessage) {
        if (aacsMessage.payload != null) {
            DialogStateChangedMessages.parseDialogState(aacsMessage.payload).ifPresent(message -> {
                sendAutoVoiceInteractionMessage(Constants.TOPIC_VOICE_CHROME, message, "");
            });
        }
    }

    @VisibleForTesting
    void handleConnectionStatusChanged(@NonNull AACSMessage aacsMessage) {
        if (aacsMessage.payload != null) {
            ConnectionStatusChangedMessages.parseConnectionStatus(aacsMessage.payload).ifPresent(message -> {
                sendAutoVoiceInteractionMessage(Constants.TOPIC_ALEXA_CONNECTION, message, "");
            });
        }
    }

    @VisibleForTesting
    void sendAutoVoiceInteractionMessage(String topic, String action, String payload) {
        Log.d(TAG, "Sending Alexa Auto voice interaction message, topic: " + topic + ", action: " + action);
        EventBus.getDefault().post(new AutoVoiceInteractionMessage(topic, action, payload));
    }
}
