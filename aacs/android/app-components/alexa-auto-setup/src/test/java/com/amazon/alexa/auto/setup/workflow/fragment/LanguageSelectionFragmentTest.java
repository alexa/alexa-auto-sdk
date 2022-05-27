package com.amazon.alexa.auto.setup.workflow.fragment;

import static org.junit.Assert.assertNotNull;

import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.R;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class LanguageSelectionFragmentTest {
    @Mock
    AlexaPropertyManager mMockPropManager;
    @Mock
    AlexaLocalesProvider mMockLocalesProvider;
    @Mock
    NavController mMockNavController;

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class LanguageSelectionFragmentOverride extends LanguageSelectionFragment {
        NavController mNavController;

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }
    }

    /**
     * Fragment factory to inject mock dependencies.
     */
    public class LanguageSelectionFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            LanguageSelectionFragmentOverride fragment = new LanguageSelectionFragmentOverride();
            fragment.mNavController = mMockNavController;
            fragment.mLocalesProvider = mMockLocalesProvider;

            return fragment;
        }
    }

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void testLanguageSettingsViewIsNavigated() {
        launchFragment().onFragment(fragment -> {
            View view = fragment.getView();
            assertNotNull(view);

            TextView finishView = view.findViewById(R.id.select_language_action_button);
            finishView.performClick();
            Mockito.verify(mMockNavController, Mockito.times(1))
                    .navigate(R.id.navigation_fragment_languageSelection, fragment.mShowContinueButton);
        });
    }

    private FragmentScenario<LanguageSelectionFragmentOverride> launchFragment() {
        return FragmentScenario.launchInContainer(LanguageSelectionFragmentOverride.class, null,
                com.amazon.alexa.auto.apps.common.ui.R.style.Theme_Alexa_Standard,
                new LanguageSelectionFragmentFactory());
    }
}
