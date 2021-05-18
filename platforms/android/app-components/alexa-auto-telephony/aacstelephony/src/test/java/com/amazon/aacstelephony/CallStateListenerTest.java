package com.amazon.aacstelephony;

import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.anyObject;
import static org.mockito.Matchers.anyString;

import android.content.Context;
import android.telecom.Call;

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
@PrepareForTest(Util.class)
public class CallStateListenerTest {
    private static String TEST_CALL_ID = "testCallID";
    private static String TEST_CALL_NUMBER = "12345";
    private CallStateListener mCallStateListener;

    @Mock
    private AACSMessageSender mAACSMessageSender;

    @Mock
    private Context mMockedContext;

    @Mock
    private Call mCall;

    @Mock
    private Context mMockedApplicationContext;

    @Before
    public void setUp() {
        PowerMockito.when(mMockedContext.getApplicationContext()).thenReturn(mMockedApplicationContext);
        PowerMockito.when(mMockedApplicationContext.getPackageName()).thenReturn("");
        mCallStateListener = new CallStateListener(mMockedContext, TEST_CALL_ID, TEST_CALL_NUMBER, mAACSMessageSender);
    }

    @Test
    public void onCallStateChanged() throws Exception {
        PowerMockito.mockStatic(Util.class);
        mCallStateListener.onStateChanged(mCall, anyInt());
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.publishCallStateToAACS(anyInt(), anyObject(), anyString(), anyString());
    }
}
