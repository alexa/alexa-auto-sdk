/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.settings.home;

import static com.amazon.alexa.auto.apps.common.Constants.ALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.ALEXA_DISABLED;
import static com.amazon.alexa.auto.apps.common.Constants.ASSISTANTS_GESTURE_CHANGED;
import static com.amazon.alexa.auto.apps.common.Constants.ASSISTANTS_STATE_CHANGED;
import static com.amazon.alexa.auto.apps.common.Constants.BOTH_DISABLED;
import static com.amazon.alexa.auto.apps.common.Constants.BOTH_ENABLED;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.NON_ALEXA_DISABLED;
import static com.amazon.alexa.auto.apps.common.Constants.PUSH_TO_TALK;

import android.content.Context;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.navigation.NavController;
import androidx.navigation.NavGraph;
import androidx.navigation.Navigation;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SettingsProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupController;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.message.AssistantMessage;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;
import com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Optional;

/**
 * Alexa menu builder to take care of voice assistance settings.
 */
public class VoiceAssistanceSettingsScreenBuilder implements AlexaSettingsScreenBuilder {
    private static final String TAG = VoiceAssistanceSettingsScreenBuilder.class.getCanonicalName();

    private AlexaApp mApp;
    private AuthController mAuthController;
    private SettingsProvider mSettingsProvider;
    private AssistantManager mAssistantManager;
    private SetupController mSetupController;
    private PreferenceScreen mScreen;

    Preference mAlexaPref;
    Preference mNonAlexaPref;
    Preference mAlexaEnablePref;
    Preference mNonAlexaEnablePref;
    Preference mPushToTalkPref;

