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

import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.isDNDSettingEnabled;
import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.updateDNDSetting;
import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.isAlexaCustomAssistantEnabled;
import static com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider.isNavFavoritesEnabled;
import static com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider.updateNavFavoritesSetting;

import android.content.Context;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

/**
 * Alexa menu builder to take care of Auth related settings.
 */
public class AuthSettingsScreenBuilder implements AlexaSettingsScreenBuilder {
    private static final String TAG = AuthSettingsScreenBuilder.class.getSimpleName();

    private static final String LOGOUT_DIALOG_KEY = "logout";

    private Context mContext;
    private AuthController mAuthController;
    private LoginUIEventListener mLoginUIEventListener;
    private AlexaSetupController mAlexaSetupController;

    @Override
    public void addRemovePreferences(@NonNull PreferenceScreen screen) {
        if (mAuthController == null) {
            mAuthController = fetchAuthController(screen.getContext());
        }

        if (!isPreviewMode()) {
            Preference signInPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNIN);
            if (signInPref != null)
                screen.removePreference(signInPref);
            Preference disablePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DISABLE);
            if (disablePref != null)
                screen.removePreference(disablePref);
        } else {
            Preference signOutPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNOUT);
            if (signOutPref != null)
                screen.removePreference(signOutPref);

            if (!isAlexaCustomAssistantEnabled(screen.getContext())) {
                Preference commsPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_COMMUNICATION);
                if (commsPref != null)
                    screen.removePreference(commsPref);
            }
        }

        if (isPreviewMode() || isAlexaCustomAssistantEnabled(screen.getContext())) {
            Preference dndPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DO_NOT_DISTURB);
            if (dndPref != null)
                screen.removePreference(dndPref);
            Preference navFavPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_NAVI_FAVORITES);
            if (navFavPref != null)
                screen.removePreference(navFavPref);
        }
    }

    @Override
    public void installEventHandlers(@NonNull PreferenceScreen screen, @NonNull View view) {
        mContext = view.getContext();

        if (mAuthController == null) {
            mAuthController = fetchAuthController(view.getContext());
        }

        if (mLoginUIEventListener == null) {
            mLoginUIEventListener = fetchLoginUIEventListener(view.getContext());
        }

        if (mAlexaSetupController == null) {
            mAlexaSetupController = fetchAlexaSetupController(view.getContext());
        }

        if (!isPreviewMode() && mAuthController.isAuthenticated()) {
            installSignoutEventHandler(screen, view);
            installDNDEventHandler(screen, view);
            installNavigationFavoritesHandler(screen, view);
        } else {
            installDisableAlexaEventHandler(screen, view);
            installSigninEventHandler(screen, view);
        }

        EventBus.getDefault().register(this);
    }

    @Override
    public void dispose() {
        EventBus.getDefault().unregister(this);
    }

    private void installDNDEventHandler(@NonNull PreferenceScreen screen, @NonNull View view) {
        SwitchPreferenceCompat defaultAlexaDNDSetting =
                screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DO_NOT_DISTURB);
        if (defaultAlexaDNDSetting != null) {
            defaultAlexaDNDSetting.setChecked(isDNDSettingEnabled(view.getContext()));

            defaultAlexaDNDSetting.setOnPreferenceChangeListener((preference, newValue) -> {
                Log.d(TAG, "Changing DND setting to:" + newValue);
                return updateDNDSetting(view.getContext(), (boolean) newValue);
            });
        }
    }

    private void installNavigationFavoritesHandler(@NonNull PreferenceScreen screen, @NonNull View view) {
        SwitchPreferenceCompat defaultAlexaNavFavoriteSetting =
                screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_NAVI_FAVORITES);
        if (defaultAlexaNavFavoriteSetting != null) {
            defaultAlexaNavFavoriteSetting.setChecked(isNavFavoritesEnabled(view.getContext()));

            defaultAlexaNavFavoriteSetting.setOnPreferenceChangeListener((preference, newValue) -> {
                Log.d(TAG, "Changing Navi Favorites setting to:" + newValue);
                return updateNavFavoritesSetting(view.getContext(), (boolean) newValue);
            });
        } else {
            Log.w(TAG, "defaultAlexaNavFavoriteSetting is null");
        }
    }

    private void installSignoutEventHandler(@NonNull PreferenceScreen screen, @NonNull View view) {
        Preference signOutPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNOUT);
        if (signOutPref != null) {
            signOutPref.setOnPreferenceClickListener(auth -> {
                // Result of dialog selection are delivered through event bus to survive config
                // changes such as screen rotation. Look for onTwoChoiceDialogEvent method to
                // see how we handle the event.
                TwoChoiceDialog dialog = createTwoChoiceDialog(
                        new TwoChoiceDialog.Params(LOGOUT_DIALOG_KEY, view.getResources().getString(R.string.logout),
                                view.getResources().getString(R.string.logout_confirm),
                                view.getResources().getString(R.string.confirm_yesno_no),
                                view.getResources().getString(R.string.confirm_yesno_yes)));
                Context context = view.getContext();
                FragmentManager fragmentManager = ((AppCompatActivity) context).getSupportFragmentManager();
                dialog.show(fragmentManager, TAG);

                return true;
            });
        }
    }

    private void installDisableAlexaEventHandler(@NonNull PreferenceScreen screen, @NonNull View view) {
        Preference disablePref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DISABLE);
        if (disablePref != null) {
            disablePref.setOnPreferenceClickListener(auth -> {
                TwoChoiceDialog dialog = createTwoChoiceDialog(new TwoChoiceDialog.Params(LOGOUT_DIALOG_KEY,
                        view.getResources().getString(R.string.disable_alexa),
                        view.getResources().getString(R.string.disable_alexa_confirm),
                        view.getResources().getString(R.string.confirm_yesno_no),
                        view.getResources().getString(R.string.confirm_yesno_disable)));
                Context context = view.getContext();
                FragmentManager fragmentManager = ((AppCompatActivity) context).getSupportFragmentManager();
                dialog.show(fragmentManager, TAG);

                return true;
            });
        }
    }

    private void installSigninEventHandler(@NonNull PreferenceScreen screen, @NonNull View view) {
        Preference signInPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNIN);
        if (signInPref != null) {
            signInPref.setOnPreferenceClickListener(auth -> {
                mLoginUIEventListener.loginSwitched(AuthMode.CBL_AUTHORIZATION);
                return true;
            });
        }
    }

    @VisibleForTesting
    TwoChoiceDialog createTwoChoiceDialog(@NonNull TwoChoiceDialog.Params params) {
        return TwoChoiceDialog.createDialog(params);
    }

    @VisibleForTesting
    AuthController fetchAuthController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAuthController();
    }

    @VisibleForTesting
    LoginUIEventListener fetchLoginUIEventListener(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getComponent(LoginUIEventListener.class).orElse(null);
    }

    @VisibleForTesting
    AlexaSetupController fetchAlexaSetupController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAlexaSetupController();
    }

    @Subscribe
    public void onTwoChoiceDialogEvent(TwoChoiceDialog.Button2Clicked event) {
        if (LOGOUT_DIALOG_KEY.equals(event.dialogKey)) {
            Log.i(TAG, "Logging user out");
            mAlexaSetupController.setSetupCompleteStatus(false);
            mAuthController.logOut();
        }
    }

    private boolean isPreviewMode() {
        AuthMode currentMode = mAuthController.getAuthMode();
        return !currentMode.equals(AuthMode.CBL_AUTHORIZATION);
    }
}
