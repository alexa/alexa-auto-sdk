package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import android.graphics.Bitmap;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.lifecycle.MutableLiveData;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.util.QRCodeGenerator;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class CBLFragmentTest {
    @Mock
    CBLViewModel mMockCBLViewModel;
    @Mock
    QRCodeGenerator mMockQRCodeGenerator;
    @Mock
    Bitmap mMockQRCodeBitmap;
    @Mock
    NavController mMockNavController;

    MutableLiveData<AuthWorkflowData> mAuthWorkflowData = new MutableLiveData<>();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        when(mMockCBLViewModel.loginWorkflowState()).thenReturn(mAuthWorkflowData);
        when(mMockQRCodeGenerator.generateQRCode(anyString())).thenReturn(mMockQRCodeBitmap);
    }

    @Test
    public void testFragmentCanStartWithCBLCodeView() {
        // Test that when fragment is started after configuration change and cbl
        // workflow state is already "code pair received", then fragment displays
        // cbl code view.

        AuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);
        FragmentScenario<CBLFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLFragment.class, null, new CBLFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnCodePairReceivedCodePairIsDisplayed() {
        FragmentScenario<CBLFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLFragment.class, null, new CBLFragmentFactory());

        AuthWorkflowData codePairReceived = buildWithCodePairReceived();
        mAuthWorkflowData.setValue(codePairReceived);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.GONE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());

            assertEquals(codePairReceived.getCodePair().getValidationCode(),
                    ((TextView) view.findViewById(R.id.cbl_code)).getText());
        });
    }

    @Test
    public void testOnEndOfCBLLoginWorkflowFinishViewIsDisplayed() {
        FragmentScenario<CBLFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLFragment.class, null, new CBLFragmentFactory());

        AuthWorkflowData loginFinished = new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null);
        mAuthWorkflowData.setValue(loginFinished);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.GONE, view.findViewById(R.id.login_display_cbl_code_layout).getVisibility());
            assertEquals(View.VISIBLE, view.findViewById(R.id.cbl_login_finished_layout).getVisibility());
        });
    }

    @Test
    public void testOnLoginErrorMessageViewIsDisplayed() {
        FragmentScenario<CBLFragment> fragmentScenario =
                FragmentScenario.launchInContainer(CBLFragment.class, null, new CBLFragmentFactory());

        AuthWorkflowData startFailed = new AuthWorkflowData(AuthState.CBL_Auth_Start_Failed, null, null);
        mAuthWorkflowData.setValue(startFailed);

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
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
    class CBLFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            CBLFragmentOverride fragment = new CBLFragmentOverride(mMockCBLViewModel, mMockQRCodeGenerator);
            fragment.mNavController = mMockNavController;

            return fragment;
        }
    }

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class CBLFragmentOverride extends CBLFragment {
        NavController mNavController;
        /**
         * Constructs an instance of CBLFragment.
         *
         * @param viewModel View Model for CBL.
         * @param generator QR Code Generator.
         */
        CBLFragmentOverride(@NonNull CBLViewModel viewModel, @NonNull QRCodeGenerator generator) {
            this.mViewModel = viewModel;
            this.mQRCodeGenerator = generator;
        }

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }
    }
}
