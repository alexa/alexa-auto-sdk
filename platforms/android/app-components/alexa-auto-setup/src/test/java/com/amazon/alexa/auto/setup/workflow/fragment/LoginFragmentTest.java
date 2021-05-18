package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;
import android.graphics.Bitmap;
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
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.util.QRCodeGenerator;

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
public class LoginFragmentTest {
    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    LoginViewModel mMockLoginViewModel;
    @Mock
    QRCodeGenerator mMockQRCodeGenerator;
    @Mock
    Bitmap mMockQRCodeBitmap;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockLoginViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
        when(mMockQRCodeGenerator.generateQRCode(anyString())).thenReturn(mMockQRCodeBitmap);
    }

    @Test
    public void testFragmentStartsWithLoginViewByDefault() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testFragmentCanStartWithCBLCodeView() {
        // Test that when fragment is started after configuration change and cbl
        // workflow state is already "code pair received", then fragment displays
        // cbl code view.

        AuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnCBLLoginWorkflowStartWaitCursorIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData workflowStarted = new AuthWorkflowData(AuthState.CBL_Auth_Started, null, null);
        mAuthWorkflowData.setValue(workflowStarted);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_progress_spinner).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnCodePairReceivedCodePairIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_progress_spinner).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());

            assertEquals(codePairReceived.getCodePair().getValidationCode(),
                    ((TextView) view.findViewById(R.id.cbl_code)).getText());
        });
    }

    @Test
    public void testOnEndOfCBLLoginWorkflowFinishViewIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnLoginErrorMessageViewIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData startFailed = new AuthWorkflowData(AuthState.CBL_Auth_Start_Failed, null, null);
        mAuthWorkflowData.setValue(startFailed);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_error_text_view).getVisibility());

            TextView loginStartBtnText = (TextView) view.findViewById(R.id.get_started_action_button);
            assertEquals("Try Again", loginStartBtnText.getText().toString());
        });
    }

    @Test
    public void testLoginIsStartedOnGetStarted() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView getStartedView = view.findViewById(R.id.get_started_action_button);
            getStartedView.performClick();

            verify(mMockLoginViewModel, times(1)).startLogin();
        });
    }

    @Test
    public void testFragmentPerformSkipCBLLogin() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData authProviderAuthorizing =
                new AuthWorkflowData(AuthState.Auth_Provider_Authorizing, null, null);
        mAuthWorkflowData.setValue(authProviderAuthorizing);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            fragment.updateUIForPreviewMode();

            assertEquals(View.VISIBLE, view.findViewById(R.id.skip_cbl_action_button).getVisibility());

            TextView skipButton = view.findViewById(R.id.skip_cbl_action_button);
            skipButton.performClick();

            verify(mMockLoginViewModel, times(1)).signInLater();
        });
    }

    @Test
    public void testOnEndOfAuthProviderLoginWorkflowFinishViewIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.Auth_Provider_Authorized, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.auth_provider_login_finished_layout).getVisibility());
        });
    }

    /**
     * Build the {@link AuthWorkflowData} with state that CBL code pair
     * is received.
     *
     * @return Instance of {@link AuthWorkflowData}.
     */
    private AuthWorkflowData buildWithCodePairReceived() {
        return new AuthWorkflowData(
                AuthState.CBL_Auth_CodePair_Received, new CodePair("code-pair-url", "code-pair-code"), null);
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class LoginFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);

                return new LoginFragment(mMockLoginViewModel, mMockQRCodeGenerator, mMockApplication);
            }
        }
    }
}
