package com.amazon.alexa.auto.settings.home;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.navigation.NavController;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.apis.BuildConfig;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.junit.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class DebugSettingsScreenBuilderTest {
    // Overriding class under test to mock statics.
    class DebugSettingsScreenBuilderTestOverride extends DebugSettingsScreenBuilder {
        @Override
        NavController findNavController(@NonNull View view) {
            return mMockNavController;
        }
    }

    @Mock
    NavController mMockNavController;
    @Mock
    PreferenceScreen mMockPrefScreen;
    @Mock
    Preference mMockPrefAACS;
    @Mock
    View mMockView;

    private DebugSettingsScreenBuilderTestOverride mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_AACS)).thenReturn(mMockPrefAACS);

        mClassUnderTest = new DebugSettingsScreenBuilderTestOverride();
    }

    @Test
    public void testDebugPreferencesAreNotRemovedForDebugBuild() {
        mClassUnderTest.addRemovePreferences(mMockPrefScreen);

        if (BuildConfig.DEBUG) {
            Mockito.verify(mMockPrefScreen, Mockito.times(0)).removePreference(mMockPrefAACS);
        }
    }

    @Test
    public void testDebugPreferencesAreRemovedForReleaseBuild() {
        mClassUnderTest.addRemovePreferences(mMockPrefScreen);

        if (!BuildConfig.DEBUG) {
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAACS);
        }
    }

    @Test
    public void testAACSPreference() {
        // AACS Preference is debug only preference and hence it is tested only for debug builds.
        if (BuildConfig.DEBUG) {
            mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

            ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                    ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
            Mockito.verify(mMockPrefAACS, Mockito.times(1)).setOnPreferenceClickListener(clickListenerCaptor.capture());

            // Invoke AACS preference by clicking on it.
            Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockPrefAACS));
            Mockito.verify(mMockNavController, Mockito.times(1)).navigate(R.id.navigation_fragment_aacs_preferences);
        }
    }
}
