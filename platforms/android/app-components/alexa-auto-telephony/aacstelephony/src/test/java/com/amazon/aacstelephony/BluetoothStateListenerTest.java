package com.amazon.aacstelephony;

import static org.mockito.Matchers.anyBoolean;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.anyObject;
import static org.mockito.Matchers.anyString;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

@RunWith(PowerMockRunner.class)
@PrepareForTest({BluetoothProfile.class, Util.class, BluetoothAdapter.class})
public class BluetoothStateListenerTest {
    private BluetoothStateListener mBluetoothStateListener;

    @Mock
    private AACSMessageSender mAACSMessageSender;

    @Mock
    private BluetoothAdapter mBluetoothAdapter;

    @Mock
    private Context mMockedContext;

    @Mock
    private Intent mMockedIntent;

    @Mock
    private BluetoothDevice mDevice;

    @Before
    public void setUp() {
        mBluetoothStateListener = new BluetoothStateListener(mAACSMessageSender);
        PowerMockito.mockStatic(BluetoothAdapter.class);
        PowerMockito.when(BluetoothAdapter.getDefaultAdapter()).thenReturn(mBluetoothAdapter);
    }

    @Test
    public void initialConnectionCheck_connected() {
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(mBluetoothAdapter.getProfileConnectionState(Mockito.anyInt()))
                .thenReturn(BluetoothAdapter.STATE_CONNECTED);
        mBluetoothStateListener.initialConnectionCheck(mMockedContext);
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.publishConnectionStateToAACS(anyString(), anyObject());
    }

    @Test
    public void initialConnectionCheck_disconnected() {
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(mBluetoothAdapter.getProfileConnectionState(Mockito.anyInt()))
                .thenReturn(BluetoothAdapter.STATE_DISCONNECTED);
        mBluetoothStateListener.initialConnectionCheck(mMockedContext);
        PowerMockito.verifyStatic(Mockito.times(0));
        Util.publishConnectionStateToAACS(anyString(), anyObject());
    }

    @Test
    public void onReceivePBAPConnectionChanged() {
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(mMockedIntent.getAction()).thenReturn(Constants.ACTION_BLUETOOTH_PBAP_CLIENT_STATE_CHANGED);
        PowerMockito.when(mMockedIntent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED))
                .thenReturn(anyInt());
        PowerMockito.when(mMockedIntent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)).thenReturn(mDevice);

        mBluetoothStateListener.onReceive(mMockedContext, mMockedIntent);
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.broadcastBluetoothState(anyObject(), anyBoolean(), anyString(), anyString());
    }

    @Test
    public void onReceiveHFPConnectionChanged_connected() {
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(mMockedIntent.getAction()).thenReturn(Constants.ACTION_BLUETOOTH_HFP_CLIENT_STATE_CHANGED);
        PowerMockito.when(mMockedIntent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED))
                .thenReturn(BluetoothAdapter.STATE_CONNECTED);
        mBluetoothStateListener.onReceive(mMockedContext, mMockedIntent);
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.publishConnectionStateToAACS(Constants.ConnectionState.CONNECTED, mAACSMessageSender);
    }

    @Test
    public void onReceivedHFPConnectionChanged_disconnected() {
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(mMockedIntent.getAction()).thenReturn(Constants.ACTION_BLUETOOTH_HFP_CLIENT_STATE_CHANGED);
        PowerMockito.when(mMockedIntent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED))
                .thenReturn(BluetoothAdapter.STATE_DISCONNECTED);
        mBluetoothStateListener.onReceive(mMockedContext, mMockedIntent);
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.publishConnectionStateToAACS(Constants.ConnectionState.DISCONNECTED, mAACSMessageSender);
    }
}
