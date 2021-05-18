package com.amazon.alexa.auto.comms.ui.receiver;

import android.content.Context;
import android.content.Intent;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class BluetoothReceiverTest {
    private BluetoothReceiver mClassUnderTest;

    private BluetoothDirectiveHandler mBluetoothDirectiveHandler;

    @Before
    public void setup() {
        mClassUnderTest = new BluetoothReceiver();
        mBluetoothDirectiveHandler = Mockito.mock(BluetoothDirectiveHandler.class);
        mClassUnderTest.mBluetoothDirectiveHandler = mBluetoothDirectiveHandler;
    }

    @Test
    public void btDeviceConnectedTest() {
        Intent btDeviceConnectedIntent = generateIntent("com.amazon.aacstelephony.bluetooth.connected");
        BTDevice device = generateBTDevice(btDeviceConnectedIntent);
        mClassUnderTest.mBTDevice = device;
        mClassUnderTest.onReceive(Mockito.mock(Context.class), btDeviceConnectedIntent);
        Mockito.verify(mBluetoothDirectiveHandler, Mockito.times(1))
                .handleBTConnectionCommand(device, Constants.BT_CONNECTED);
    }

    @Test
    public void btDeviceDisconnectedTest() {
        Intent btDeviceConnectedIntent = generateIntent("com.amazon.aacstelephony.bluetooth.disconnected");
        BTDevice device = generateBTDevice(btDeviceConnectedIntent);
        mClassUnderTest.mBTDevice = device;
        mClassUnderTest.onReceive(Mockito.mock(Context.class), btDeviceConnectedIntent);
        Mockito.verify(mBluetoothDirectiveHandler, Mockito.times(1))
                .handleBTConnectionCommand(device, Constants.BT_DISCONNECTED);
    }

    private Intent generateIntent(String action) {
        Intent intent = new Intent(action);
        intent.putExtra("deviceName", "testDeviceName");
        intent.putExtra("deviceAddress", "testDevicAddress");
        return intent;
    }

    private BTDevice generateBTDevice(Intent intent) {
        BTDevice device = new BTDevice();
        device.setDeviceAddress(intent.getExtras().getString(Constants.AACS_BT_DEVICE_ADDRESS, ""));
        device.setDeviceName(intent.getExtras().getString(Constants.AACS_BT_DEVICE_NAME, ""));
        return device;
    }
}
