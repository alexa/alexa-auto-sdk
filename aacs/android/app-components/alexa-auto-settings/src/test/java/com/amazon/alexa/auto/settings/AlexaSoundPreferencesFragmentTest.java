package com.amazon.alexa.auto.settings;

import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.EARCON_SETTINGS;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.EARCON_SETTINGS_END;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.EARCON_SETTINGS_START;

import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.preference.Preference;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class AlexaSoundPreferencesFragmentTest {
    @Mock
    private Context mMockContext;
    @Mock
    private SharedPreferences mMockSharedPrefs;
    @Mock
    private SharedPreferences.Editor mMockEditor;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        when(mMockContext.getSharedPreferences(EARCON_SETTINGS, 0)).thenReturn(mMockSharedPrefs);
        when(mMockSharedPrefs.edit()).thenReturn(mMockEditor);
    }

    @Test
    public void testOnLoad_invokesSharesPrefsAndSetsValueInView() {
        when(mMockSharedPrefs.getBoolean(EARCON_SETTINGS_START, true)).thenReturn(true);
        when(mMockSharedPrefs.getBoolean(EARCON_SETTINGS_END, true)).thenReturn(true);

        launchFragment().onFragment(fragment -> {
            SwitchPreferenceCompat soundStartPreference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_START);
            SwitchPreferenceCompat soundEndPreference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_END);
            Assert.assertNotNull(soundStartPreference);
            Assert.assertNotNull(soundEndPreference);

            verify(mMockSharedPrefs).getBoolean(EARCON_SETTINGS_START, true);
            verify(mMockSharedPrefs).getBoolean(EARCON_SETTINGS_END, true);
            Assert.assertTrue(soundStartPreference.isChecked());
            Assert.assertTrue(soundEndPreference.isChecked());
        });
    }

    @Test
    public void testOnEarconSettingDisableEnable_appropriateMethodIsInvoked() {
        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_START);
            Assert.assertNotNull(preference);

            preference.performClick();
            Mockito.verify(mMockEditor).putBoolean(EARCON_SETTINGS_START, true);

            preference.performClick();
            Mockito.verify(mMockEditor).putBoolean(EARCON_SETTINGS_START, false);

            preference = fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_END);
            Assert.assertNotNull(preference);

            preference.performClick();
            Mockito.verify(mMockEditor).putBoolean(EARCON_SETTINGS_END, true);

            preference.performClick();
            Mockito.verify(mMockEditor).putBoolean(EARCON_SETTINGS_END, false);
        });
    }

    private FragmentScenario<AlexaSoundPreferencesFragmentOverride> launchFragment() {
        return FragmentScenario.launchInContainer(AlexaSoundPreferencesFragmentOverride.class, null,
                com.amazon.alexa.auto.apps.common.ui.R.style.Theme_Alexa_Standard,
                new AlexaSoundPreferencesFragmentTest.AlexaSoundPreferencesFragmentFactory());
    }

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class AlexaSoundPreferencesFragmentOverride extends AlexaSoundPreferencesFragment {
        @Override
        void setDefaultPreferences() {
            // Do nothing
        }
    }

    /**
     * Fragment factory to inject mocks into Fragment.
     */
    class AlexaSoundPreferencesFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            AlexaSoundPreferencesFragmentOverride fragment = new AlexaSoundPreferencesFragmentOverride();
            fragment.mContext = mMockContext;
            return fragment;
        }
    }
}
