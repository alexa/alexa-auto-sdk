package com.amazon.alexa.auto.comms.ui.handler;

import android.content.Intent;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class BluetoothDirectiveHandlerTest {
    @Mock
    private BTDeviceRepository mBTDeviceRepository;
    @Mock
    private ConnectedBTDeviceRepository mConnectedBTDeviceRepository;

    private BluetoothDirectiveHandler mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        mClassUnderTest = new BluetoothDirectiveHandler(mBTDeviceRepository, mConnectedBTDeviceRepository);
    }

    @Test
    public void handleConnectedBTDeviceTest() {
        Intent btDeviceConnectedIntent = generateIntent("com.amazon.aacstelephony.bluetooth.connected");
        BTDevice device = generateBTDevice(btDeviceConnectedIntent);
        mClassUnderTest.handleBTConnectionCommand(device, Constants.BT_CONNECTED);
        Mockito.verify(mBTDeviceRepository, Mockito.times(1)).insertEntry(device);
        Mockito.verify(mConnectedBTDeviceRepository, Mockito.times(1)).insertEntry(device);
    }

    @Test
    public void handleDisconnectedBTDeviceTest() {
        Intent btDeviceConnectedIntent = generateIntent("com.amazon.aacstelephony.bluetooth.disconnected");
        BTDevice device = generateBTDevice(btDeviceConnectedIntent);
        mClassUnderTest.handleBTConnectionCommand(device, Constants.BT_DISCONNECTED);
        Mockito.verify(mConnectedBTDeviceRepository, Mockito.times(1)).findConnectedBTDeviceEntry(device);
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
