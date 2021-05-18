package com.amazon.alexa.auto.comms.ui.handler;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.ContactsControllerImpl;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import java.lang.ref.WeakReference;

/**
 * Handler for bluetooth directives coming from AACS.
 */
public class BluetoothDirectiveHandler {
    private final static String TAG = BluetoothDirectiveHandler.class.getSimpleName();

    Context mContext;
    BTDeviceRepository mBTDeviceRepository;
    ConnectedBTDeviceRepository mConnectedBTDeviceRepository;

    /**
     * Constructs the bluetooth commands handler.
     */
    public BluetoothDirectiveHandler(WeakReference<Context> context) {
        mContext = context.get();

        AlexaApp mApp = AlexaApp.from(mContext);
        if (!mApp.getRootComponent().getComponent(ContactsController.class).isPresent()) {
            mApp.getRootComponent().activateScope(new ContactsControllerImpl(context));
        }

        mBTDeviceRepository = BTDeviceRepository.getInstance(mContext);
        mConnectedBTDeviceRepository = ConnectedBTDeviceRepository.getInstance(mContext);
    }

    /**
     * Constructs the bluetooth commands handler.
     * This is created for unit testing.
     */
    @VisibleForTesting
    BluetoothDirectiveHandler(
            BTDeviceRepository btDeviceRepository, ConnectedBTDeviceRepository connectedBTDeviceRepository) {
        mBTDeviceRepository = btDeviceRepository;
        mConnectedBTDeviceRepository = connectedBTDeviceRepository;
    }

    /**
     * Handle bluetooth connection directive coming from AACS.
     *
     * @param device BT device.
     */
    public void handleBTConnectionCommand(@NonNull BTDevice device, String connectedState) {
        if (connectedState.equals(Constants.BT_CONNECTED)) {
            mBTDeviceRepository.insertEntry(device);
            mConnectedBTDeviceRepository.insertEntry(device);
        } else {
            // Device is disconnected, it needs to be removed from connected device database, when found, the remove
            // action will be triggered.
            mConnectedBTDeviceRepository.findConnectedBTDeviceEntry(device);
        }
    }
}
