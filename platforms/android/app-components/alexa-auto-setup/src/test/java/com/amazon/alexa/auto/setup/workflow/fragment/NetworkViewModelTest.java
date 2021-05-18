package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class NetworkViewModelTest {
    private final static String ANDROID_CONNECTIVITY_CHANGE_ACTION = "android.net.conn.CONNECTIVITY_CHANGE";

    private NetworkViewModel mClassUnderTest;
    private NetworkViewModel.NetworkBroadcastReceiver networkBroadcastReceiver;

    @Mock
    Application mMockApplication;
    @Mock
    ConnectivityManager mMockConnectivityManager;
    @Mock
    NetworkInfo mMockNetworkInfo;
    @Mock
    Context mMockContext;
    @Mock
    EventBus mMockEventBus;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        doReturn(mMockConnectivityManager).when(mMockContext).getSystemService(eq(Context.CONNECTIVITY_SERVICE));

        doReturn(mMockNetworkInfo).when(mMockConnectivityManager).getActiveNetworkInfo();

        doReturn(mMockContext).when(mMockApplication).getApplicationContext();

        mClassUnderTest = new NetworkViewModel(mMockApplication, mMockEventBus);

        networkBroadcastReceiver = new NetworkViewModel.NetworkBroadcastReceiver(mMockEventBus);
        mClassUnderTest.broadcastReceiver = networkBroadcastReceiver;
    }

    @Test
    public void testSendConnectedEventIfNetworkConnected() {
        doReturn(true).when(mMockNetworkInfo).isConnectedOrConnecting();

        doReturn(ConnectivityManager.TYPE_WIFI).when(mMockNetworkInfo).getType();

        Intent intent = new Intent(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        networkBroadcastReceiver.onReceive(mMockContext, intent);

        verify(mMockEventBus, times(1)).post(networkBroadcastReceiver.mNetworkConnectedMessage);
    }

    @Test
    public void testNeverSendConnectedEventIfNetworkDisconnected() {
        doReturn(null).when(mMockConnectivityManager).getActiveNetworkInfo();

        Intent intent = new Intent(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        networkBroadcastReceiver.onReceive(mMockContext, intent);

        verify(mMockEventBus, never()).post(networkBroadcastReceiver.mNetworkConnectedMessage);
    }
}
