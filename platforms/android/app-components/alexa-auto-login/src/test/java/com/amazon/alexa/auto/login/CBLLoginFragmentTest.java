package com.amazon.alexa.auto.login;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.graphics.Bitmap;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.lifecycle.MutableLiveData;

import com.amazon.alexa.auto.apis.auth.CBLAuthState;
import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class CBLLoginFragmentTest {
    @Mock
    CBLLoginViewModel mMockLoginViewModel;
    @Mock
    QRCodeGenerator mMockQRCodeGenerator;
    @Mock
    Bitmap mMockQRCodeBitmap;

    MutableLiveData<CBLAuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockLoginViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
        when(mMockQRCodeGenerator.generateQRCode(anyString())).thenReturn(mMockQRCodeBitmap);
    }

    @Test
    public void testFragmentStartsWithLoginViewByDefault() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testFragmentCanStartWithCBLCodeView() {
        // Test that when fragment is started after configuration change and cbl
        // workflow state is already "code pair received", then fragment displays
        // cbl code view.

        CBLAuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnCBLLoginWorkflowStartWaitCursorIsDisplayed() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        CBLAuthWorkflowData workflowStarted = new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Started, null);
        mAuthWorkflowData.setValue(workflowStarted);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_progress_spinner).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnCodePairReceivedCodePairIsDisplayed() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        CBLAuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_progress_spinner).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_finished_layout).getVisibility());

            assertEquals(codePairReceived.getCodePair().getValidationCode(),
                    ((TextView) view.findViewById(R.id.cbl_code)).getText());
        });
    }

    @Test
    public void testOnEndOfLoginWorkflowFinishViewIsDisplayed() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        CBLAuthWorkflowData loginFinished = new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Finished, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_start_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testLoginIsStartedOnGetStarted() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView getStartedView = view.findViewById(R.id.get_started_action_button);
            getStartedView.performClick();

            verify(mMockLoginViewModel, times(1)).starLogin();
        });
    }

    @Test
    public void testLoginFinishIsNotifiedToViewModel() {
        FragmentScenario<CBLLoginFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLLoginFragment.class, null, new LoginFragmentFactory());

        CBLAuthWorkflowData loginFinished = new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Finished, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView finishView = view.findViewById(R.id.login_finished_btn);
            finishView.performClick();

            verify(mMockLoginViewModel, times(1)).userFinishedLogin();
        });
    }

    /**
     * Build the {@link CBLAuthWorkflowData} with state that CBL code pair
     * is received.
     *
     * @return Instance of {@link CBLAuthWorkflowData}.
     */
    private CBLAuthWorkflowData buildWithCodePairReceived() {
        return new CBLAuthWorkflowData(
                CBLAuthState.CBL_Auth_CodePair_Received, new CodePair("code-pair-url", "code-pair-code"));
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class LoginFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            return new CBLLoginFragment(mMockLoginViewModel, mMockQRCodeGenerator);
        }
    }
}
