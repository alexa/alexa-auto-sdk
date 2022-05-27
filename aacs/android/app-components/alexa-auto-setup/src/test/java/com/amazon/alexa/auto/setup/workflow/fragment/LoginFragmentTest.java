package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.CBL_AUTH_FINISHED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;
import android.content.Context;
import android.graphics.Bitmap;
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
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.util.QRCodeGenerator;

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
import org.robolectric.shadows.ShadowLooper;

import java.lang.reflect.Field;
import java.util.Optional;
import java.util.concurrent.TimeUnit;

@RunWith(RobolectricTestRunner.class)
public class LoginFragmentTest {
    private boolean mPreviewModeEnabled = true;
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
    @Mock
    NavController mMockNavController;
    @Captor
    ArgumentCaptor<WorkflowMessage> workflowMessageArgumentCaptor;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.openMocks(this);

        when(mMockLoginViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
        when(mMockQRCodeGenerator.generateQRCode(anyString())).thenReturn(mMockQRCodeBitmap);

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnFragmentStart_LoginScreenIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());

            assertEquals(View.VISIBLE, view.findViewById(R.id.sign_in_action_button).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.try_alexa_action_button).getVisibility());
        });
    }

    @Test
    public void testWhenPreviewModeIsDisabled_LoginScreenDoesNotContainTryAlexaButton() {
        mPreviewModeEnabled = false;
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());

            assertEquals(View.VISIBLE, view.findViewById(R.id.sign_in_action_button).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.try_alexa_action_button).getVisibility());
        });
    }

    @Test
    public void testOnCodePairReceived_FragmentCanStartWithCBLCodeView() {
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
        });
    }

    @Test
    public void testOnCBLLoginWorkflowStart_WaitCursorIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData workflowStarted = new AuthWorkflowData(AuthState.CBL_Auth_Started, null, null);
        mAuthWorkflowData.setValue(workflowStarted);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());

            assertEquals(View.VISIBLE, view.findViewById(R.id.login_progress_spinner).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.sign_in_action_button).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.try_alexa_action_button).getVisibility());
        });
    }

    @Test
    public void testOnCodePairReceived_CodePairIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());

            assertEquals(codePairReceived.getCodePair().getValidationCode(),
                    ((TextView) view.findViewById(R.id.cbl_code)).getText());
            assertEquals(View.GONE, view.findViewById(R.id.login_progress_spinner).getVisibility());
        });
    }

    @Test
    public void testOnEndOfCBLLoginWorkflow_FinishViewIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), CBL_AUTH_FINISHED);
        });
    }

    @Test
    public void testOnLoginError_ErrorEventIsTriggered() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        AuthWorkflowData startFailed = new AuthWorkflowData(AuthState.CBL_Auth_Start_Failed, null, null);
        mAuthWorkflowData.setValue(startFailed);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), SETUP_ERROR);
        });
    }

    @Test
    public void testOnSignInClick_LoginIsStarted() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView signInView = view.findViewById(R.id.sign_in_action_button);
            signInView.performClick();

            verify(mMockLoginViewModel, times(1)).startLogin();
        });
    }

    @Test
    public void testOnTryAlexaClicked_PreviewModeScreenIsDisplayed() {
        FragmentScenario<LoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(LoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView getStartedView = view.findViewById(R.id.try_alexa_action_button);
            getStartedView.performClick();
            ShadowLooper.idleMainLooper(2000, TimeUnit.MILLISECONDS);
            verify(mMockNavController, times(1)).navigate(R.id.navigation_fragment_enablePreviewMode);
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
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class LoginFragmentOverride extends LoginFragment {
        NavController mNavController;
        boolean mPreviewModeEnabled;

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }

        @Override
        boolean isPreviewModeEnabled(@NonNull Context context) {
            return mPreviewModeEnabled;
        }
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class LoginFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
                staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
                when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
                LoginFragmentOverride fragment = new LoginFragmentOverride();
                fragment.mNavController = mMockNavController;
                fragment.mViewModel = mMockLoginViewModel;
                fragment.mQRCodeGenerator = mMockQRCodeGenerator;
                fragment.mApp = mMockAlexaApp;
                fragment.mPreviewModeEnabled = mPreviewModeEnabled;
                return fragment;
            }
        }
    }
}
