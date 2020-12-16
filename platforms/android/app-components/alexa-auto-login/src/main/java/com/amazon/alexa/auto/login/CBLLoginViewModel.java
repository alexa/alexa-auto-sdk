package com.amazon.alexa.auto.login;

import android.app.Application;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppLoggedOutScopedComponent;
import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponents;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.util.Optional;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * ViewModel for @{link CBLLoginFragment}
 */
public class CBLLoginViewModel extends AndroidViewModel {
    private static final String TAG = CBLLoginViewModel.class.getSimpleName();

    private final @NonNull AuthController mAuthController;
    private final @Nullable LoginUIEventListener mUIEventListener;

    private MutableLiveData<CBLAuthWorkflowData> mCBLWorkflowState = new MutableLiveData<>();
    private Disposable mLoginWorkflowSubscription;

    /**
     * Constructor for LoginViewModel
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public CBLLoginViewModel(@NonNull Application application) {
        super(application);

        AlexaApp app = AlexaApp.from(application);

        mAuthController = app.getRootComponent().getAuthController();

        AlexaAppScopedComponents scopedComponents = app.getRootComponent().getScopedComponents();
        Optional<AlexaAppLoggedOutScopedComponent> loggedOutComponent =
                scopedComponents.getComponent(AlexaAppLoggedOutScopedComponent.class);
        mUIEventListener =
                loggedOutComponent.map(AlexaAppLoggedOutScopedComponent::getLoginUIEventListener).orElse(null);
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
     * @return {@link LiveData} of {@link CBLAuthWorkflowData}
     */
    public LiveData<CBLAuthWorkflowData> loginWorkflowState() {
        return mCBLWorkflowState;
    }

    /**
     * Start the new CBL Login workflow.
     *
     * Login workflow state changes can be listened to by subscribing to
     * {@link LiveData} fetched from {@link #loginWorkflowState()}
     */
    public void starLogin() {
        Log.d(TAG, "CBL Login Workflow starting");

        // Why not expose the RX stream directly to the UI instead of transforming
        // it to LiveData? The reason is that ViewModel can survive configuration
        // changes (display size changes, orientation changes) but View cannot.
        // On configuration changes, we do not wish to interrupt the login process.
        // Therefore ViewModel owns the subscription to auth workflow stream and
        // login UI can reconnect to login workflow any time using LiveData.
        this.mLoginWorkflowSubscription = this.mAuthController.newAuthenticationWorkflow().subscribe(loginData -> {
            Log.d(TAG, "CBL Login Workflow state changed: " + loginData.getAuthState());
            mCBLWorkflowState.setValue(loginData);
        });
    }

    /**
     * User expressed her/his intent to finish the login process.
     */
    public void userFinishedLogin() {
        Log.d(TAG, "CBL Login Workflow finished");

        if (mUIEventListener != null) {
            mUIEventListener.loginFinished();
        }
    }
}
