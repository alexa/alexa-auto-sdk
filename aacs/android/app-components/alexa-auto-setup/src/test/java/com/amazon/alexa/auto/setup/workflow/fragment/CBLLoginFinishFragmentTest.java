package com.amazon.alexa.auto.setup.workflow.fragment;

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

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class CBLLoginFinishFragmentTest {
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    LoginViewModel mMockLoginViewModel;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockLoginViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
    }

    @Test
    public void testFragmentSetupFinishView() {
        FragmentScenario<CBLLoginFinishFragment> fragmentScenario = FragmentScenario.launchInContainer(
                CBLLoginFinishFragment.class, null, new CBLLoginFinishFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testLoginFinishIsNotifiedToViewModel() {
        FragmentScenario<CBLLoginFinishFragment> fragmentScenario = FragmentScenario.launchInContainer(
                CBLLoginFinishFragment.class, null, new CBLLoginFinishFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            verify(mMockLoginViewModel, times(1)).setupCompleted();

            TextView finishView = view.findViewById(R.id.cbl_login_finished_btn);
            finishView.performClick();

            verify(mMockLoginViewModel, times(1)).userFinishedLogin();
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class CBLLoginFinishFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);

                return new CBLLoginFinishFragment(mMockLoginViewModel, mMockApplication);
            }
        }
    }
}
