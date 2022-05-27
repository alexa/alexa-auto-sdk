package com.amazon.alexa.auto.setup.workflow.receiver;

import static com.amazon.aacsconstants.NetworkConstants.ANDROID_CONNECTIVITY_CHANGE_ACTION;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.NETWORK_CONNECTED_EVENT;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.NETWORK_DISCONNECTED_EVENT;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import com.amazon.alexa.auto.setup.receiver.NetworkStateChangeReceiver;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.reflect.Field;

@RunWith(RobolectricTestRunner.class)
public class NetworkStateChangeReceiverTest {
    private NetworkStateChangeReceiver mClassUnderTest;
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
    public void setup() throws Exception {
        MockitoAnnotations.openMocks(this);

        doReturn(mMockConnectivityManager).when(mMockContext).getSystemService(eq(Context.CONNECTIVITY_SERVICE));
        doReturn(mMockNetworkInfo).when(mMockConnectivityManager).getActiveNetworkInfo();
        doReturn(mMockContext).when(mMockApplication).getApplicationContext();

        mClassUnderTest = new NetworkStateChangeReceiver();

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnNetworkConnected_sendNetworkConnectedEvent() {
        doReturn(true).when(mMockNetworkInfo).isConnectedOrConnecting();
        doReturn(ConnectivityManager.TYPE_WIFI).when(mMockNetworkInfo).getType();

        Intent intent = new Intent(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        mClassUnderTest.onReceive(mMockContext, intent);

        ArgumentCaptor<WorkflowMessage> argumentCaptor = ArgumentCaptor.forClass(WorkflowMessage.class);
        verify(EventBus.getDefault()).post(argumentCaptor.capture());
        assertEquals(argumentCaptor.getValue().getWorkflowEvent(), NETWORK_CONNECTED_EVENT);
    }

    @Test
    public void testOnNetworkDisconnected_sendNetworkDisconnectedEvent() {
        doReturn(null).when(mMockConnectivityManager).getActiveNetworkInfo();

        Intent intent = new Intent(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        mClassUnderTest.onReceive(mMockContext, intent);

        ArgumentCaptor<WorkflowMessage> argumentCaptor = ArgumentCaptor.forClass(WorkflowMessage.class);
        verify(EventBus.getDefault()).post(argumentCaptor.capture());
        assertEquals(argumentCaptor.getValue().getWorkflowEvent(), NETWORK_DISCONNECTED_EVENT);
    }
}
