package com.amazon.alexa.auto.setup.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.NetworkUtil;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import static com.amazon.aacsconstants.NetworkConstants.ANDROID_CONNECTIVITY_CHANGE_ACTION;
import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.ModuleName.LVC;
import static com.amazon.alexa.auto.apps.common.util.NetworkUtil.TYPE_NOT_CONNECTED;

/**
 * Receives changes to network state and publishes appropriate events
 */
public class NetworkStateChangeReceiver extends BroadcastReceiver {
    private static final String TAG = NetworkStateChangeReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (AlexaApp.from(context).getRootComponent().getAlexaSetupController().isSetupCompleted()) {
            return;
        } else if (ModuleProvider.containsModule(context, LVC)
            && AlexaApp.from(context).getRootComponent().getAuthController().isAuthenticated()) {
            return;
        }
        if (ANDROID_CONNECTIVITY_CHANGE_ACTION.equals(intent.getAction())) {
            int connectivityStatus = NetworkUtil.getConnectivityStatus(context);
            if (connectivityStatus == TYPE_NOT_CONNECTED) {
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.NETWORK_DISCONNECTED_EVENT));
            } else if (connectivityStatus == NetworkUtil.TYPE_WIFI
                    || connectivityStatus == NetworkUtil.TYPE_MOBILE) {
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.NETWORK_CONNECTED_EVENT));
            }
        }
    }
}
