package com.amazon.alexa.auto.voiceinteraction.settings;

import android.app.Application;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppLoggedOutScopedComponent;
import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponentsActivator;
import com.amazon.alexa.auto.apis.app.DefaultLoggedOutScopedComponent;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.voiceinteraction.config.AACSConfigurator;
import com.amazon.alexa.auto.voiceinteraction.config.AlexaPropertyManager;
import com.amazon.alexa.auto.voiceinteraction.config.LocalesProvider;
import com.amazon.alexa.auto.voiceinteraction.dependencies.AndroidModule;
import com.amazon.alexa.auto.voiceinteraction.dependencies.DaggerSettingsComponent;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * ViewModel for {@link SettingsActivity}.
 */
public class SettingsActivityViewModel extends AndroidViewModel implements LoginUIEventListener {
    private static final String TAG = SettingsActivityViewModel.class.getSimpleName();

    public enum AuthState { LOGGED_OUT, LOGGED_IN }

    AuthController mAuthController;

    AlexaAppScopedComponentsActivator mScopedComponents;

    @Inject
    AACSConfigurator mAACSConfigurator;
    @Inject
    AlexaPropertyManager mAlexaPropertyManager;
    @Inject
    LocalesProvider mLocalesProvider;

    private @NonNull final DefaultLoggedOutScopedComponent mLoggedOutComponent;
    private @NonNull final MutableLiveData<AuthState> mAuthState;
    private Disposable mAuthStatusChangeSubscription;

    private boolean isLocaleUpdateNeeded;

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
        mAuthController = mApp.getRootComponent().getAuthController();
        mScopedComponents = mApp.getRootComponent().getScopedComponentsActivator();
        mLoggedOutComponent = new DefaultLoggedOutScopedComponent(this);

        initialize();
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        uninitialize();
    }

    public void transitionToLoggedInState() {
        Log.i(TAG, "Transitioning to logged in state.");

        mAuthState.setValue(AuthState.LOGGED_IN);

        deactivateLoggedOutScope();

        if (isLocaleUpdateNeeded) {
            applyLocalePropertyToAACS();
            isLocaleUpdateNeeded = false;
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

        applyConfigurationToAACS();

        mAuthState.setValue(AuthState.LOGGED_OUT);

        mScopedComponents.activateScope(mLoggedOutComponent);

        isLocaleUpdateNeeded = true;
    }

    public void initialize() {
        if (!mAuthController.isAuthenticated()) {
            transitionToLoggedOutState();
        } else {
            transitionToLoggedInState();
        }
    }

    public void uninitialize() {
        disposeLoggedInStateSubscriptions();
        deactivateLoggedOutScope();
    }

    /**
     * Send the local configuration to AACS.
     */
    private void applyConfigurationToAACS() {
        AACSServiceController.stopAACS(getApplication());
        AACSServiceController.startAACS(getApplication(), true);
        mAACSConfigurator.configureAACSWithPreferenceOverrides();
    }

    /**
     * Deactivate the logged out state apparatus.
     */
    private void deactivateLoggedOutScope() {
        mScopedComponents.deactivateScope(AlexaAppLoggedOutScopedComponent.class);
    }

    /**
     * Send the local property to AACS, if current selected locale is supported by Alexa, it will be used to update
     * Alexa Locale property setting, otherwise, set locale property to default en-US.
     */
    private void applyLocalePropertyToAACS() {
        String currentLocale = mLocalesProvider.getCurrentDeviceLocale();

        if (mLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale)) {
            Log.d(TAG, "Update Alexa locale to " + currentLocale);
            mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, currentLocale);
        } else {
            Log.d(TAG, "Current device locale is not supported by Alexa, update Alexa locale to en-US by default");
            mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, "en-US");
        }
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

    /**
     * Provides Auth state that can be monitored by the View.
     *
     * @return {@link LiveData} of {@link AuthState}.
     */
    public LiveData<AuthState> getAuthState() {
        return mAuthState;
    }
}
