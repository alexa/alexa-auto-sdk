package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
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
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class CBLLoginErrorFragmentTest {
    @Mock
    NavController mMockNavController;
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    AlexaSetupWorkflowControllerImpl alexaSetupWorkflowController;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void testOnFragmentLoad_inflatesCBLLoginErrorView() {
        FragmentScenario<CBLLoginErrorFragment> fragmentScenario = FragmentScenario.launchInContainer(
                CBLLoginErrorFragment.class, null, new CBLLoginErrorFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.cbl_login_error).getVisibility());
        });
    }

    @Test
    public void testOnClickTryAgain_resetsSetupWorkflow() {
        FragmentScenario<CBLLoginErrorFragment> fragmentScenario = FragmentScenario.launchInContainer(
                CBLLoginErrorFragment.class, null, new CBLLoginErrorFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            TextView tryAgainButton = view.findViewById(R.id.cbl_retry_button);
            tryAgainButton.performClick();

            verify(mMockRootComponent, times(1)).getComponent(AlexaSetupWorkflowController.class);
            verify(alexaSetupWorkflowController, times(1)).stopSetupWorkflow();
            verify(alexaSetupWorkflowController, times(1)).startSetupWorkflow(any(), eq(mMockNavController), eq(null));
        });
    }

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class CBLLoginErrorFragmentOverride extends CBLLoginErrorFragment {
        NavController mNavController;

        public CBLLoginErrorFragmentOverride(NavController navController, Application application) {
            super(navController, application);
            mNavController = navController;
        }

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class CBLLoginErrorFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
                when(mMockRootComponent.getComponent(AlexaSetupWorkflowController.class))
                        .thenReturn(Optional.of(alexaSetupWorkflowController));
                return new CBLLoginErrorFragmentOverride(mMockNavController, mMockApplication);
            }
        }
    }
}
