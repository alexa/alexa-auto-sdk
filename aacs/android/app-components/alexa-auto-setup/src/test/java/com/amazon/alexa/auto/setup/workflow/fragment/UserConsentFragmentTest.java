package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.aacsconstants.AACSPropertyConstants.GEOLOCATION_ENABLED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.LOCATION_CONSENT_COMPLETED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.DISABLED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.ENABLED;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.app.Application;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.reflect.Field;

import io.reactivex.rxjava3.core.Single;

@RunWith(RobolectricTestRunner.class)
public class UserConsentFragmentTest {
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaPropertyManager mMockAlexaPropertyManager;
    @Captor
    ArgumentCaptor<WorkflowMessage> workflowMessageArgumentCaptor;

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.openMocks(this);

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnFragmentLoad_inflatesLocationConsentView() {
        FragmentScenario<LocationConsentFragment> fragmentScenario = FragmentScenario.launchInContainer(
                LocationConsentFragment.class, null, new LocationConsentFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.location_consent).getVisibility());

            assertEquals(View.VISIBLE, view.findViewById(R.id.use_location_button_view).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.setup_skip_button_view).getVisibility());
        });
    }

    @Test
    public void testOnSkipClick_propertyManagerIsInvokedToDisableGeolocation() {
        Mockito.when(mMockAlexaPropertyManager.updateAlexaProperty(GEOLOCATION_ENABLED, DISABLED.getValue()))
                .thenReturn(Single.just(true));

        FragmentScenario<LocationConsentFragment> fragmentScenario = FragmentScenario.launchInContainer(
                LocationConsentFragment.class, null, new LocationConsentFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView skipButton = view.findViewById(R.id.setup_skip_button_view);
            skipButton.performClick();

            verify(mMockAlexaPropertyManager, times(1)).updateAlexaProperty(GEOLOCATION_ENABLED, DISABLED.getValue());
            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), LOCATION_CONSENT_COMPLETED);
        });
    }

    @Test
    public void testOnUseLocationClick_propertyManagerIsInvokedToDisableGeolocation() {
        Mockito.when(mMockAlexaPropertyManager.updateAlexaProperty(GEOLOCATION_ENABLED, ENABLED.getValue()))
                .thenReturn(Single.just(true));

        FragmentScenario<LocationConsentFragment> fragmentScenario = FragmentScenario.launchInContainer(
                LocationConsentFragment.class, null, new LocationConsentFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView skipButton = view.findViewById(R.id.use_location_button_view);
            skipButton.performClick();

            verify(mMockAlexaPropertyManager, times(1)).updateAlexaProperty(GEOLOCATION_ENABLED, ENABLED.getValue());
            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), LOCATION_CONSENT_COMPLETED);
        });
    }

    @Test
    public void testOnUseLocationFailed_SetupErrorEventIsTriggered() {
        Mockito.when(mMockAlexaPropertyManager.updateAlexaProperty(GEOLOCATION_ENABLED, ENABLED.getValue()))
                .thenReturn(Single.just(false));

        FragmentScenario<LocationConsentFragment> fragmentScenario = FragmentScenario.launchInContainer(
                LocationConsentFragment.class, null, new LocationConsentFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView skipButton = view.findViewById(R.id.use_location_button_view);
            skipButton.performClick();

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), SETUP_ERROR);
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class LocationConsentFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                LocationConsentFragment fragment = new LocationConsentFragment();
                fragment.mAlexaPropertyManager = mMockAlexaPropertyManager;
                return fragment;
            }
        }
    }
}
