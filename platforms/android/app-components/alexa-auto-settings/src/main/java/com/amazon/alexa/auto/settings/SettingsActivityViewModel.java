package com.amazon.alexa.auto.settings;

import android.app.Application;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
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
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.Disposable;

import static com.amazon.alexa.auto.setup.workflow.model.LocationConsent.DISABLED;

/**
 * ViewModel for {@link SettingsActivity}.
 */
public class SettingsActivityViewModel extends AndroidViewModel implements LoginUIEventListener {
    private static final String TAG = SettingsActivityViewModel.class.getSimpleName();

    public enum AuthState { LOGGED_OUT, LOGGED_IN, CBL_START }

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

    private boolean isLocaleUpdateNeeded;
    @Inject
    AlexaPropertyManager mAlexaPropertyManager;
    @Inject
    LocalesProvider mLocalesProvider;

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

        mAuthState.setValue(AuthState.CBL_START);
    }

    public void transitionToLoggedInState() {
        Log.i(TAG, "Transitioning to logged in state.");

        mAlexaSetupController.setSetupCompleteStatus(true);

        mAuthState.setValue(AuthState.LOGGED_IN);

        if (isLocaleUpdateNeeded) {
            if (mAuthController.getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
                applyLocalePropertyToAACS();
            }
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

        mAuthState.setValue(AuthState.LOGGED_OUT);

        isLocaleUpdateNeeded = true;
    }

    public void initialize() {
        mAppRootComponent.activateScope(this);

        if (!mAuthController.isAuthenticated() || !mAlexaSetupController.isSetupCompleted()) {
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
        String currentLocale = mLocalesProvider.getCurrentDeviceLocale();

        mLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale).subscribe(localeSupported -> {
            if (localeSupported) {
                Log.d(TAG, "Update Alexa locale to " + currentLocale);
                mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, currentLocale);
            }
        });
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
}
