package com.amazon.alexa.auto.navigation.receiver;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.navigation.handlers.NaviDirectiveHandler;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class NaviReceiverTest {
    private NaviReceiver mClassUnderTest;
    private NaviDirectiveHandler mNaviDirectiveHandler;

    @Before
    public void setup() {
        mClassUnderTest = new NaviReceiver();
        mNaviDirectiveHandler = Mockito.mock(NaviDirectiveHandler.class);
        mClassUnderTest.mNaviDirectiveHandler = mNaviDirectiveHandler;
    }

    @Test
    public void startNavigationTest() {
        Intent startNavigationIntent =
                generateIntent("aacs/StartNavigation.json", "com.amazon.aacs.aasb.StartNavigation");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), startNavigationIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mNaviDirectiveHandler, Mockito.times(1))
                .handleNavigationCommand(aacsMessageArgumentCaptor.capture());
        AACSMessage aacsMessage = aacsMessageArgumentCaptor.getValue();
        Assert.assertEquals(aacsMessage.topic, Topic.NAVIGATION);
        Assert.assertEquals(aacsMessage.action, Action.Navigation.START_NAVIGATION);

        Optional<String> sampleAACSNavigationMessage =
                TestResourceFileReader.readFileContent("aacs/StartNavigation.json");
        try {
            JSONObject sampleMessageJsonObject = new JSONObject(sampleAACSNavigationMessage.get());
            sampleMessageJsonObject.get("payload").equals(aacsMessage.payload);
        } catch (JSONException e) {
            throw new IllegalArgumentException();
        }
    }

    @Test
    public void cancelNavigationTest() {
        Intent cancelNavigationIntent =
                generateIntent("aacs/CancelNavigation.json", "com.amazon.aacs.aasb.CancelNavigation");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), cancelNavigationIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mNaviDirectiveHandler, Mockito.times(1))
                .handleNavigationCommand(aacsMessageArgumentCaptor.capture());
        AACSMessage aacsMessage = aacsMessageArgumentCaptor.getValue();
        Assert.assertEquals(aacsMessage.topic, Topic.NAVIGATION);
        Assert.assertEquals(aacsMessage.action, Action.Navigation.CANCEL_NAVIGATION);

        Optional<String> sampleAACSNavigationMessage =
                TestResourceFileReader.readFileContent("aacs/CancelNavigation.json");
        try {
            JSONObject sampleMessageJsonObject = new JSONObject(sampleAACSNavigationMessage.get());
            sampleMessageJsonObject.get("payload").equals(aacsMessage.payload);
        } catch (JSONException e) {
            throw new IllegalArgumentException();
        }
    }

    @Test
    public void getNavigationStateTest() {
        Intent getNavigationStateIntent =
                generateIntent("aacs/GetNavigationState.json", "com.amazon.aacs.aasb.GetNavigationState");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), getNavigationStateIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mNaviDirectiveHandler, Mockito.times(1))
                .handleNavigationCommand(aacsMessageArgumentCaptor.capture());
        AACSMessage aacsMessage = aacsMessageArgumentCaptor.getValue();
        Assert.assertEquals(aacsMessage.topic, Topic.NAVIGATION);
        Assert.assertEquals(aacsMessage.action, Action.Navigation.GET_NAVIGATION_STATE);

        Optional<String> sampleAACSNavigationMessage =
                TestResourceFileReader.readFileContent("aacs/GetNavigationState.json");
        try {
            JSONObject sampleMessageJsonObject = new JSONObject(sampleAACSNavigationMessage.get());
            sampleMessageJsonObject.get("payload").equals(aacsMessage.payload);
        } catch (JSONException e) {
            throw new IllegalArgumentException();
        }
    }

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSNavigationMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSNavigationMessage.get());
        intent.putExtra("payload", sampleListeningPayload);
        return intent;
    }
}
