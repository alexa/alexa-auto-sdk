package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;

import com.amazon.alexa.auto.setup.R;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class NetworkFragmentTest {
    @Mock
    NetworkViewModel mMockNetworkViewModel;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void testFragmentStartsWithNetworkSettingsView() {
        FragmentScenario<NetworkFragment> fragmentScenario =
                FragmentScenario.launchInContainer(NetworkFragment.class, null, new NetworkFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();

            assertNotNull(view);
            assertEquals(View.VISIBLE, view.findViewById(R.id.network_layout).getVisibility());
        });
    }

    @Test
    public void testGoToAndroidNetworkSettingsScreen() {
        FragmentScenario<NetworkFragment> fragmentScenario =
                FragmentScenario.launchInContainer(NetworkFragment.class, null, new NetworkFragmentFactory());

        fragmentScenario.onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView finishView = view.findViewById(R.id.network_settings_btn);
            finishView.performClick();

            verify(mMockNetworkViewModel, times(1)).goToNetworkSettings();
        });
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class NetworkFragmentFactory extends FragmentFactory {
        @NonNull
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            return new NetworkFragment(mMockNetworkViewModel);
        }
    }
}
