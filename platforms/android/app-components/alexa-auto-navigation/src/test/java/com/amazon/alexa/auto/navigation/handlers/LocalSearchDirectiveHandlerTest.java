package com.amazon.alexa.auto.navigation.handlers;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.when;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.LocalSearchDetailTemplate;
import com.amazon.alexa.auto.aacs.common.LocalSearchListTemplate;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponents;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;
import com.amazon.alexa.auto.navigation.receiver.TestResourceFileReader;

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
public class LocalSearchDirectiveHandlerTest {
    @Mock
    private NavigationProvider mNavigationProvider;
    @Mock
    private Context mContext;
    @Mock
    private Application mMockApplication;
    @Mock
    private AlexaApp mMockAlexaApp;
    @Mock
    private AlexaAppRootComponent mMockRootComponent;
    @Mock
    private SessionViewController mMockSessionController;
    @Mock
    private ViewGroup mViewGroup;
    @Mock
    private AlexaAppScopedComponents mMockScopedComponents;
    private LocalSearchDirectiveHandler mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
        when(mMockRootComponent.getScopedComponents()).thenReturn(mMockScopedComponents);
        when(mMockScopedComponents.getComponent(SessionViewController.class))
                .thenReturn(Optional.of(mMockSessionController));
        when(mMockSessionController.getTemplateRuntimeViewContainer()).thenReturn(Optional.of(mViewGroup));
        mClassUnderTest = new LocalSearchDirectiveHandler(new WeakReference<>(mContext), mNavigationProvider);
    }

    @Test
    public void renderLocalSearchListTest() {
        Intent startNavigationIntent =
                generateIntent("aacs/RenderTemplateLocalSearchList.json", "com.amazon.aacs.aasb.RenderTemplate");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(startNavigationIntent);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            LocalSearchDirectiveHandler spyLocalSearchDirectiveHandler = Mockito.spy(mClassUnderTest);
            Mockito.doNothing()
                    .when(spyLocalSearchDirectiveHandler)
                    .renderLocalSearchListView(
                            Mockito.any(ViewGroup.class), Mockito.any(LocalSearchListTemplate.class));
            spyLocalSearchDirectiveHandler.renderLocalSearchListTemplate(aacsMessage.get());

            ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
            Mockito.verify(spyLocalSearchDirectiveHandler, times(1))
                    .renderLocalSearchListTemplate(aacsMessageArgumentCaptor.capture());

            AACSMessage capturedAACSMessage = aacsMessageArgumentCaptor.getValue();
            Assert.assertEquals(capturedAACSMessage.action, "RenderTemplate");
            Assert.assertEquals(capturedAACSMessage.topic, "TemplateRuntime");
            Mockito.verify(mMockSessionController, times(1)).getTemplateRuntimeViewContainer();
        }
    }

    @Test
    public void renderLocalSearchDetailTest() {
        Intent renderTemplateIntent =
                generateIntent("aacs/RenderTemplateLocalSearchDetail.json", "com.amazon.aacs.aasb.RenderTemplate");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(renderTemplateIntent);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            LocalSearchDirectiveHandler spyLocalSearchDirectiveHandler = Mockito.spy(mClassUnderTest);
            Mockito.doNothing()
                    .when(spyLocalSearchDirectiveHandler)
                    .renderLocalSearchDetailView(
                            Mockito.any(ViewGroup.class), Mockito.any(LocalSearchDetailTemplate.class));
            spyLocalSearchDirectiveHandler.renderLocalSearchDetailTemplate(aacsMessage.get());

            ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
            Mockito.verify(spyLocalSearchDirectiveHandler, times(1))
                    .renderLocalSearchDetailTemplate(aacsMessageArgumentCaptor.capture());

            AACSMessage capturedAACSMessage = aacsMessageArgumentCaptor.getValue();
            Assert.assertEquals(capturedAACSMessage.action, "RenderTemplate");
            Assert.assertEquals(capturedAACSMessage.topic, "TemplateRuntime");
            Mockito.verify(mMockSessionController, times(1)).getTemplateRuntimeViewContainer();
        }
    }

    @Test
    public void testGettingDateTimeStringTest() {
        Intent renderTemplateIntent =
                generateIntent("aacs/RenderTemplateLocalSearchDetail.json", "com.amazon.aacs.aasb.RenderTemplate");
        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(renderTemplateIntent);

        if (aacsMessage.isPresent()) {
            Optional<LocalSearchDetailTemplate> localSearchDetail =
                    TemplateRuntimeMessages.parseLocalSearchDetailTemplate(aacsMessage.get().payload);
            if (localSearchDetail.isPresent()) {
                TextView hoursOfOperationView = Mockito.mock(TextView.class);
                mClassUnderTest.setHoursOfOperation(
                        hoursOfOperationView, localSearchDetail.get().getHoursOfOperation());

                ArgumentCaptor<String> argumentCaptor = ArgumentCaptor.forClass(String.class);
                Mockito.verify(hoursOfOperationView, times(1)).setText(argumentCaptor.capture());
                Assert.assertEquals("Today: 5 AM - 5 PM", argumentCaptor.getValue());
            }
        }
    }

    @Test
    public void clearPOITest() {
        Intent startNavigationIntent = generateIntent("aacs/ClearTemplate.json", "com.amazon.aacs.aasb.ClearTemplate");

        Optional<AACSMessage> aacsMessage = AACSMessageBuilder.parseEmbeddedIntent(startNavigationIntent);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            mClassUnderTest.clearTemplate();
            Mockito.verify(mMockSessionController, times(1)).clearTemplate();
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
