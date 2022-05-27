package com.amazon.alexa.auto.settings.home;

import static com.amazon.alexa.auto.apps.common.Constants.ALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.ALEXA_DISABLED;
import static com.amazon.alexa.auto.apps.common.Constants.BOTH_ENABLED;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.NON_ALEXA_DISABLED;

import static org.mockito.Mockito.when;

import android.app.Application;
import android.content.Context;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.navigation.NavController;
import androidx.navigation.NavGraph;
import androidx.navigation.NavInflater;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SettingsProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupController;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.Optional;

public class VoiceAssistanceSettingsScreenBuilderTest {
    // Overriding class under test to mock statics.
    class VoiceAssistanceSettingsScreenBuilderOverride extends VoiceAssistanceSettingsScreenBuilder {
        @Override
        AlexaApp fetchAlexaApp(@NonNull Context context) {
            return mMockAlexaApp;
        }

        @Override
        AuthController fetchAuthController(@NonNull Context context) {
            return mMockAuthController;
        }

        @Override
        SetupController fetchVoiceAssistanceSetupController(@NonNull Context context) {
            return mMockSetupController;
        }

        @Override
        SettingsProvider fetchSettingsProvider(@NonNull Context context) {
            return mMockSettingsProvider;
        }

        @Override
        AssistantManager fetchAssistantManager(@NonNull Context context) {
            return mMockAssistantManager;
        }

        @Override
        NavController findNavController(@NonNull View view) {
            return mMockNavController;
        }
    }

    @Mock
    AuthController mMockAuthController;
    @Mock
    SetupController mMockSetupController;
    @Mock
    SettingsProvider mMockSettingsProvider;
    @Mock
    AssistantManager mMockAssistantManager;
    @Mock
    NavController mMockNavController;
    @Mock
    Context mMockContext;
    @Mock
    PreferenceScreen mMockPrefScreen;
    @Mock
    Preference mMockPrefVoiceAssistanceNonAlexa;
    @Mock
    Preference mMockPrefVoiceAssistanceAlexa;
    @Mock
    Preference mMockPrefVoiceAssistanceEnableNonAlexa;
    @Mock
    Preference mMockPrefVoiceAssistanceEnableAlexa;
    @Mock
    Preference mMockPrefVoiceAssistancePushToTalk;
    @Mock
    Preference mMockPrefAlexaSettingsSignin;
    @Mock
    Preference mMockPrefAlexaSettingsDisable;
    @Mock
    Preference mMockPrefAlexaSettingsSignout;
    @Mock
    View mMockView;

    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    AlexaSetupWorkflowController mMockAlexaSetupWorkflowController;
    @Mock
    SetupProvider mMockSetupProvider;
    @Mock
    NavInflater mMockNavInflater;
    @Mock
    NavGraph mMockNavGraph;

