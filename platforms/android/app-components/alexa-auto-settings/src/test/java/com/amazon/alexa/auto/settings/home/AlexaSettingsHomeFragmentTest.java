package com.amazon.alexa.auto.settings.home;

import static com.amazon.aacsconstants.AACSPropertyConstants.LOCALE;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_SUPPORTED;

import android.content.Context;
import android.util.Pair;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;
import androidx.fragment.app.testing.FragmentScenario;
import androidx.lifecycle.Lifecycle;
import androidx.navigation.NavController;
import androidx.preference.Preference;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import io.reactivex.rxjava3.core.Single;

@RunWith(RobolectricTestRunner.class)
public class AlexaSettingsHomeFragmentTest {
    @Mock
    AlexaPropertyManager mMockPropManager;
    @Mock
    LocalesProvider mMockLocalesProvider;
    @Mock
    NavController mMockNavController;
    @Mock
    AuthController mMockAuthController;

    // Screen builders
    @Mock
    AlexaSettingsScreenBuilder mMockSettingsScreenBuilder1;
    List<AlexaSettingsScreenBuilder> mScreenBuilders = new ArrayList<>();

    /**
     * Override class under test to supply dependencies that are obtained using
     * static methods.
     */
    public static class AlexaSettingsHomeFragmentOverride extends AlexaSettingsHomeFragment {
        NavController mNavController;
        AuthController mAuthController;

        @Override
        NavController findNavController(@NonNull View view) {
            return mNavController;
        }

        @Override
        AuthController fetchAuthController(@NonNull Context context) {
            return mAuthController;
        }
    }

    /**
     * Fragment factory to inject mock dependencies.
     */
    public class AlexaSettingsHomeFragmentFactory extends FragmentFactory {
        @NonNull
        @Override
        public Fragment instantiate(@NonNull ClassLoader classLoader, @NonNull String className) {
            AlexaSettingsHomeFragmentOverride fragment = new AlexaSettingsHomeFragmentOverride();
            fragment.mNavController = mMockNavController;
            fragment.mAlexaPropertyManager = mMockPropManager;
            fragment.mLocalesProvider = mMockLocalesProvider;
            fragment.mScreenBuilders = mScreenBuilders;
            fragment.mAuthController = mMockAuthController;

            return fragment;
        }
    }

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        mScreenBuilders.add(mMockSettingsScreenBuilder1);
        Mockito.when(mMockAuthController.getAuthMode()).thenReturn(AuthMode.CBL_AUTHORIZATION);
    }

    @Test
    public void testScreenBuilderLifecycle() {
        setupPropMocks(false, false, null);

        FragmentScenario<AlexaSettingsHomeFragmentOverride> fragmentScenario = launchFragment();

        fragmentScenario.onFragment(fragment -> {
            Mockito.verify(mMockSettingsScreenBuilder1, Mockito.times(1)).addRemovePreferences(Mockito.any());
            Mockito.verify(mMockSettingsScreenBuilder1, Mockito.times(1))
                    .installEventHandlers(Mockito.any(), Mockito.any());
            Mockito.verify(mMockSettingsScreenBuilder1, Mockito.times(0)).dispose();

            fragmentScenario.moveToState(Lifecycle.State.DESTROYED);
            Mockito.verify(mMockSettingsScreenBuilder1, Mockito.times(1)).dispose();
        });
    }

    @Test
    public void testOnWakeWordNotSupportedTheHandsFreePreferenceIsTakenOut() {
        setupPropMocks(false /*wake word not supported*/, false, null);

        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_HANDS_FREE);
            Assert.assertNull(preference);
        });
    }

    @Test
    public void testWakewordEnableDisable() {
        setupPropMocks(true, true, null);

        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_HANDS_FREE);
            Assert.assertNotNull(preference);

            preference.performClick();
            Mockito.verify(mMockPropManager, Mockito.times(1)).updateAlexaProperty(WAKEWORD_ENABLED, "false");

            preference.performClick();
            Mockito.verify(mMockPropManager, Mockito.times(1)).updateAlexaProperty(WAKEWORD_ENABLED, "true");
        });
    }

    @Test
    public void testThatLanguageSummaryIsSetToAlexaLanguage() {
        String localeId = "test-locale-id";
        Pair<String, String> languageCountryPair = new Pair<>("test-lang", "test-country");
        setupPropMocks(true, true, localeId);
        Mockito.when(mMockLocalesProvider.fetchAlexaSupportedLocaleWithId(localeId))
                .thenReturn(Single.just(Optional.of(languageCountryPair)));

        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES);
            Assert.assertNotNull(preference);

            Assert.assertEquals(String.format("%s (%s)", languageCountryPair.first, languageCountryPair.second),
                    preference.getSummary());
        });
    }

    @Test
    public void testOnLanguagePrefClickLanguageSettingsPageIsNavigated() {
        setupPropMocks(true, true, null);

        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES);
            Assert.assertNotNull(preference);

            preference.performClick();
            Mockito.verify(mMockNavController, Mockito.times(1))
                    .navigate(R.id.navigation_fragment_alexa_settings_languages);
        });
    }

    @Test
    public void testOnCommunicationPreferencePresents() {
        setupPropMocks(false, false, null);

        launchFragment().onFragment(fragment -> {
            Preference preference =
                    fragment.getPreferenceScreen().findPreference(PreferenceKeys.ALEXA_SETTINGS_COMMUNICATION);
            Assert.assertNotNull(preference);
        });
    }

    private FragmentScenario<AlexaSettingsHomeFragmentOverride> launchFragment() {
        return FragmentScenario.launchInContainer(AlexaSettingsHomeFragmentOverride.class, null,
                com.amazon.alexa.auto.apps.common.ui.R.style.Theme_Alexa_Standard,
                new AlexaSettingsHomeFragmentFactory());
    }

    private void setupPropMocks(boolean wakewordSupported, boolean wakewordEnabled, @Nullable String locale) {
        Mockito.when(mMockPropManager.getAlexaPropertyBoolean(WAKEWORD_SUPPORTED))
                .thenReturn(Single.just(Optional.of(wakewordSupported)));
        Mockito.when(mMockPropManager.getAlexaPropertyBoolean(WAKEWORD_ENABLED))
                .thenReturn(Single.just(Optional.of(wakewordEnabled)));
        Mockito.when(mMockPropManager.getAlexaProperty(LOCALE))
                .thenReturn(Single.just(locale == null ? Optional.empty() : Optional.of(locale)));
        Mockito.when(mMockPropManager.updateAlexaProperty(Mockito.anyString(), Mockito.anyString()))
                .thenReturn(Single.just(true));
    }
}