    @Override
    public void addRemovePreferences(@NonNull PreferenceScreen screen) {
        Log.d(TAG, "addRemovePreferences");
        mAlexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA);
        mNonAlexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA);
        mAlexaEnablePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_ALEXA);
        mNonAlexaEnablePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_NONALEXA);
        mPushToTalkPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_PUSH_TO_TALK);

        if (!ModuleProvider.isAlexaCustomAssistantEnabled(screen.getContext())) {
            if (mNonAlexaPref != null)
                screen.removePreference(mNonAlexaPref);

            if (mAlexaPref != null)
                screen.removePreference(mAlexaPref);

            if (mAlexaEnablePref != null)
                screen.removePreference(mAlexaEnablePref);

            if (mNonAlexaEnablePref != null)
                screen.removePreference(mNonAlexaEnablePref);

            if (mPushToTalkPref != null)
                screen.removePreference(mPushToTalkPref);
        } else {
            Preference languagePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES);
            if (languagePref != null) {
                languagePref.setTitle(R.string.setting_voice_assistance_language);
            }

            Preference handsfreePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_HANDS_FREE);
            if (handsfreePref != null) {
                handsfreePref.setTitle(R.string.setting_voice_assistance_wake_word);
                handsfreePref.setSummary(R.string.setting_voice_assistance_wake_word_summary);
            }

            Preference locationConsentPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_LOCATION_CONSENT);
            if (locationConsentPref != null) {
                locationConsentPref.setSummary(R.string.setting_voice_assistance_setting_location_consent_summary);
            }

            // These settings will be within assistant settings instead of in the root level
            Preference signInPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNIN);
            if (signInPref != null)
                screen.removePreference(signInPref);
            Preference disablePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DISABLE);
            if (disablePref != null)
                screen.removePreference(disablePref);
            Preference signOutPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNOUT);
            if (signOutPref != null)
                screen.removePreference(signOutPref);
            Preference thingsToTryPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_THINGS_TO_TRY);
            if (thingsToTryPref != null)
                screen.removePreference(thingsToTryPref);

            if (mApp == null) {
                mApp = fetchAlexaApp(screen.getContext());
            }

            if (mAuthController == null) {
                mAuthController = fetchAuthController(screen.getContext());
            }

            if (mSetupController == null) {
                mSetupController = fetchVoiceAssistanceSetupController(screen.getContext());
            }

            if (mAssistantManager == null) {
                mAssistantManager = fetchAssistantManager(screen.getContext());
            }

            if (mScreen == null) {
                mScreen = screen;
            }

            EventBus.getDefault().register(this);

            updateAssistantPreferences(screen, mAssistantManager.getAssistantsState());
            updatePTTPreference(mAssistantManager.getDefaultAssistantForPTT());
        }
    }

    @Override
    public void installEventHandlers(@NonNull PreferenceScreen screen, @NonNull View view) {
        Log.d(TAG, "installEventHandlers");
        if (ModuleProvider.isAlexaCustomAssistantEnabled(screen.getContext())) {
            if (mSettingsProvider == null) {
                mSettingsProvider = fetchSettingsProvider(screen.getContext());
                if (mSettingsProvider == null) {
                    Log.w(TAG, "Alexa setting provider is null");
                    return;
                }
            }

            if (mAlexaPref != null) {
                mAlexaPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    updateNavigationGraph(navController);
                    int alexaSettingResId = mSettingsProvider.getSettingResId(ALEXA);
                    if (alexaSettingResId != 0) {
                        navController.navigate(alexaSettingResId);
                    } else {
                        Log.e(TAG, "alexaSettingResId is invalid");
                        return false;
                    }
                    return true;
                });
            }

            if (mNonAlexaPref != null) {
                mNonAlexaPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    updateNavigationGraph(navController);
                    int nonAlexaSettingResId = mSettingsProvider.getSettingResId(NONALEXA);
                    if (nonAlexaSettingResId != 0) {
                        navController.navigate(nonAlexaSettingResId);
                    } else {
                        Log.e(TAG, "nonAlexaSettingResId is invalid");
                        return false;
                    }
                    return true;
                });
            }

            if (mAlexaEnablePref != null) {
                mAlexaEnablePref.setOnPreferenceClickListener(pref -> {
                    Log.i(TAG, "Enabling Alexa");

                    mSetupController.setCurrentSetupFlow(ALEXA);

                    // Clear out token so we can restart the login flow for alexa
                    mAuthController.logOut();

                    resetNavigationGraphWithAlexaSetupFlowAsInitialDestination(view);
                    return true;
                });
            }

            if (mNonAlexaEnablePref != null) {
                mNonAlexaEnablePref.setOnPreferenceClickListener(pref -> {
                    Log.i(TAG, "Enabling Non Alexa");

                    mSetupController.setCurrentSetupFlow(NONALEXA);

                    resetNavigationGraphWithNonAlexaSetupFlowAsInitialDestination(view);
                    return true;
                });
            }

            if (mPushToTalkPref != null) {
                mPushToTalkPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    updateNavigationGraph(navController);
                    int alexaSettingResId = mSettingsProvider.getSettingResId(PUSH_TO_TALK);
                    if (alexaSettingResId != 0) {
                        navController.navigate(alexaSettingResId);
                    } else {
                        Log.e(TAG, "alexaSettingResId is invalid");
                        return false;
                    }
                    return true;
                });
            }
        }
    }

    @Override
    public void dispose() {
        Log.d(TAG, "dispose");
        EventBus.getDefault().unregister(this);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onAssistantMessage(AssistantMessage message) {
        Log.d(TAG, "Receiving assistant message, action: " + message.getAction());
        String action = message.getAction();

        switch (action) {
            case ASSISTANTS_GESTURE_CHANGED:
                updatePTTPreference(message.getAssistantName());
                break;
            case ASSISTANTS_STATE_CHANGED:
                updateAssistantPreferences(mScreen, message.getPayload());
                break;
        }
    }

    private void updatePTTPreference(String defaultPTTAssistant) {
        if (mPushToTalkPref != null) {
            mPushToTalkPref.setSummary(ALEXA.equals(defaultPTTAssistant)
                            ? R.string.setting_voice_assistance_setting_push_to_talk_alexa
                            : R.string.setting_voice_assistance_setting_push_to_talk_non_alexa);
        }
    }

    private void updateAssistantPreferences(@NonNull PreferenceScreen screen, String assistantState) {
        Log.d(TAG, "updateAssistantPreferences: " + mAssistantManager.getAssistantsState());

        if (mAlexaPref != null) {
            screen.removePreference(mAlexaPref);
        }

        if (mAlexaEnablePref != null) {
            screen.removePreference(mAlexaEnablePref);
        }

        if (mNonAlexaPref != null) {
            screen.removePreference(mNonAlexaPref);
        }

        if (mNonAlexaEnablePref != null) {
            screen.removePreference(mNonAlexaEnablePref);
        }

        switch (assistantState) {
            case ALEXA_DISABLED:
                screen.addPreference(mAlexaEnablePref);
                screen.addPreference(mNonAlexaPref);
                break;
            case NON_ALEXA_DISABLED:
                screen.addPreference(mNonAlexaEnablePref);
                screen.addPreference(mAlexaPref);
                break;
            case BOTH_ENABLED:
                screen.addPreference(mAlexaPref);
                screen.addPreference(mNonAlexaPref);
                break;
            case BOTH_DISABLED:
                screen.addPreference(mAlexaEnablePref);
                screen.addPreference(mNonAlexaEnablePref);
                break;
        }
    }

    /**
     * Reset the navigation graph and make the initial destination as Alexa-only setup view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private void resetNavigationGraphWithAlexaSetupFlowAsInitialDestination(@NonNull View view) {
        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.i(TAG, "Switching navigation graph's destination to alexa-only view");
            NavController controller = findNavController(view);

            SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
            NavGraph graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
            graph.setStartDestination(setupProvider.getSetupWorkflowStartDestinationByKey(ALEXA));

            controller.setGraph(graph);

            Log.d(TAG, "Activate Alexa setup workflow controller");
            mApp.getRootComponent().activateScope(new AlexaSetupWorkflowControllerImpl(view.getContext()));
            mApp.getRootComponent()
                    .getComponent(AlexaSetupWorkflowController.class)
                    .ifPresent(alexaSetupWorkflowController -> {
                        alexaSetupWorkflowController.startSetupWorkflow(
                                view.getContext(), controller, VoiceAssistanceEvent.ALEXA_ONLY);
                    });
        }
    }

    /**
     * Reset the navigation graph and make the initial destination as Non-Alexa-only setup view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private void resetNavigationGraphWithNonAlexaSetupFlowAsInitialDestination(@NonNull View view) {
        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.i(TAG, "Switching navigation graph's destination to non-alexa-only view");
            NavController controller = findNavController(view);
            SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
            NavGraph graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
            graph.setStartDestination(setupProvider.getSetupWorkflowStartDestinationByKey(NONALEXA));

            controller.setGraph(graph);

            Log.d(TAG, "Activate Alexa setup workflow controller");
            mApp.getRootComponent().activateScope(new AlexaSetupWorkflowControllerImpl(view.getContext()));
            mApp.getRootComponent()
                    .getComponent(AlexaSetupWorkflowController.class)
                    .ifPresent(alexaSetupWorkflowController -> {
                        alexaSetupWorkflowController.startSetupWorkflow(
                                view.getContext(), controller, VoiceAssistanceEvent.NON_ALEXA_ONLY);
                    });
        }
    }

    private void updateNavigationGraph(NavController navController) {
        NavGraph graph = navController.getNavInflater().inflate(mSettingsProvider.getCustomSettingNavigationGraph());
        graph.setStartDestination(mSettingsProvider.getSettingStartDestination());
        navController.setGraph(graph);
    }

    @VisibleForTesting
    AlexaApp fetchAlexaApp(@NonNull Context context) {
        return AlexaApp.from(context);
    }

    @VisibleForTesting
    AuthController fetchAuthController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAuthController();
    }

    @VisibleForTesting
    SetupController fetchVoiceAssistanceSetupController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getComponent(SetupController.class).orElse(null);
    }

    @VisibleForTesting
    SettingsProvider fetchSettingsProvider(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        Optional<SettingsProvider> settingsProvider = app.getRootComponent().getComponent(SettingsProvider.class);
        return settingsProvider.orElse(null);
    }

    @VisibleForTesting
    AssistantManager fetchAssistantManager(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        Optional<AssistantManager> assistantManager = app.getRootComponent().getComponent(AssistantManager.class);
        return assistantManager.orElse(null);
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
