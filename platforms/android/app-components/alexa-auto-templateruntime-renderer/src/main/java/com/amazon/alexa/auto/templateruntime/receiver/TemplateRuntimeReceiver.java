package com.amazon.alexa.auto.templateruntime.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.templateruntime.dependencies.AndroidModule;
import com.amazon.alexa.auto.templateruntime.dependencies.DaggerTemplateRuntimeComponent;
import com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler;

import javax.inject.Inject;

import static com.amazon.aacsconstants.TemplateRuntimeConstants.TEMPLATE_TYPE_WEATHER;

/**
 * Receives Weather Template Runtime messages, unpacks it and renders view
 */
public class TemplateRuntimeReceiver extends BroadcastReceiver {
    private static final String TAG = TemplateRuntimeReceiver.class.getSimpleName();

    @Inject
    WeatherDirectiveHandler mWeatherDirectiveHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, this + " | onReceive: intent: " + intent);
        if (mWeatherDirectiveHandler == null) {
            Log.i(TAG, this + " | inject handler on first broadcast receipt");
            DaggerTemplateRuntimeComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectTemplateRuntimeReceiver(this);
        }

        if (intent != null && intent.getExtras() != null) {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                if (message.topic.equals(Topic.TEMPLATE_RUNTIME)
                        && message.action.equals(Action.TemplateRuntime.RENDER_TEMPLATE)) {
                    TemplateRuntimeMessages.getTemplateType(message.payload).ifPresent(type -> {
                        if (TEMPLATE_TYPE_WEATHER.equals(type)) {
                            // clearing template in case there is one currently rendered
                            mWeatherDirectiveHandler.clearTemplate();
                            mWeatherDirectiveHandler.renderWeatherTemplate(message);
                        } else {
                            Log.i(TAG, "Unknown template type: " + type);
                        }
                    });
                }
            });
        }
    }
}
