package com.amazon.alexa.auto.navigation.handlers;

import android.content.Intent;
import android.os.Bundle;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.StartNavigation;
import com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider;
import com.amazon.alexa.auto.navigation.receiver.TestResourceFileReader;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class NavigationDirectiveHandlerTest {
    private NavigationDirectiveHandler mClassUnderTest;
    private GoogleMapsNavigationProvider mGoogleMapsNavigationProvider;

    @Before
    public void setup() {
        mGoogleMapsNavigationProvider = Mockito.mock(GoogleMapsNavigationProvider.class);
        mClassUnderTest = new NavigationDirectiveHandler(mGoogleMapsNavigationProvider);
    }

    @Test
    public void startNavigationTest() {
        Intent startNavigationIntent =
                generateIntent("aacs/StartNavigation.json", "com.amazon.aacs.aasb.StartNavigation");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(startNavigationIntent);

        mClassUnderTest.handleNavigationCommand(aacsMessage.get());
        ArgumentCaptor<StartNavigation> startNavigationArgumentCaptor = ArgumentCaptor.forClass(StartNavigation.class);

        Mockito.verify(mGoogleMapsNavigationProvider, Mockito.times(1))
                .startNavigation(startNavigationArgumentCaptor.capture());

        StartNavigation startNavigation = startNavigationArgumentCaptor.getValue();
        Assert.assertEquals(startNavigation.getTransportationMode(), "DRIVING");
        Assert.assertEquals(startNavigation.getWaypoints().size(), 1);
        Assert.assertEquals(startNavigation.getWaypoints().get(0).getName(), "Home");
    }

    @Test
    public void cancelNavigationTest() {
        Intent cancelNavigationIntent =
                generateIntent("aacs/CancelNavigation.json", "com.amazon.aacs.aasb.CancelNavigation");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(cancelNavigationIntent);
        mClassUnderTest.handleNavigationCommand(aacsMessage.get());

        Mockito.verify(mGoogleMapsNavigationProvider, Mockito.times(1)).cancelNavigation();
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
