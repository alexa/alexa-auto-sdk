package com.amazon.alexa.auto.navigation.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.navigation.dependencies.AndroidModule;
import com.amazon.alexa.auto.navigation.dependencies.DaggerNavigationComponent;
import com.amazon.alexa.auto.navigation.handlers.NavigationDirectiveHandler;

import javax.inject.Inject;

/**
 * Receiver that gets navigation directives.
 */
public class NavigationReceiver extends BroadcastReceiver {
    private static final String TAG = NavigationReceiver.class.getSimpleName();

    @Inject
    NavigationDirectiveHandler mNavigationDirectiveHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mNavigationDirectiveHandler == null) {
            DaggerNavigationComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectNavigationReceiver(this);
        }

        Log.d(TAG, "onReceive: " + intent.getAction());
        if (intent.getExtras() != null) {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                if (Topic.NAVIGATION.equals(message.topic)) {
                    if (Action.Navigation.START_NAVIGATION.equals(message.action)
                            || Action.Navigation.CANCEL_NAVIGATION.equals(message.action)) {
                        mNavigationDirectiveHandler.handleNavigationCommand(message);
                    }
                }
            });
        }
    }
}
