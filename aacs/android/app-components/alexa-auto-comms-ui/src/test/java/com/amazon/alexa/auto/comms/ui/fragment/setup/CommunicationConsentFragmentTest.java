package com.amazon.alexa.auto.comms.ui.fragment.setup;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;

import com.amazon.alexa.auto.comms.ui.R;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class CommunicationConsentFragmentTest {
    @Mock
    CommunicationConsentViewModel mMockCommunicationConsentViewModel;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void testFragmentStartsWithCommunicationConsentView() {
        FragmentScenario<CommunicationConsentFragment> fragmentScenario = FragmentScenario.launchInContainer(
                CommunicationConsentFragment.class, null, new CommunicationsFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.communication_layout).getVisibility());
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class CommunicationsFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            return new CommunicationConsentFragment(mMockCommunicationConsentViewModel);
        }
    }
}
