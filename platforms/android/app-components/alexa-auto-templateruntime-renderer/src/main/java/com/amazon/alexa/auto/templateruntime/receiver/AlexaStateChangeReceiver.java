package com.amazon.alexa.auto.templateruntime.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.DialogStateChangedMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.templateruntime.R;

import org.greenrobot.eventbus.EventBus;

/**
 * Receives Alexa dialog state change broadcasts. When state changes to idle and weather template is 
 * rendered on the screen, it publishes an event which is received by
 * {@link com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler#OnReceive(AlexaVoiceoverCompletedMessage)}
 */
public class AlexaStateChangeReceiver extends BroadcastReceiver {
    private static final String TAG = TemplateRuntimeReceiver.class.getSimpleName();
    public static final String ALEXA_VOICE_INTERACTION_STATE_IDLE = "IDLE";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (Action.AlexaClient.DIALOG_STATE_CHANGED.equals(message.action)) {
                DialogStateChangedMessages.parseDialogState(message.payload)
                        .filter(state -> state.equals(ALEXA_VOICE_INTERACTION_STATE_IDLE))
                        .flatMap(state -> AlexaApp.from(context).getRootComponent().getComponent(SessionViewController.class))
                        .flatMap(SessionViewController::getTemplateRuntimeViewContainer)
                        .ifPresent(viewGroup -> {
                    if (viewGroup.findViewById(R.id.template_weather_view) != null) {
                        EventBus.getDefault().post(new AlexaVoiceoverCompletedMessage(System.currentTimeMillis()));
                    }
                });
            }
        });
    }
}
