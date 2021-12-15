package com.amazon.aacstelephony;

import static org.mockito.Mockito.never;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.provider.CallLog;
import android.telecom.Call;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Util.class, Uri.class, Bundle.class, CallLog.Calls.class})
public class PhoneCallControllerTest {
    private static String TEST_CALL_ID = "testCallID";
    private static String TEST_CALL_NUMBER = "12345";
    private PhoneCallController mPhoneCallController;

    @Mock
    private PhoneAccountHandle mMockedHandle;

    @Mock
    private Context mMockedContext;

    @Mock
    private AACSMessageSender mAACSMessageSender;

    @Mock
    private TelecomManager mTelecomManager;

    @Mock
    private Call mCall;

    @Before
    public void setUp() {
        mPhoneCallController = new PhoneCallController(mMockedContext, mAACSMessageSender, mTelecomManager);

        // Mock Util methods
        PowerMockito.mockStatic(Util.class);
        PowerMockito.when(Util.bluetoothNotConnected()).thenReturn(false);
    }

    @After
    public void tearDown() {
        mPhoneCallController.cleanUp();
        Assert.assertNull(mPhoneCallController.getCallMap());
    }

    public void setUpForDialing() throws Exception {
        Uri uri = Uri.fromParts(PhoneAccount.SCHEME_TEL, TEST_CALL_NUMBER, null);
        Bundle extra = new Bundle();
        PowerMockito.whenNew(Bundle.class).withAnyArguments().thenReturn(extra);
        PowerMockito.doNothing().when(mTelecomManager).placeCall(uri, extra);
    }

    public void setUpIncomingCall(boolean validCase) {
        if (validCase)
            mPhoneCallController.getCallMap().addCall(mCall, TEST_CALL_ID, TEST_CALL_NUMBER);
        else
            mPhoneCallController.getCallMap().addCall(null, TEST_CALL_ID, TEST_CALL_NUMBER);
    }

    public void setUpActiveCall() {
        mPhoneCallController.getCallMap().addCall(mCall, TEST_CALL_ID, TEST_CALL_NUMBER);
        PowerMockito.when(mCall.getState()).thenReturn(Call.STATE_ACTIVE);
    }

    @Test
    public void dial_correctParameters() throws Exception {
        setUpForDialing();

        mPhoneCallController.dial(TEST_CALL_ID, TEST_CALL_NUMBER, mMockedHandle);
        CallMap callMap = mPhoneCallController.getCallMap();
        Assert.assertNotNull(mMockedHandle);
        Assert.assertNotNull(callMap);
        Assert.assertNotNull(callMap.getOutgoingCallInfo());
        Mockito.verify(mTelecomManager, Mockito.times(1)).placeCall(Mockito.anyObject(), Mockito.anyObject());
    }

    @Test
    public void dial_nullPhoneAccount() throws Exception {
        setUpForDialing();

        mPhoneCallController.dial(TEST_CALL_ID, TEST_CALL_NUMBER, null);
        Assert.assertNull(mPhoneCallController.getCallMap().getOutgoingCallInfo());
        Mockito.verify(mTelecomManager, never()).placeCall(Mockito.anyObject(), Mockito.anyObject());
    }

    @Test
    public void redial_validNumber() throws Exception {
        setUpForDialing();

        PowerMockito.mockStatic(CallLog.Calls.class);
        PowerMockito.when(CallLog.Calls.getLastOutgoingCall(mMockedContext)).thenReturn("12345");
        mPhoneCallController.redial(TEST_CALL_ID, mMockedHandle);
        Assert.assertNotNull(mPhoneCallController.getCallMap().getOutgoingCallInfo());
        Mockito.verify(mTelecomManager, Mockito.times(1)).placeCall(Mockito.anyObject(), Mockito.anyObject());
    }

    @Test
    public void redial_noLastDialedNumber() throws Exception {
        setUpForDialing();

        PowerMockito.mockStatic(CallLog.Calls.class);
        PowerMockito.when(CallLog.Calls.getLastOutgoingCall(mMockedContext)).thenReturn("");
        mPhoneCallController.redial(TEST_CALL_ID, mMockedHandle);
        Assert.assertNull(mPhoneCallController.getCallMap().getOutgoingCallInfo());
        Mockito.verify(mTelecomManager, never()).placeCall(Mockito.anyObject(), Mockito.anyObject());
    }

    @Test
    public void answer_validCall() {
        setUpIncomingCall(true);

        mPhoneCallController.answer(TEST_CALL_ID);
        Assert.assertEquals(mCall, mPhoneCallController.getCallMap().getCall(TEST_CALL_ID));
        Mockito.verify(mCall, Mockito.times(1)).answer(0);
    }

    @Test
    public void answer_noCall() {
        setUpIncomingCall(false);

        mPhoneCallController.answer(TEST_CALL_ID);
        Assert.assertNull(mPhoneCallController.getCallMap().getCall(TEST_CALL_ID));
        Mockito.verify(mCall, never()).answer(0);
    }

    @Test
    public void reject_incomingCall() {
        setUpIncomingCall(true);

        PowerMockito.when(mCall.getState()).thenReturn(Call.STATE_RINGING);

        mPhoneCallController.stop(TEST_CALL_ID);
        Mockito.verify(mCall, Mockito.times(1)).reject(false, null);
    }

    @Test
    public void stop_activeCall() {
        setUpActiveCall();

        mPhoneCallController.stop(TEST_CALL_ID);
        Mockito.verify(mCall, Mockito.times(1)).disconnect();
    }

    @Test
    public void sendDTMF() {
        setUpActiveCall();

        PowerMockito.doNothing().when(mCall).playDtmfTone(Mockito.anyChar());
        PowerMockito.doNothing().when(mCall).stopDtmfTone();

        mPhoneCallController.sendDTMF(TEST_CALL_ID, "12");
        Mockito.verify(mCall, Mockito.times(1)).playDtmfTone('1');
        Mockito.verify(mCall, Mockito.times(1)).playDtmfTone('2');
    }

    @Test
    public void initialCallStateCheck_hasCall() {
        PowerMockito.mockStatic(Util.class);
        mPhoneCallController.getCallMap().addCall(mCall, TEST_CALL_ID, TEST_CALL_NUMBER);
        mPhoneCallController.initialCallStateCheck();
        PowerMockito.verifyStatic(Mockito.times(1));
        Util.checkAndReportCurrentCalls(mPhoneCallController.getCallMap().getCallIdCallInfoMap(), mAACSMessageSender);
    }

    @Test
    public void initialCallStateCheck_noCall() {
        PowerMockito.mockStatic(Util.class);
        mPhoneCallController.initialCallStateCheck();
        PowerMockito.verifyStatic(Mockito.never());
        Util.checkAndReportCurrentCalls(mPhoneCallController.getCallMap().getCallIdCallInfoMap(), mAACSMessageSender);
    }
}
