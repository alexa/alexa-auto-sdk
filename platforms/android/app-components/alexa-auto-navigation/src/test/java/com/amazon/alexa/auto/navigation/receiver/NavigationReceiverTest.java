package com.amazon.alexa.auto.navigation.receiver;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.navigation.handlers.NavigationDirectiveHandler;

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
public class NavigationReceiverTest {
    private NavigationReceiver mClassUnderTest;
    private NavigationDirectiveHandler mNavigationDirectiveHandler;

    @Before
    public void setup() {
        mClassUnderTest = new NavigationReceiver();
        mNavigationDirectiveHandler = Mockito.mock(NavigationDirectiveHandler.class);
        mClassUnderTest.mNavigationDirectiveHandler = mNavigationDirectiveHandler;
    }

    @Test
    public void startNavigationTest() {
        Intent startNavigationIntent =
                generateIntent("aacs/StartNavigation.json", "com.amazon.aacs.aasb.StartNavigation");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), startNavigationIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mNavigationDirectiveHandler, Mockito.times(1))
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
        Intent startNavigationIntent =
                generateIntent("aacs/CancelNavigation.json", "com.amazon.aacs.aasb.CancelNavigation");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), startNavigationIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mNavigationDirectiveHandler, Mockito.times(1))
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

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSNavigationMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSNavigationMessage.get());
        intent.putExtra("payload", sampleListeningPayload);
        return intent;
    }
}
