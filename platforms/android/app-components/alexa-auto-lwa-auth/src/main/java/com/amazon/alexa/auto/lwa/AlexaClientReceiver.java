package com.amazon.alexa.auto.lwa;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.ConnectionStatusChangedMessages;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;

import org.greenrobot.eventbus.EventBus;

public class AlexaClientReceiver extends BroadcastReceiver {
    private static final String TAG = AlexaClientReceiver.class.getSimpleName();

    private static final String ALEXA_CLIENT_CONNECTED = "CONNECTED";
    private static final String ALEXA_CLIENT_DISCONNECTED = "DISCONNECTED";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (Action.AlexaClient.CONNECTION_STATUS_CHANGED.equals(message.action)) {
                handleConnectionStateChanged(message);
            }
        });
    }

    /**
     * Handles Alexa Client AASB message with ConnectionStatusChanged action.
     */
    private void handleConnectionStateChanged(AACSMessage aacsMessage) {
        if (aacsMessage.payload != null) {
            ConnectionStatusChangedMessages.parseConnectionStatus(aacsMessage.payload).ifPresent(message -> {
                if (message.equals(ALEXA_CLIENT_CONNECTED)) {
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Alexa_Client_Connected, null, null));
                } else if (message.equals(ALEXA_CLIENT_DISCONNECTED)) {
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Alexa_Client_Disconnected, null, null));
                }
            });
        }
    }
}
