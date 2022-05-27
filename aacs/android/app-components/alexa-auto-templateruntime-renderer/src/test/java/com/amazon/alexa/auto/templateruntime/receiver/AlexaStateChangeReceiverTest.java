package com.amazon.alexa.auto.templateruntime.receiver;

import static com.amazon.aacsconstants.TemplateRuntimeConstants.TEMPLATE_TYPE_WEATHER;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.apps.common.Constants;
import com.amazon.alexa.auto.templateruntime.R;
import com.amazon.alexa.auto.templateruntime.common.TestResourceFileReader;
import com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.reflect.Field;
import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class AlexaStateChangeReceiverTest {
    private AlexaStateChangeReceiver mClassUnderTest;
    private Intent mIntent;
    @Mock
    private AlexaApp mMockAlexaApp;
    @Mock
    private AlexaAppRootComponent mMockRootComponent;
    @Mock
    private SessionViewController mMockSessionController;
    @Mock
    private ViewGroup mViewGroup;
    @Mock
    private View mView;

    @Before
    public void setup() throws Exception {
        mClassUnderTest = new AlexaStateChangeReceiver();
        MockitoAnnotations.openMocks(this);
        when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
        when(mMockRootComponent.getComponent(SessionViewController.class))
                .thenReturn(Optional.of(mMockSessionController));
        when(mMockSessionController.getTemplateRuntimeViewContainer()).thenReturn(Optional.of(mViewGroup));

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnDialogStateIdleAndWeatherTemplateActive_publishesEvent() {
        generateIntent("aacs/DialogStateChangedIdle.json");
        when(mViewGroup.findViewById(R.id.template_weather_view)).thenReturn(mView);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            mClassUnderTest.onReceive(Mockito.mock(Context.class), mIntent);
            ArgumentCaptor<AlexaVoiceoverCompletedMessage> messageArgumentCaptor =
                    ArgumentCaptor.forClass(AlexaVoiceoverCompletedMessage.class);
            verify(EventBus.getDefault()).post(messageArgumentCaptor.capture());
            assertNotNull(messageArgumentCaptor.getValue().getCompletedAt());
        }
    }

    @Test
    public void testOnDialogStateNotIdle_doesNotPublishEvent() {
        generateIntent("aacs/DialogStateChangedListening.json");
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            mClassUnderTest.onReceive(Mockito.mock(Context.class), mIntent);
            verify(EventBus.getDefault(), never()).post(any());
        }
    }

    @Test
    public void testOnDialogStateIdleAndWeatherTemplateNotActive_doesNotPublishEvent() {
        generateIntent("aacs/DialogStateChangedIdle.json");
        when(mViewGroup.findViewById(123)).thenReturn(mView);
        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Context.class))).thenReturn(mMockAlexaApp);
            mClassUnderTest.onReceive(Mockito.mock(Context.class), mIntent);
            verify(EventBus.getDefault(), never()).post(any());
        }
    }

    private void generateIntent(String resPath) {
        mIntent = new Intent(Action.AlexaClient.DIALOG_STATE_CHANGED);
        Optional<String> sampleAACSListeningMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSListeningMessage.get());
        mIntent.putExtra("payload", sampleListeningPayload);
    }
}
