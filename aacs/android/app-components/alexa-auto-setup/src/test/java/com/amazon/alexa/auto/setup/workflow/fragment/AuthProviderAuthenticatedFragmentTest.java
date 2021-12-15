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
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.auth.AuthMode;
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
public class AuthProviderAuthenticatedFragmentTest {
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    LoginViewModel mMockLoginViewModel;
    @Mock
    NavController mMockNavController;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockLoginViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
    }

    @Test
    public void testFragmentSetupFinishView() {
        FragmentScenario<AuthProviderAuthenticatedFragment> fragmentScenario = FragmentScenario.launchInContainer(
                AuthProviderAuthenticatedFragment.class, null, new AuthProviderLoginFinishFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.auth_provider_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testLoginFinishIsNotifiedToViewModel() {
        FragmentScenario<AuthProviderAuthenticatedFragment> fragmentScenario = FragmentScenario.launchInContainer(
                AuthProviderAuthenticatedFragment.class, null, new AuthProviderLoginFinishFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.Auth_Provider_Authorized, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            verify(mMockLoginViewModel, times(1)).setupCompleted();

            TextView finishView = view.findViewById(R.id.auth_provider_login_finished_btn);
            finishView.performClick();

            verify(mMockLoginViewModel, times(1)).userFinishedLogin();
        });
    }

    @Test
    public void testSignInIsNotifiedToViewModel() {
        FragmentScenario<AuthProviderAuthenticatedFragment> fragmentScenario = FragmentScenario.launchInContainer(
                AuthProviderAuthenticatedFragment.class, null, new AuthProviderLoginFinishFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView finishView = view.findViewById(R.id.sign_in_action_button);
            finishView.performClick();

            verify(mMockLoginViewModel, times(1)).userFinishedLogin();
            verify(mMockLoginViewModel, times(1)).userSwitchedLogin(AuthMode.CBL_AUTHORIZATION);
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class AuthProviderLoginFinishFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);

                AuthProviderAuthenticatedFragmentOverride fragment =
                        new AuthProviderAuthenticatedFragmentOverride(mMockLoginViewModel, mMockApplication);
                fragment.mNavController = mMockNavController;

                return fragment;
            }
        }
    }

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class AuthProviderAuthenticatedFragmentOverride extends AuthProviderAuthenticatedFragment {
        NavController mNavController;

        AuthProviderAuthenticatedFragmentOverride(@NonNull LoginViewModel viewModel, @NonNull Application application) {
            this.mViewModel = viewModel;
            this.mApp = AlexaApp.from(application);
        }

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }
    }
}
