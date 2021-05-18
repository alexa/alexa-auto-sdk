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
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * ViewModel for @{link CBLFragment}
 */
public class CBLViewModel extends AndroidViewModel {
    private static final String TAG = CBLViewModel.class.getSimpleName();

    public final static int WAIT_FOR_LOGIN_START_MS = 5000;

    private final @NonNull AuthController mAuthController;
    private final @Nullable LoginUIEventListener mUIEventListener;

    private MutableLiveData<AuthWorkflowData> mAuthWorkflowState = new MutableLiveData<>();
    private Disposable mLoginWorkflowSubscription;

    // State
    private final WaitForStartLoginRunnable mWaitForStartLogin;

    /**
     * Constructor for CBLViewModel
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public CBLViewModel(@NonNull Application application) {
        super(application);

        AlexaApp app = AlexaApp.from(application);

        mAuthController = app.getRootComponent().getAuthController();
        mUIEventListener = app.getRootComponent().getComponent(LoginUIEventListener.class).orElse(null);

        mWaitForStartLogin = new WaitForStartLoginRunnable();
    }

    @Override
    public void onCleared() {
        if (this.mLoginWorkflowSubscription != null) {
            this.mLoginWorkflowSubscription.dispose();
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
        Log.d(TAG, "CBL Workflow starting");

        // Make sure to cancel the login with auth provider, before we start CBL login flow.
        mAuthController.cancelLogin(AuthMode.AUTH_PROVIDER_AUTHORIZATION);

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
}