    private VoiceAssistanceSettingsScreenBuilderOverride mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA))
                .thenReturn(mMockPrefVoiceAssistanceNonAlexa);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA))
                .thenReturn(mMockPrefVoiceAssistanceAlexa);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_NONALEXA))
                .thenReturn(mMockPrefVoiceAssistanceEnableNonAlexa);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_ALEXA))
                .thenReturn(mMockPrefVoiceAssistanceEnableAlexa);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_PUSH_TO_TALK))
                .thenReturn(mMockPrefVoiceAssistancePushToTalk);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNIN))
                .thenReturn(mMockPrefAlexaSettingsSignin);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DISABLE))
                .thenReturn(mMockPrefAlexaSettingsDisable);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNOUT))
                .thenReturn(mMockPrefAlexaSettingsSignout);
        Mockito.when(mMockPrefScreen.getContext()).thenReturn(mMockContext);

        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
            when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
            when(mMockRootComponent.getAuthController()).thenReturn(mMockAuthController);
            when(mMockRootComponent.getComponent(AlexaSetupWorkflowController.class))
                    .thenReturn(Optional.of(mMockAlexaSetupWorkflowController));
            when(mMockRootComponent.getComponent(SetupProvider.class)).thenReturn(Optional.of(mMockSetupProvider));

            mClassUnderTest = new VoiceAssistanceSettingsScreenBuilderOverride();
        }
    }

    @Test
    public void testPreferencesAreRemovedForNonAlexaCustomAssistantMode() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(false);

            // Action
            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Verify
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistancePushToTalk);
        }
    }

    @Test
    public void testPreferencesWhenBothAssistantsAreEnabled() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(BOTH_ENABLED);

            // Action
            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Verify
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignin);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsDisable);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignout);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableAlexa);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceAlexa);
        }
    }

    @Test
    public void testPreferencesWhenAlexaIsDisabled() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(ALEXA_DISABLED);

            // Action
            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Verify
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignin);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsDisable);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignout);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableAlexa);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceEnableAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceNonAlexa);
        }
    }

    @Test
    public void testPreferencesWhenNonAlexaIsDisabled() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(NON_ALEXA_DISABLED);

            // Action
            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Verify
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignin);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsDisable);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefAlexaSettingsSignout);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefVoiceAssistanceEnableAlexa);

            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceEnableNonAlexa);
            Mockito.verify(mMockPrefScreen, Mockito.times(1)).addPreference(mMockPrefVoiceAssistanceAlexa);
        }
    }

    @Test
    public void testAlexaEnablePreference() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(ALEXA_DISABLED);
            Mockito.when(mMockAuthController.getAuthMode()).thenReturn(AuthMode.CBL_AUTHORIZATION);
            Mockito.when(mMockNavController.getNavInflater()).thenReturn(mMockNavInflater);
            Mockito.when(mMockNavInflater.inflate(0)).thenReturn(mMockNavGraph);

            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Action
            mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

            // Verify
            ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                    ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
            Mockito.verify(mMockPrefVoiceAssistanceEnableAlexa, Mockito.times(1))
                    .setOnPreferenceClickListener(clickListenerCaptor.capture());

            // Invoke Signout preference by clicking on it.
            Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockPrefVoiceAssistanceEnableAlexa));

            // Verify
            Mockito.verify(mMockSetupController, Mockito.times(1)).setCurrentSetupFlow(ALEXA);
            Mockito.verify(mMockAuthController, Mockito.times(1)).logOut();
            Mockito.verify(mMockAlexaSetupWorkflowController, Mockito.times(1))
                    .startSetupWorkflow(mMockView.getContext(), mMockNavController, VoiceAssistanceEvent.ALEXA_ONLY);
        }
    }

    @Test
    public void testNonAlexaEnablePreference() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(NON_ALEXA_DISABLED);
            Mockito.when(mMockNavController.getNavInflater()).thenReturn(mMockNavInflater);
            Mockito.when(mMockNavInflater.inflate(0)).thenReturn(mMockNavGraph);

            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Action
            mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

            // Verify
            ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                    ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
            Mockito.verify(mMockPrefVoiceAssistanceEnableNonAlexa, Mockito.times(1))
                    .setOnPreferenceClickListener(clickListenerCaptor.capture());

            // Invoke Signout preference by clicking on it.
            Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockPrefVoiceAssistanceEnableNonAlexa));

            // Verify
            Mockito.verify(mMockSetupController, Mockito.times(1)).setCurrentSetupFlow(NONALEXA);
            Mockito.verify(mMockAlexaSetupWorkflowController, Mockito.times(1))
                    .startSetupWorkflow(
                            mMockView.getContext(), mMockNavController, VoiceAssistanceEvent.NON_ALEXA_ONLY);
        }
    }

    @Test
    public void testAlexaPreference() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(BOTH_ENABLED);
            Mockito.when(mMockNavController.getNavInflater()).thenReturn(mMockNavInflater);
            Mockito.when(mMockNavInflater.inflate(0)).thenReturn(mMockNavGraph);
            Mockito.when(mMockSettingsProvider.getSettingResId(ALEXA)).thenReturn(1);

            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Action
            mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

            // Verify
            ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                    ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
            Mockito.verify(mMockPrefVoiceAssistanceAlexa, Mockito.times(1))
                    .setOnPreferenceClickListener(clickListenerCaptor.capture());

            // Invoke Signout preference by clicking on it.
            Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockPrefVoiceAssistanceAlexa));

            // Verify
            Mockito.verify(mMockNavController, Mockito.times(1)).navigate(1);
        }
    }

    @Test
    public void testNonAlexaPreference() {
        try (MockedStatic<ModuleProvider> mockModuleProvider = Mockito.mockStatic(ModuleProvider.class)) {
            // Prepare
            mockModuleProvider.when(() -> ModuleProvider.isAlexaCustomAssistantEnabled(mMockContext)).thenReturn(true);
            Mockito.when(mMockAssistantManager.getAssistantsState()).thenReturn(BOTH_ENABLED);
            Mockito.when(mMockNavController.getNavInflater()).thenReturn(mMockNavInflater);
            Mockito.when(mMockNavInflater.inflate(0)).thenReturn(mMockNavGraph);
            Mockito.when(mMockSettingsProvider.getSettingResId(NONALEXA)).thenReturn(1);

            mClassUnderTest.addRemovePreferences(mMockPrefScreen);

            // Action
            mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

            // Verify
            ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                    ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
            Mockito.verify(mMockPrefVoiceAssistanceNonAlexa, Mockito.times(1))
                    .setOnPreferenceClickListener(clickListenerCaptor.capture());

            // Invoke Signout preference by clicking on it.
            Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockPrefVoiceAssistanceNonAlexa));

            // Verify
            Mockito.verify(mMockNavController, Mockito.times(1)).navigate(1);
        }
    }
}
