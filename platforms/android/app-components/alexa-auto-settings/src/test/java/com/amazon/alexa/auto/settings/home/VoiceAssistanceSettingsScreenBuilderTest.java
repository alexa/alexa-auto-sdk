package com.amazon.alexa.auto.settings.home;

import android.content.Context;
import android.view.View;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

public class VoiceAssistanceSettingsScreenBuilderTest {
    @Mock
    Context mMockContext;
    @Mock
    PreferenceScreen mMockPrefScreen;
    @Mock
    Preference mMockPrefVoiceAssistanceNonAlexa;
    @Mock
    Preference mMockPrefVoiceAssistanceAlexa;
    @Mock
    View mMockView;

    private VoiceAssistanceSettingsScreenBuilder mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA))
                .thenReturn(mMockPrefVoiceAssistanceNonAlexa);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA))
                .thenReturn(mMockPrefVoiceAssistanceAlexa);
        Mockito.when(mMockPrefScreen.getContext()).thenReturn(mMockContext);

        mClassUnderTest = new VoiceAssistanceSettingsScreenBuilder();
    }

    @Test
    public void testPreferencesAreRemovedForAlexaOnlyMode() {
        mClassUnderTest.addRemovePreferences(mMockPrefScreen);

        Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceNonAlexa);
        Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceAlexa);
    }
}
