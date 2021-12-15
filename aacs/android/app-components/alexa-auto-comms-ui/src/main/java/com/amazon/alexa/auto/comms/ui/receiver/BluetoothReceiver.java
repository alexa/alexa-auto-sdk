package com.amazon.alexa.auto.comms.ui.receiver;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.dependencies.AndroidModule;
import com.amazon.alexa.auto.comms.ui.dependencies.DaggerCommunicationComponent;
import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import javax.inject.Inject;

import static com.amazon.alexa.auto.comms.ui.Constants.AACS_BT_CONNECTION_CHECK_COMPLETED;
import static com.amazon.alexa.auto.comms.ui.Constants.ALEXA_AUTO_COMMS_PRIMARY_PHONE_CHANGED;

/**
 * Receiver that gets Android telephony bluetooth directives.
 */
public class BluetoothReceiver extends BroadcastReceiver {
    private static final String TAG = BluetoothReceiver.class.getSimpleName();

    @VisibleForTesting
    BTDevice mBTDevice = new BTDevice();

    @Inject
    BluetoothDirectiveHandler mBluetoothDirectiveHandler;

    @Inject
    TelecomManager mTelecomManager;

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
        if ( intent.getAction() != null && (intent.getAction().equals(AACS_BT_CONNECTION_CHECK_COMPLETED)
                || intent.getAction().equals(ALEXA_AUTO_COMMS_PRIMARY_PHONE_CHANGED))) {
            mBluetoothDirectiveHandler.handlePrimaryPhoneChangedCommand(getPrimaryDevice());
            return;
        }

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

    public String getPrimaryDevice() {
        String deviceAddress = "";
        if (mTelecomManager != null) {
            PhoneAccountHandle handle = mTelecomManager.getDefaultOutgoingPhoneAccount(PhoneAccount.SCHEME_TEL);
            if (handle != null) {
                deviceAddress = handle.getId();
            }
        }
        if (deviceAddress.isEmpty() || !BluetoothAdapter.checkBluetoothAddress(deviceAddress)) {
            Log.e(TAG, "cannot find any valid primary device.");
            return null;
        }
        return deviceAddress;
    }
}
