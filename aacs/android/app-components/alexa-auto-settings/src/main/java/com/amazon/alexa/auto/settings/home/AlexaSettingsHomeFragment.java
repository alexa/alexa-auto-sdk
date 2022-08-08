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

import static com.amazon.aacsconstants.AACSPropertyConstants.GEOLOCATION_ENABLED;
import static com.amazon.aacsconstants.AACSPropertyConstants.LOCALE;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_SUPPORTED;
import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.updateDNDInPreferences;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.DISABLED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.ENABLED;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.lifecycle.LiveData;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;
import com.amazon.alexa.auto.settings.DNDChangeMessage;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;
import com.amazon.alexa.auto.setup.workflow.model.UserConsent;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.util.List;
import java.util.Optional;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * Settings fragment for Alexa menu home screen.
 */
public class AlexaSettingsHomeFragment extends PreferenceFragmentCompat {
    private static final String TAG = AlexaSettingsHomeFragment.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;
    @Inject
    AlexaLocalesProvider mLocalesProvider;
    @Inject
    List<AlexaSettingsScreenBuilder> mScreenBuilders;

    private AuthController mAuthController;
    private CompositeDisposable mInFlightOperations;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.alexa_preferences, rootKey);

        Context context = getContext();
        if (context == null) {
            Log.w(TAG, "Failed to obtain Context");
            return;
        }

        PreferenceManager.setDefaultValues(context, R.xml.alexa_preferences, false);
        if (mAlexaPropertyManager == null) {
            // Inject only if it is not already injected by test.
            DaggerSettingsComponent.builder().androidModule(new AndroidModule(context)).build().inject(this);
        }

        PreferenceScreen screen = getPreferenceScreen();
        mScreenBuilders.forEach(processor -> processor.addRemovePreferences(screen));
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mInFlightOperations = new CompositeDisposable();

        if (mAuthController == null) {
            mAuthController = fetchAuthController(view.getContext());
        }

        installHandsFreeEventHandler();
        installLocationConsentHandler();
        installLanguageEventHandler();
        installSoundsEventHandler();
        installThingsToTryEventHandler();

        if (!isPreviewMode() || ModuleProvider.isAlexaCustomAssistantEnabled(view.getContext())) {
            installCommunicationsEventHandler();
        }

        PreferenceScreen screen = getPreferenceScreen();
        mScreenBuilders.forEach(processor -> processor.installEventHandlers(screen, view));

        // Decorate the last item to not have bottom bar.
        Preference lastPreference = screen.getPreference(screen.getPreferenceCount() - 1);
        lastPreference.setLayoutResource(R.layout.alexa_last_preference_layout);

        // Add margin per UI spec.
        ViewGroup.LayoutParams layoutParams = view.getLayoutParams();
        if (layoutParams instanceof ViewGroup.MarginLayoutParams) {
            ViewGroup.MarginLayoutParams marginLayoutParams = (ViewGroup.MarginLayoutParams) layoutParams;
            marginLayoutParams.setMarginStart(
                    (int) getResources().getDimension(R.dimen.item_horizontal_margin_quadruple));
            marginLayoutParams.setMarginEnd((int) getResources().getDimension(R.dimen.item_horizontal_margin_double));
            view.setLayoutParams(layoutParams);
        }
        EventBus.getDefault().register(this);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mInFlightOperations.dispose();
        mScreenBuilders.forEach(AlexaSettingsScreenBuilder::dispose);
        EventBus.getDefault().unregister(this);
    }

    private void installHandsFreeEventHandler() {
        SwitchPreferenceCompat defaultAlexaHandsFree = findPreference(PreferenceKeys.ALEXA_SETTINGS_HANDS_FREE);

        mInFlightOperations.add(
                mAlexaPropertyManager.getAlexaPropertyBoolean(WAKEWORD_SUPPORTED)
                        .filter(Optional::isPresent)
                        .map(Optional::get)
                        .subscribe(wakeWordSupported -> {
                            if (!wakeWordSupported) {
                                Log.d(TAG, "Alexa wake word is disabled, removing the configuration for handsfree");
                                if (defaultAlexaHandsFree != null) {
                                    getPreferenceScreen().removePreference(defaultAlexaHandsFree);
                                }
                                return;
                            }

                            Preconditions.checkNotNull(defaultAlexaHandsFree);

                            mInFlightOperations.add(mAlexaPropertyManager.getAlexaPropertyBoolean(WAKEWORD_ENABLED)
                                                            .filter(Optional::isPresent)
                                                            .map(Optional::get)
                                                            .subscribe(defaultAlexaHandsFree::setChecked));

                            defaultAlexaHandsFree.setOnPreferenceChangeListener((preference, newValue) -> {
                                Log.d(TAG, "Changing hands-free status to:" + newValue);
                                mAlexaPropertyManager.updateAlexaProperty(WAKEWORD_ENABLED, newValue.toString())
                                        .subscribe(update -> {});
                                return true;
                            });
                        }));
    }

    private void installLocationConsentHandler() {
        SwitchPreferenceCompat defaultLocationConsent = findPreference(PreferenceKeys.ALEXA_SETTINGS_LOCATION_CONSENT);
        String extraModules = ModuleProvider.getModules(getContext());
        if (defaultLocationConsent != null && !extraModules.contains(ModuleProvider.ModuleName.GEOLOCATION.name())) {
            getPreferenceScreen().removePreference(defaultLocationConsent);
            return;
        }

        mInFlightOperations.add(
                mAlexaPropertyManager.getAlexaProperty(GEOLOCATION_ENABLED)
                        .filter(Optional::isPresent)
                        .map(Optional::get)
                        .subscribe(currentConsent -> {
                            Preconditions.checkNotNull(defaultLocationConsent);
                            Preconditions.checkNotNull(currentConsent);
                            Log.d(TAG, "Geolocation current value:" + currentConsent);
                            defaultLocationConsent.setChecked(currentConsent.equals(ENABLED.getValue()));

                            defaultLocationConsent.setOnPreferenceChangeListener((preference, newValue) -> {
                                UserConsent newConsent = (boolean) newValue ? ENABLED : DISABLED;
                                Log.d(TAG, "Changing geolocation value to: " + newConsent.getValue());
                                mAlexaPropertyManager.updateAlexaProperty(GEOLOCATION_ENABLED, newConsent.getValue())
                                        .doOnSuccess((succeeded) -> {
                                            if (succeeded) {
                                                Log.d(TAG,
                                                        "Successfully updated geolocation value to: "
                                                                + newConsent.getValue());
                                            } else {
                                                Log.e(TAG,
                                                        "Failed to update geolocation value to: "
                                                                + newConsent.getValue());
                                            }
                                        })
                                        .doOnError(throwable -> {
                                            Log.e(TAG, "Failed to update geolocation to: " + newConsent.getValue());
                                        })
                                        .subscribe();
                                return true;
                            });
                        }));
    }

    private void installLanguageEventHandler() {
        mAlexaPropertyManager.updateAlexaLocaleWithPersistentConfig();

        Preference defaultAlexaLanguages = findPreference(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES);
        Preconditions.checkNotNull(defaultAlexaLanguages);

        mInFlightOperations.add(mAlexaPropertyManager.getAlexaProperty(LOCALE)
                                        .filter(Optional::isPresent)
                                        .map(Optional::get)
                                        .subscribe(propValue -> {
                                            mInFlightOperations.add(
                                                    mLocalesProvider.fetchAlexaSupportedLocaleWithId(propValue)
                                                            .filter(Optional::isPresent)
                                                            .map(Optional::get)
                                                            .subscribe(localeValue -> {
                                                                defaultAlexaLanguages.setSummary(localeValue.first
                                                                        + " (" + localeValue.second + ")");
                                                            }));
                                        }));

        defaultAlexaLanguages.setOnPreferenceClickListener(preference -> {
            View view = getView();
            if (view != null) {
                NavController navController = findNavController(view);
                navController.navigate(R.id.navigation_fragment_alexa_settings_languages);
            }
            return false;
        });
    }

    private void installThingsToTryEventHandler() {
        Preference defaultTTT = findPreference(PreferenceKeys.ALEXA_SETTINGS_THINGS_TO_TRY);
        if (defaultTTT != null) {
            defaultTTT.setOnPreferenceClickListener(preference -> {
                View view = getView();
                if (view != null) {
                    NavController navController = findNavController(view);
                    navController.navigate(R.id.navigation_fragment_alexa_settings_things_to_try);
                }
                return false;
            });
        } else {
            Log.i(TAG, "defaultTTT is null");
        }
    }

    private void installSoundsEventHandler() {
        Preference defaultAlexaSounds = findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUNDS);
        Preconditions.checkNotNull(defaultAlexaSounds);
        defaultAlexaSounds.setOnPreferenceClickListener(preference -> {
            View view = getView();
            if (view != null) {
                NavController navController = findNavController(view);
                navController.navigate(R.id.navigation_fragment_alexa_settings_sounds);
            }
            return false;
        });
    }

    private void installCommunicationsEventHandler() {
        Preference defaultAlexaComms = findPreference(PreferenceKeys.ALEXA_SETTINGS_COMMUNICATION);
        Preconditions.checkNotNull(defaultAlexaComms);

        LiveData<List<ConnectedBTDevice>> listData =
                ConnectedBTDeviceRepository.getInstance(getContext()).getConnectedDevices();
        listData.observe(getViewLifecycleOwner(), listObserver -> {
            if (listData.getValue() != null && listData.getValue().size() > 0) {
                // Per Android telephony, it uses last paired bluetooth device for calling and handle messages.
                String deviceAddress = listData.getValue().get(listData.getValue().size() - 1).getDeviceAddress();
                LiveData<BTDevice> device =
                        BTDeviceRepository.getInstance(getContext()).getBTDeviceByAddress(deviceAddress);
                device.observe(getViewLifecycleOwner(), observer -> {
                    if (device.getValue() != null) {
                        defaultAlexaComms.setSummary(device.getValue().getDeviceName());
                    } else {
                        Log.d(TAG, "Primary device is not found.");
                        defaultAlexaComms.setSummary("None");
                    }
                });
            } else {
                Log.d(TAG, "Device is not found.");
                defaultAlexaComms.setSummary("None");
            }

            defaultAlexaComms.setOnPreferenceClickListener(preference -> {
                View view = getView();
                if (view != null) {
                    NavController navController = findNavController(view);
                    navController.navigate(R.id.navigation_fragment_alexa_settings_communication);
                }
                return false;
            });
        });
    }

    @Subscribe
    public void onDNDChangedEvent(@NonNull DNDChangeMessage message) {
        SwitchPreferenceCompat defaultAlexaDNDSetting = findPreference(PreferenceKeys.ALEXA_SETTINGS_DO_NOT_DISTURB);
        if (defaultAlexaDNDSetting != null) {
            if (defaultAlexaDNDSetting.isChecked() != message.isChecked()) {
                defaultAlexaDNDSetting.setChecked(message.isChecked());
                updateDNDInPreferences(getContext(), message.isChecked());
            }
        }
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }

    @VisibleForTesting
    AuthController fetchAuthController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAuthController();
    }

    private boolean isPreviewMode() {
        AuthMode currentMode = mAuthController.getAuthMode();
        return !currentMode.equals(AuthMode.CBL_AUTHORIZATION);
    }
}
