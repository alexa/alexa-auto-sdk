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
package com.amazon.alexa.auto.settings;

import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.resetDNDSetting;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.resetEarconSettings;
import static com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil.clearDiscoveredFeatures;
import static com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider.updateNavFavoritesSetting;

import android.app.Application;
import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.os.LocaleListCompat;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;
import io.reactivex.rxjava3.disposables.Disposable;

/**
 * ViewModel for {@link SettingsActivity}.
 */
public class SettingsActivityViewModel extends AndroidViewModel implements LoginUIEventListener {
    private static final String TAG = SettingsActivityViewModel.class.getSimpleName();

    public enum AuthState {LOGGED_OUT, LOGGED_IN, CBL_START}

    @NonNull
    private final AlexaAppRootComponent mAppRootComponent;
    @NonNull
    private final AuthController mAuthController;
    @NonNull
    private final AlexaSetupController mAlexaSetupController;
    @NonNull
    private final MutableLiveData<AuthState> mAuthState;
    @Nullable
    private Disposable mAuthStatusChangeSubscription;
    @Inject
    AlexaPropertyManager mAlexaPropertyManager;
    @Inject
    AlexaLocalesProvider mAlexaLocalesProvider;

    CompositeDisposable mCompositeDisposable;

    /**
     * Creates an instance of {@link SettingsActivityViewModel}.
     *
     * @param application Android {@link Application}.
     */
    public SettingsActivityViewModel(@NonNull Application application) {
        super(application);

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(application)).build().inject(this);

        mAuthState = new MutableLiveData<>();

        AlexaApp mApp = AlexaApp.from(application);
        mAppRootComponent = mApp.getRootComponent();
        mAuthController = mAppRootComponent.getAuthController();
        mAlexaSetupController = mAppRootComponent.getAlexaSetupController();

        initialize();
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        uninitialize();
    }

    public void transitionToSwitchCBLStartState() {
        Log.i(TAG, "Transitioning to CBL start state.");
        mAlexaSetupController.setSetupCompleteStatus(false);
        mAuthState.setValue(AuthState.CBL_START);
    }

    public void transitionToLoggedInState() {
        Log.i(TAG, "Transitioning to logged in state.");

        mAlexaSetupController.setSetupCompleteStatus(true);

        mAuthState.setValue(AuthState.LOGGED_IN);

        if (mAuthController.getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            applyLocalePropertyToAACS();
        }


        mAuthStatusChangeSubscription = mAuthController.observeAuthChangeOrLogOut()
                .map(AuthStatus::getLoggedIn)
                .distinctUntilChanged()
                .subscribe(authStatus -> {
                    if (!authStatus) {
                        transitionToLoggedOutState();
                    }
                });
    }

    public void transitionToLoggedOutState() {
        Log.i(TAG, "Transitioning to logged out state.");

        disposeLoggedInStateSubscriptions();
        resetUserPreferences(getApplication().getApplicationContext());

        mAuthState.setValue(AuthState.LOGGED_OUT);
    }

    public void initialize() {
        mAppRootComponent.activateScope(this);
        mCompositeDisposable = new CompositeDisposable();

        if (!mAuthController.isAuthenticated()) {
            transitionToLoggedOutState();
        } else {
            transitionToLoggedInState();
        }
    }

    public void uninitialize() {
        disposeLoggedInStateSubscriptions();
        deactivateLoggedOutScope();
        mCompositeDisposable.dispose();
    }

    /**
     * Reset preferences to default values
     *
     * @param context
     */
    private void resetUserPreferences(Context context) {
        mAlexaSetupController.observeAACSReadiness().subscribe(isAACSReady -> {
            if (isAACSReady) {
                Log.i(TAG, "AACS is ready. Resetting user preferences.");
                resetEarconSettings(context);
                resetDNDSetting(context);
                clearDiscoveredFeatures(context);
                updateNavFavoritesSetting(context, false);
                mCompositeDisposable.add(
                        mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, null).subscribe(successStatus -> {
                            Log.d(TAG, "Success Status: " + successStatus);
                        })
                );
            }
        });
    }

    /**
     * Deactivate the logged out state apparatus.
     */
    private void deactivateLoggedOutScope() {
        mAppRootComponent.deactivateScope(LoginUIEventListener.class);
    }

    /**
     * Send the local property to AACS, if current selected locale is supported by Alexa, it will be used to update
     * Alexa Locale property setting, otherwise, we have set the locale based on user's choice in the setup.
     */
    private void applyLocalePropertyToAACS() {
        String currentLocale = LocaleUtil.INSTANCE.parseAndroidLocaleListToAlexaLocaleString(
                AppCompatDelegate.getApplicationLocales()
        );
        mCompositeDisposable.add(
                mAlexaLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale).subscribe(localeSupported -> {
                    if (localeSupported) {
                        Log.d(TAG, "Update Alexa locale to " + currentLocale);

                        mCompositeDisposable.add(
                                mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, currentLocale).subscribe(successStatus -> {
                                    Log.d(TAG, "Success: " + successStatus);
                                })
                        );
                    }
                })
        );

    }

    /**
     * Dispose the logged in state subscriptions.
     */
    private void disposeLoggedInStateSubscriptions() {
        if (mAuthStatusChangeSubscription != null) {
            mAuthStatusChangeSubscription.dispose();
            mAuthStatusChangeSubscription = null;
        }
    }

    @Override
    public void loginFinished() {
        transitionToLoggedInState();
    }

    @Override
    public void loginSwitched(AuthMode mode) {
        if (mode.equals(AuthMode.CBL_AUTHORIZATION)) {
            transitionToSwitchCBLStartState();
        }
    }

    /**
     * Provides Auth state that can be monitored by the View.
     *
     * @return {@link LiveData} of {@link AuthState}.
     */
    public LiveData<AuthState> getAuthState() {
        return mAuthState;
    }

    public void disableWakeWord() {
        mAlexaPropertyManager.updateAlexaProperty(WAKEWORD_ENABLED, "false")
                .doOnSuccess((succeeded) -> {
                    if (!succeeded) {
                        Log.d(TAG, " Wakeword disable failed ");
                    }
                })
                .subscribe();
    }

    public void enableWakeWord() {
        mAlexaPropertyManager.updateAlexaProperty(WAKEWORD_ENABLED, "true")
                .doOnSuccess((succeeded) -> {
                    if (!succeeded) {
                        Log.d(TAG, " Wakeword enable failed ");
                    }
                })
                .subscribe();
    }
}
