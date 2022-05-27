package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.PREVIEW_MODE_ENABLED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.lifecycle.MutableLiveData;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
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

@RunWith(RobolectricTestRunner.class)
public class EnablePreviewModeFragmentTest {
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    EnablePreviewModeViewModel mMockViewModel;
    @Captor
    ArgumentCaptor<WorkflowMessage> workflowMessageArgumentCaptor;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.openMocks(this);

        when(mMockViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnFragmentStart_enablePreviewModeScreenIsDisplayed() {
        FragmentScenario<EnablePreviewModeFragment> fragmentScenario = FragmentScenario.launchInContainer(
                EnablePreviewModeFragment.class, null, new EnablePreviewModeFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.enable_preview_mode).getVisibility());

            assertEquals(View.VISIBLE, view.findViewById(R.id.enable_preview_mode_action_button).getVisibility());
        });
    }

    @Test
    public void testOnAuthStart_showsSpinnerAndHidesButton() {
        FragmentScenario<EnablePreviewModeFragment> fragmentScenario = FragmentScenario.launchInContainer(
                EnablePreviewModeFragment.class, null, new EnablePreviewModeFragmentFactory());

        AuthWorkflowData authStarted = new AuthWorkflowData(AuthState.Auth_Provider_Auth_Started, null, null);
        mAuthWorkflowData.setValue(authStarted);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            assertEquals(View.GONE, view.findViewById(R.id.enable_preview_mode_action_button).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.enable_preview_mode_progress_spinner).getVisibility());
        });
    }

    @Test
    public void testOnAuthSuccess_postsPreviewModeEnabledEvent() {
        FragmentScenario<EnablePreviewModeFragment> fragmentScenario = FragmentScenario.launchInContainer(
                EnablePreviewModeFragment.class, null, new EnablePreviewModeFragmentFactory());

        AuthWorkflowData authWorkflowData = new AuthWorkflowData(AuthState.Auth_Provider_Authorized, null, null);
        mAuthWorkflowData.setValue(authWorkflowData);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), PREVIEW_MODE_ENABLED);
        });
    }

    @Test
    public void testOnAuthError_postsErrorEvent() {
        FragmentScenario<EnablePreviewModeFragment> fragmentScenario = FragmentScenario.launchInContainer(
                EnablePreviewModeFragment.class, null, new EnablePreviewModeFragmentFactory());

        AuthWorkflowData authWorkflowData =
                new AuthWorkflowData(AuthState.Auth_Provider_Authorization_Error, null, null);
        mAuthWorkflowData.setValue(authWorkflowData);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), SETUP_ERROR);
        });
    }

    @Test
    public void testOnEnablePreviewModeClick_AuthIsStarted() {
        FragmentScenario<EnablePreviewModeFragment> fragmentScenario = FragmentScenario.launchInContainer(
                EnablePreviewModeFragment.class, null, new EnablePreviewModeFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView enablePreviewModeButton = view.findViewById(R.id.enable_preview_mode_action_button);
            enablePreviewModeButton.performClick();

            verify(mMockViewModel, times(1)).enablePreviewMode();
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class EnablePreviewModeFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
                EnablePreviewModeFragment fragment = new EnablePreviewModeFragment(mMockViewModel);
                return fragment;
            }
        }
    }
}
