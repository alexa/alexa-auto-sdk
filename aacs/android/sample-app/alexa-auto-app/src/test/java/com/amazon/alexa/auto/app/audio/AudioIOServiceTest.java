package com.amazon.alexa.auto.app.audio;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.when;

import android.app.Service;
import android.content.Intent;
import android.os.Build;

import com.amazon.aacsipc.AACSReceiver;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.app.AutoApplication;
import com.amazon.alexa.auto.app.dependencies.AppComponent;
import com.amazon.alexa.auto.app.dependencies.AudioIOComponent;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.android.controller.ServiceController;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowService;

import io.reactivex.rxjava3.subjects.PublishSubject;

@RunWith(RobolectricTestRunner.class)
@Config(sdk = Build.VERSION_CODES.P)
public class AudioIOServiceTest {
    private ServiceController<AudioIOService> mServiceController;

    @Mock
    AutoApplication mMockAlexaApp;
    @Mock
    AppComponent mMockAppComponent;
    @Mock
    AudioIOComponent mMockAudioIOComponent;
    @Mock
    AudioInputHandler mMockAudioInputHandler;
    @Mock
    AACSReceiver.Builder mMockReceiverBuilder;
    @Mock
    AACSReceiver mMockReceiver;

    private ShadowService mShadowService;
    private PublishSubject<Integer> mWorkerStateStream = PublishSubject.create();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockReceiverBuilder.withFetchCallback(any(AACSReceiver.FetchStreamCallback.class)))
                .thenReturn(mMockReceiverBuilder);
        when(mMockReceiverBuilder.build()).thenReturn(mMockReceiver);
        when(mMockAudioInputHandler.getWorkerState()).thenReturn(mWorkerStateStream);

        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(any(Service.class))).thenReturn(mMockAlexaApp);
            when(mMockAlexaApp.getAppComponent()).thenReturn(mMockAppComponent);
            when(mMockAppComponent.createAudioIOComponent()).thenReturn(mMockAudioIOComponent);

            doAnswer(invocation -> {
                AudioIOService injectTo = invocation.getArgument(0);
                injectTo.mAudioInputHandler = mMockAudioInputHandler;
                injectTo.mAACSReceiverBuilder = mMockReceiverBuilder;
                return null;
            })
                    .when(mMockAudioIOComponent)
                    .inject(any(AudioIOService.class));

            mServiceController = Robolectric.buildService(AudioIOService.class);

            mShadowService = Shadows.shadowOf(mServiceController.get());

            mServiceController.create();
        }
    }

    @Test
    public void testServiceStartedOnReceivingIntent() {
        Intent intent = new Intent();
        mServiceController.get().onStartCommand(intent, 0, 0);

        assertNotNull(mShadowService.getLastForegroundNotification());
    }

    @Test
    public void testServiceSelfStopsAfterWorkerIsIdled() {
        Intent intent = new Intent();
        mServiceController.get().onStartCommand(intent, 0, 0);

        assertFalse(mShadowService.isStoppedBySelf());

        // Make the worker idle.
        mWorkerStateStream.onNext(AudioIOServiceWorker.IDLE);

        // Verify that service is stopped.
        assertTrue(mShadowService.isStoppedBySelf());
    }
}
