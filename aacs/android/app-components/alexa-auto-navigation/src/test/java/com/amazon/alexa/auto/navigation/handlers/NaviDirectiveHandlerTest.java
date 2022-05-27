package com.amazon.alexa.auto.navigation.handlers;

import static org.mockito.ArgumentMatchers.anyString;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigation;
import com.amazon.alexa.auto.navigation.providers.GenericNaviProvider;
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
public class NaviDirectiveHandlerTest {
    private NaviDirectiveHandler mClassUnderTest;
    private GenericNaviProvider mGenericNaviProvider;
    private Context mContext;

    @Before
    public void setup() {
        mGenericNaviProvider = Mockito.mock(GenericNaviProvider.class);
        mClassUnderTest = new NaviDirectiveHandler(mGenericNaviProvider);
        mContext = Mockito.mock(Context.class);
    }

    @Test
    public void startNavigationTest() {
        Intent startNavigationIntent =
                generateIntent("aacs/StartNavigation.json", "com.amazon.aacs.aasb.StartNavigation");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(startNavigationIntent);

        aacsMessage.ifPresent(message -> mClassUnderTest.handleNavigationCommand(message));

        ArgumentCaptor<StartNavigation> startNavigationArgumentCaptor = ArgumentCaptor.forClass(StartNavigation.class);

        Mockito.verify(mGenericNaviProvider, Mockito.times(1)).startNavigation(startNavigationArgumentCaptor.capture());

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
        aacsMessage.ifPresent(message -> mClassUnderTest.handleNavigationCommand(message));

        Mockito.verify(mGenericNaviProvider, Mockito.times(1)).cancelNavigation();
    }

    @Test
    public void getNavigationStateTest() {
        Intent getNavigationStateIntent =
                generateIntent("aacs/GetNavigationState.json", "com.amazon.aacs.aasb.GetNavigationState");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(getNavigationStateIntent);
        aacsMessage.ifPresent(message -> mClassUnderTest.handleNavigationCommand(message));

        Mockito.verify(mGenericNaviProvider, Mockito.times(1)).getNavigationState(anyString());
    }

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSNavigationMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleAACSNavigationMessage.ifPresent(s -> sampleListeningPayload.putString("message", s));
        intent.putExtra("payload", sampleListeningPayload);
        return intent;
    }
}
