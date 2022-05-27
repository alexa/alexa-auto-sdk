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
package com.amazon.alexa.auto.setup.workflow.fragment;

import android.app.Application;
import android.os.Handler;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.UserIdentity;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * ViewModel for @{link LoginFragment}
 */
public class LoginViewModel extends AndroidViewModel {
    private static final String TAG = LoginViewModel.class.getSimpleName();

    public final static int WAIT_FOR_LOGIN_START_MS = 5000;

    private final @NonNull AuthController mAuthController;
    private final @NonNull AlexaSetupController mAlexaSetupController;
    private final @Nullable LoginUIEventListener mUIEventListener;
    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    private MutableLiveData<AuthWorkflowData> mAuthWorkflowState = new MutableLiveData<>();
    private Disposable mLoginWorkflowSubscription;
    @Nullable
    private Disposable mAuthStatusChangeSubscription;

    // State
    private final WaitForStartLoginRunnable mWaitForStartLogin;

    /**
     * Constructor for LoginViewModel
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public LoginViewModel(@NonNull Application application) {
        super(application);

        AlexaApp app = AlexaApp.from(application);

        mAuthController = app.getRootComponent().getAuthController();
        mAlexaSetupController = app.getRootComponent().getAlexaSetupController();
        mUIEventListener = app.getRootComponent().getComponent(LoginUIEventListener.class).orElse(null);
        DaggerSetupComponent.builder().androidModule(new AndroidModule(application)).build().injectLoginViewModel(this);

        mWaitForStartLogin = new WaitForStartLoginRunnable();
    }

    @Override
    public void onCleared() {
        if (this.mLoginWorkflowSubscription != null) {
            this.mLoginWorkflowSubscription.dispose();
        }

        if (mAuthStatusChangeSubscription != null) {
            mAuthStatusChangeSubscription.dispose();
            mAuthStatusChangeSubscription = null;
        }
    }

    /**
     * {@link LiveData} to fetch last CBL Login Workflow state as well as
     * to listen to subsequent state changes.
     *
     * @return {@link LiveData} of {@link AuthWorkflowData}
     */
    public LiveData<AuthWorkflowData> loginWorkflowState() {
        return mAuthWorkflowState;
    }

    /**
     * Start the new Authorization Login workflow.
     *
     * Login workflow state changes can be listened to by subscribing to
     * {@link LiveData} fetched from {@link #loginWorkflowState()}
     */
    public void startLogin() {
        Log.d(TAG, "Authorization Login Workflow starting");

        mAlexaPropertyManager.updateAlexaLocaleWithPersistentConfig();

        if (!this.mAuthController.isAuthenticated()) {
            // Why not expose the RX stream directly to the UI instead of transforming
            // it to LiveData? The reason is that ViewModel can survive configuration
            // changes (display size changes, orientation changes) but View cannot.
            // On configuration changes, we do not wish to interrupt the login process.
            // Therefore ViewModel owns the subscription to auth workflow stream and
            // login UI can reconnect to login workflow any time using LiveData.
            mAuthController.setAuthMode(AuthMode.CBL_AUTHORIZATION);
            this.mLoginWorkflowSubscription = this.mAuthController.newAuthenticationWorkflow().subscribe(loginData -> {
                Log.d(TAG, "Login Workflow state changed: " + loginData.getAuthState());
                mAuthWorkflowState.setValue(loginData);

                if (loginData.getAuthState().equals(AuthState.CBL_Auth_Not_Started)) {
                    mWaitForStartLogin.startWaitForLoginStartTimer();
                } else if (loginData.getAuthState().equals(AuthState.CBL_Auth_Started)) {
                    mWaitForStartLogin.cancelWaitForLoginStartTimer();
                }
            });
        } else {
            EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
        }
    }

    /**
     * All the required setup steps are completed.
     */
    public void setupCompleted() {
        Log.d(TAG, "Setup is completed");
        mAlexaSetupController.setSetupCompleteStatus(true);
    }

    /**
     * User expressed their  intent to finish the login process.
     */
    public void userFinishedLogin() {
        Log.d(TAG, "Login Workflow finished");

        if (mUIEventListener != null) {
            mUIEventListener.loginFinished();
        }
    }

    /**
     * User expressed their intent to switch the login process.
     */
    public void userSwitchedLogin(AuthMode authMode) {
        Log.d(TAG, "User switched login authmode: " + authMode.toString());

        if (mUIEventListener != null) {
            mUIEventListener.loginSwitched(authMode);
        }
    }

    /**
     * Logout.
     */
    public void logout() {
        Log.d(TAG, "Logout.");

        mAuthController.logOut();
    }

    /**
     * Get user's first name from user identity data.
     */
    public String getUserFirstName() {
        Log.d(TAG, "Getting user's first name");

        UserIdentity userIdentity = mAuthController.getUserIdentity();
        if (userIdentity != null) {
            return userIdentity.getUserName().split(" ")[0];
        }

        return null;
    }

    /**
     * Create a runnable to wait for login workflow get started within the timeout,
     * if it failed to get login started within the timeout, we need to show login error message
     * and replace the login button text with "Try Again" based on UX requirement.
     */
    private class WaitForStartLoginRunnable implements Runnable {
        private boolean mLoginStarted = false;
        private Handler mHandler = new Handler();

        /**
         * Start scheduling a timer when start login workflow gets triggered.
         */
        void startWaitForLoginStartTimer() {
            Log.v(TAG, "Resetting WaitForLoginStart timer.");
            cancelWaitForLoginStartTimer();

            scheduleWaitForLoginStartTimer();
        }

        /**
         * Cancels the timer to wait for login workflow gets started.
         */
        void cancelWaitForLoginStartTimer() {
            Log.v(TAG, "Canceling WaitForLoginStart timer.");

            // We can safely assume login workflow get started every time we request to cancel the timer
            mLoginStarted = true;
            mHandler.removeCallbacks(this);
        }

        /**
         * Schedule a timer which will wait for login workflow gets started.
         * Elapsed time is defined with WAIT_FOR_LOGIN_START_MS constant.
         */
        private void scheduleWaitForLoginStartTimer() {
            Log.v(TAG, "Scheduling WaitForLoginStart timer.");

            // Reset login started flag to false by default
            mLoginStarted = false;
            mHandler.postDelayed(this, WAIT_FOR_LOGIN_START_MS);
        }

        /**
         * WaitForLoginStart timer is up, set CBL workflow state to CBL_Auth_Started_Failed.
         */
        @Override
        public void run() {
            Preconditions.checkArgument(!mLoginStarted,
                    "Could not set CBL workflow state to CBL_Auth_Started_Failed if login workflow is started.");

            Log.i(TAG, "WaitForLoginStart timer kicked in. Sending start login failed update.");

            AuthWorkflowData loginFailed = new AuthWorkflowData(AuthState.CBL_Auth_Start_Failed, null, null);
            mAuthWorkflowState.setValue(loginFailed);
        }
    }

    public void subscribeCBLAuthChange() {
        mAuthStatusChangeSubscription =
                mAuthController.observeAuthChangeOrLogOut()
                        .map(AuthStatus::getLoggedIn)
                        .distinctUntilChanged()
                        .subscribe(authStatus -> {
                            if (authStatus) {
                                if (mAuthController.getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)
                                        && mAuthController.isAuthenticated()) {
                                    Log.d(TAG, "CBL auth is finished, skipping the CBL login step.");
                                    mAuthWorkflowState.setValue(
                                            new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null));
                                }
                            }
                        });
    }
}
