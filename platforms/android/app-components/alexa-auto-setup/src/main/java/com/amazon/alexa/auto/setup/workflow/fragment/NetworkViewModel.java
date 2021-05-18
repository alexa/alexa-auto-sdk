package com.amazon.alexa.auto.setup.workflow.fragment;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.lifecycle.AndroidViewModel;

import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;
import com.amazon.alexa.auto.setup.workflow.util.NetworkUtil;

import org.greenrobot.eventbus.EventBus;

/**
 * ViewModel for @{link NetworkFragment}
 */
public class NetworkViewModel extends AndroidViewModel {
    private static final String TAG = NetworkViewModel.class.getSimpleName();

    private final static String ANDROID_CONNECTIVITY_CHANGE_ACTION = "android.net.conn.CONNECTIVITY_CHANGE";

    private EventBus mEventBus;

    @VisibleForTesting
    NetworkBroadcastReceiver broadcastReceiver;

    /**
     * Constructor for NetworkViewModel.
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public NetworkViewModel(@NonNull Application application) {
        super(application);

        mEventBus = EventBus.getDefault();
        broadcastReceiver = new NetworkBroadcastReceiver(mEventBus);
        observeNetworkStatus();
    }

    /**
     * Constructor for NetworkViewModel.
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     * @param eventBus Event bus.
     */
    @VisibleForTesting
    NetworkViewModel(@NonNull Application application, EventBus eventBus) {
        super(application);

        mEventBus = eventBus;
        broadcastReceiver = new NetworkBroadcastReceiver(mEventBus);

        observeNetworkStatus();
    }

    @Override
    public void onCleared() {
        getApplication().getApplicationContext().unregisterReceiver(broadcastReceiver);
    }

    public void goToNetworkSettings() {
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getApplication().getApplicationContext().startActivity(intent);
    }

    private void observeNetworkStatus() {
        IntentFilter intentFilter = new IntentFilter(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        getApplication().getApplicationContext().registerReceiver(broadcastReceiver, intentFilter);
    }

    static class NetworkBroadcastReceiver extends BroadcastReceiver {
        private EventBus mEventBus;

        @VisibleForTesting
        WorkflowMessage mNetworkConnectedMessage;

        NetworkBroadcastReceiver(EventBus eventBus) {
            mEventBus = eventBus;
            mNetworkConnectedMessage = new WorkflowMessage(LoginEvent.NETWORK_CONNECTED_EVENT);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            int status = NetworkUtil.getConnectivityStatusString(context);
            if (ANDROID_CONNECTIVITY_CHANGE_ACTION.equals(intent.getAction())) {
                if (status == NetworkUtil.NETWORK_STATUS_WIFI || status == NetworkUtil.TYPE_MOBILE) {
                    mEventBus.post(mNetworkConnectedMessage);
                } else {
                    Log.d(TAG, "Network is not yet connected, new workflow event will not be published");
                }
            }
        }
    }
}
