package com.amazon.alexa.auto.settings.home;

import static com.amazon.aacsconstants.AACSPropertyConstants.LOCALE;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_SUPPORTED;

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
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

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
    LocalesProvider mLocalesProvider;
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
        installLanguageEventHandler();

        if (!isPreviewMode()) {
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
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mInFlightOperations.dispose();
        mScreenBuilders.forEach(AlexaSettingsScreenBuilder::dispose);
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

    private void installLanguageEventHandler() {
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
                        defaultAlexaComms.setSummary("");
                    }
                });
            } else {
                Log.d(TAG, "Device is not found.");
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
