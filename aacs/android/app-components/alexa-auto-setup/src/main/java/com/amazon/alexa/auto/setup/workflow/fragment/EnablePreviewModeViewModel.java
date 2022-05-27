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
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;
import org.jetbrains.annotations.NotNull;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * View model to handle business logic related to enabling preview mode
 */
public class EnablePreviewModeViewModel extends AndroidViewModel {
    private static final String TAG = EnablePreviewModeViewModel.class.getSimpleName();

    public final static int WAIT_FOR_LOGIN_START_MS = 15000;

    private final @NonNull AuthController mAuthController;
    private Disposable mAuthWorkflowSubscription;
    private MutableLiveData<AuthWorkflowData> mAuthWorkflowState = new MutableLiveData<>();
    private final EnablePreviewModeViewModel.WaitForStartAuthRunnable mWaitForStartLogin;

    public EnablePreviewModeViewModel(@NonNull @NotNull Application application) {
        super(application);
        AlexaApp app = AlexaApp.from(application);
        mAuthController = app.getRootComponent().getAuthController();
        mWaitForStartLogin = new EnablePreviewModeViewModel.WaitForStartAuthRunnable();
    }

    public void enablePreviewMode() {
        if (!this.mAuthController.isAuthenticated()) {
            Log.d(TAG, "Enable Preview Mode workflow starting");
            mAuthController.setAuthMode(AuthMode.AUTH_PROVIDER_AUTHORIZATION);
            this.mAuthWorkflowSubscription = this.mAuthController.newAuthenticationWorkflow().subscribe(loginData -> {
                Log.d(TAG, "Login Workflow state changed: " + loginData.getAuthState());
                if (Looper.myLooper() == Looper.getMainLooper()) {
                    mAuthWorkflowState.setValue(loginData);
                } else {
                    mAuthWorkflowState.postValue(loginData);
                }

                if (loginData.getAuthState().equals(AuthState.Auth_Provider_Auth_Started)) {
                    mWaitForStartLogin.startWaitForAuthStartTimer();
                } else if (loginData.getAuthState().equals(AuthState.Auth_Provider_Authorizing)) {
                    mWaitForStartLogin.cancelWaitForAuthStartTimer();
                }
            });
            EventBus.getDefault().post(new AuthWorkflowData(AuthState.Auth_Provider_Auth_Started, null, null));
        } else {
            EventBus.getDefault().post(new WorkflowMessage(LoginEvent.PREVIEW_MODE_ENABLED));
        }
    }

    @Override
    public void onCleared() {
        if (this.mAuthWorkflowSubscription != null) {
            this.mAuthWorkflowSubscription.dispose();
        }
    }

    /**
     * {@link LiveData} to fetch last Login Workflow state as well as
     * to listen to subsequent state changes.
     *
     * @return {@link LiveData} of {@link AuthWorkflowData}
     */
    public LiveData<AuthWorkflowData> loginWorkflowState() {
        return mAuthWorkflowState;
    }

    /**
     * A runnable with a timer that is triggered once preview mode is triggered and auth state is
     * Auth_Provider_Auth_Started. It waits until the state changes to Auth_Provider_Authorizing
     * for WAIT_FOR_LOGIN_START_MS. If Auth state changes, the timer is cancelled and execution
     * continues normally. If Auth state is stuck at Auth_Provider_Auth_Started for
     * WAIT_FOR_LOGIN_START_MS we assume that there is likely an error and we change Auth State to
     * Auth_Provider_Authorization_Error and the user then has an option to try again.
     */
    private class WaitForStartAuthRunnable implements Runnable {
        private boolean mLoginStarted = false;
        private Handler mHandler = new Handler();

        /**
         * Start scheduling a timer when start login workflow gets triggered.
         */
        void startWaitForAuthStartTimer() {
            Log.v(TAG, "Resetting WaitForLoginStart timer.");
            cancelWaitForAuthStartTimer();

            scheduleWaitForAuthStartTimer();
        }

        /**
         * Cancels the timer to wait for login workflow gets started.
         */
        void cancelWaitForAuthStartTimer() {
            Log.v(TAG, "Canceling WaitForLoginStart timer.");

            // We can safely assume login workflow get started every time we request to cancel the timer
            mLoginStarted = true;
            mHandler.removeCallbacks(this);
        }

        /**
         * Schedule a timer which will wait for login workflow gets started.
         * Elapsed time is defined with WAIT_FOR_LOGIN_START_MS constant.
         */
        private void scheduleWaitForAuthStartTimer() {
            Log.v(TAG, "Scheduling WaitForLoginStart timer.");

            // Reset login started flag to false by default
            mLoginStarted = false;
            mHandler.postDelayed(this, WAIT_FOR_LOGIN_START_MS);
        }

        /**
         * WaitForLoginStart timer is up, set CBL workflow state to Auth_Provider_Authorization_Error.
         */
        @Override
        public void run() {
            Preconditions.checkArgument(!mLoginStarted,
                    "Cannot set Preview mode workflow state to "
                            + "Auth_Provider_Authorization_Error since login workflow is started.");

            Log.i(TAG, "WaitForLoginStart timer kicked in. Sending start login failed update.");

            AuthWorkflowData loginFailed =
                    new AuthWorkflowData(AuthState.Auth_Provider_Authorization_Error, null, null);
            mAuthWorkflowState.setValue(loginFailed);
        }
    }
}
