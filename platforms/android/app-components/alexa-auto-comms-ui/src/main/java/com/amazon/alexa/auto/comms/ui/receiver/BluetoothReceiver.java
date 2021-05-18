package com.amazon.alexa.auto.comms.ui.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.dependencies.AndroidModule;
import com.amazon.alexa.auto.comms.ui.dependencies.DaggerCommunicationComponent;
import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import javax.inject.Inject;

/**
 * Receiver that gets Android telephony bluetooth directives.
 */
public class BluetoothReceiver extends BroadcastReceiver {
    private static final String TAG = BluetoothReceiver.class.getSimpleName();

    @VisibleForTesting
    BTDevice mBTDevice = new BTDevice();

    @Inject
    BluetoothDirectiveHandler mBluetoothDirectiveHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mBluetoothDirectiveHandler == null) {
            Log.i(TAG, this + " | first onReceive so doing injection");
            DaggerCommunicationComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectBluetoothReceiver(this);
        }

        Log.d(TAG, "onReceive: " + intent.getAction());

        if (intent.getAction() != null && intent.getExtras() != null) {
            mBTDevice.setDeviceAddress(intent.getExtras().getString(Constants.AACS_BT_DEVICE_ADDRESS, ""));
            mBTDevice.setDeviceName(intent.getExtras().getString(Constants.AACS_BT_DEVICE_NAME, ""));

            String connectionState;
            if (intent.getAction().equals(Constants.AACS_BT_CONNECTED)) {
                connectionState = Constants.BT_CONNECTED;
            } else {
                connectionState = Constants.BT_DISCONNECTED;
            }

            mBluetoothDirectiveHandler.handleBTConnectionCommand(mBTDevice, connectionState);
        }
    }
}
