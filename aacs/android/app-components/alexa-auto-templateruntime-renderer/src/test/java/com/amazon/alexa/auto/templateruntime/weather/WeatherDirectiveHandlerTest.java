package com.amazon.alexa.auto.templateruntime.weather;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.ViewGroup;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.WeatherTemplate;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.templateruntime.common.TestResourceFileReader;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;
import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class WeatherDirectiveHandlerTest {
    @Mock
    private Context mContext;
    @Mock
    private AlexaApp mMockAlexaApp;
    @Mock
    private AlexaAppRootComponent mMockRootComponent;
    @Mock
    private SessionViewController mMockSessionController;
    @Mock
    private ViewGroup mViewGroup;
    private WeatherDirectiveHandler mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
        when(mMockRootComponent.getComponent(SessionViewController.class))
                .thenReturn(Optional.of(mMockSessionController));
        when(mMockSessionController.getTemplateRuntimeViewContainer()).thenReturn(Optional.of(mViewGroup));
        mClassUnderTest = new WeatherDirectiveHandler(mContext);
    }

    @Test
    public void testOnRenderWeatherTemplate_renderLocalSearchListTest() {
        Intent intent = generateIntent("aacs/RenderTemplateWeather.json", "com.amazon.aacs.aasb.RenderTemplate");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(intent);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            WeatherDirectiveHandler spyWeatherDirectiveHandler = Mockito.spy(mClassUnderTest);
            Mockito.doNothing()
                    .when(spyWeatherDirectiveHandler)
                    .renderWeatherView(Mockito.any(ViewGroup.class), Mockito.any(WeatherTemplate.class));
            spyWeatherDirectiveHandler.renderTemplate(aacsMessage.get());

            ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
            Mockito.verify(spyWeatherDirectiveHandler, times(1)).renderTemplate(aacsMessageArgumentCaptor.capture());

            AACSMessage capturedAACSMessage = aacsMessageArgumentCaptor.getValue();
            Assert.assertEquals(capturedAACSMessage.action, "RenderTemplate");
            Assert.assertEquals(capturedAACSMessage.topic, "TemplateRuntime");
            Mockito.verify(mMockSessionController, times(1)).getTemplateRuntimeViewContainer();
        }
    }

    @Test
    public void testOnClearTemplate_shouldInvokeUnderlyingFunction() {
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            mClassUnderTest.clearTemplate();
            Mockito.verify(mMockSessionController, times(1)).clearTemplate();
        }
    }

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSnMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSnMessage.get());
        intent.putExtra("payload", sampleListeningPayload);
        return intent;
    }
}
