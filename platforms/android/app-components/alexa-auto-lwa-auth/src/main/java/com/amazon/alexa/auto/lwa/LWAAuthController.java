package com.amazon.alexa.auto.lwa;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.AuthorizationHandlerInterface;
import com.amazon.alexa.auto.apis.auth.UserIdentity;
import com.amazon.alexa.auto.apps.common.util.FileUtil;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.core.ObservableEmitter;
import io.reactivex.rxjava3.core.Single;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Auth controller for Alexa authorization.
 */
public class LWAAuthController implements AuthController {
    private static final String TAG = LWAAuthController.class.getSimpleName();

    private static final String AUTH_PROVIDER_SERVICE_NAME = "alexa:auth-provider";
    private static final String AUTH_SERVICE = "service";
    private static final String AUTH_DATA = "data";

    private WeakReference<Context> context;
    private BehaviorSubject<AuthStatus> authStatusColdStream;
    private AACSMessageSender messageSender;
    private AuthMode mAuthMode;
    private ExecutorService mExecutorService;
    private Handler mMainThreadHandler;
    private AlexaClientEventReceiver mAlexaClientEventReceiver;
    private boolean mIsAlexaConnected;
    private boolean mEnableUserProfile;

    /**
     * Constructs the LWAAuthController.
     */
    public LWAAuthController(WeakReference<Context> context) {
        this.context = context;
        authStatusColdStream = BehaviorSubject.create();
        messageSender = new AACSMessageSender(context, new AACSSender());

        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));
        mAuthMode = AuthMode.CBL_AUTHORIZATION;
        mIsAlexaConnected = false;

        mExecutorService = Executors.newSingleThreadExecutor();
        mMainThreadHandler = new Handler(Looper.getMainLooper());

        getExtraAuthorizationHandlerFactoryAsync()
                .filter(Optional::isPresent)
                .map(Optional::get)
                .subscribe((authHandlerFactory) -> {
                    for (AuthorizationHandlerInterface authorizationHandlerInterface : authHandlerFactory) {
                        authorizationHandlerInterface.initialize(this.context.get());
                    }
                });

        subscribeAlexaClientConnectionStatus();

        FileUtil.readAACSConfigurationAsync(context.get()).subscribe(this::isCBLUserProfileEnabled);
    }

    @Override
    public void setAuthMode(AuthMode authMode) {
        mAuthMode = authMode;
    }

    @Override
    public AuthMode getAuthMode() {
        return mAuthMode;
    }

    @Override
    public boolean isAuthenticated() {
        Context contextStrong = context.get();
        if (contextStrong == null) {
            throw new RuntimeException("Context not valid any more.");
        }

        try {
            return TokenStore.getRefreshToken(contextStrong).isPresent();
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to get refresh token.");
            return false;
        }
    }

    @Override
    public void setAuthState(AuthStatus authStatus) {
        authStatusColdStream.onNext(authStatus);
    }

    @Override
    public Observable<AuthWorkflowData> newAuthenticationWorkflow() {
        return Observable.create(emitter -> {
            class AuthEventReceiver {
                @Subscribe
                public void OnReceive(AuthWorkflowData data) {
                    emitter.onNext(data);
                    switch (data.getAuthState()) {
                        case Auth_Provider_Auth_Started:
                            mAuthMode = AuthMode.AUTH_PROVIDER_AUTHORIZATION;
                            if (!startLoginWorkflow()) {
                                emitter.onError(new Exception("Failed to start Login workflow"));
                            }
                            break;
                        case Alexa_Client_Connected:
                            // When Alexa is connected, and the auth token has also been saved, we will send the auth
                            // finished/authorized event to subscribers.
                            if (isAuthenticated()) {
                                publishAuthFinishedStatus(emitter);
                            }
                            break;
                        case CBL_Auth_User_Identity_Saved:
                        case Auth_Provider_Token_Saved:
                            // When CBL user identity or other authorization provider's auth token is saved, we want to
                            // make sure Alexa connection is also established with the user identity or token, then we
                            // send the auth finished/authorized event to subscribers.
                            if (mIsAlexaConnected) {
                                publishAuthFinishedStatus(emitter);
                            }
                            break;
                        case CBL_Auth_Token_Saved:
                            if (mEnableUserProfile) {
                                // If user profile is enabled, we need to make sure we have also saved user identity on
                                // the device, so that we can get the user name and display it on the screen when
                                // needed.
                                Log.d(TAG,
                                        "User profile is enabled, waiting for user identity to be saved successfully.");
                            } else {
                                if (mIsAlexaConnected) {
                                    // If user profile is disabled, we can send the CBL_Auth_Finished event now.
                                    emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null));
                                    authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));
                                }
                            }
                            break;
                    }
                }
            }

            AuthEventReceiver eventReceiver = new AuthEventReceiver();
            EventBus.getDefault().register(eventReceiver);

            if (mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
                emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Not_Started, null, null));
            }
            if (!startLoginWorkflow()) {
                emitter.onError(new Exception("Failed to start Login workflow"));
            }

            emitter.setCancellable(() -> {
                EventBus.getDefault().unregister(eventReceiver);

                // When subscriber has cancelled the workflow.
                if (!isAuthenticated()) {
                    cancelLoginWorkflow();
                }
            });
        });
    }

    @Override
    public Observable<AuthStatus> observeAuthChangeOrLogOut() {
        return authStatusColdStream;
    }

    @Override
    public void logOut() {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot cancel login workflow. Context is invalid");
            return;
        }

        Log.d(TAG, "Resetting the login state");

        if (mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
            messageSender.sendMessage(Topic.CBL, Action.CBL.RESET, "");
        } else {
            try {
                String payload = new JSONStringer()
                                         .object()
                                         .key(AUTH_SERVICE)
                                         .value(AUTH_PROVIDER_SERVICE_NAME)
                                         .endObject()
                                         .toString();
                messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.LOGOUT, payload);
            } catch (JSONException e) {
                Log.e(TAG, "Fail to generate authorization logout message.");
            }
        }

        try {
            TokenStore.resetRefreshToken(strongContext);
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to reset refresh token.");
        }

        try {
            UserIdentityStore.resetUserIdentity(strongContext);
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to reset user identity.");
        }


        authStatusColdStream.onNext(new AuthStatus(false, null));
    }

    @Override
    public void cancelLogin(AuthMode mode) {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot cancel login workflow. Context is invalid");
            return;
        }

        Log.d(TAG, "Cancelling the login state");

        if (mode.equals(AuthMode.CBL_AUTHORIZATION)) {
            messageSender.sendMessage(Topic.CBL, Action.CBL.RESET, "");
        } else {
            try {
                String payload = new JSONStringer()
                                         .object()
                                         .key(AUTH_SERVICE)
                                         .value(AUTH_PROVIDER_SERVICE_NAME)
                                         .endObject()
                                         .toString();
                messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.LOGOUT, payload);
            } catch (JSONException e) {
                Log.e(TAG, "Fail to generate authorization logout message.");
            }
        }
    }

    @Override
    public UserIdentity getUserIdentity() {
        if (mAuthMode != null) {
            if (isAuthenticated() && mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
                try {
                    String userName = UserIdentityStore.getUserIdentity(context.get());
                    if (userName != null) {
                        return new UserIdentity(userName);
                    }
                    return null;
                } catch (GeneralSecurityException | IOException e) {
                    Log.e(TAG, "Fail to get user identity data.");
                    return null;
                }
            } else {
                Log.w(TAG, "Device is not authenticated or it is not for CBL login, we cannot get user identity data.");
                return null;
            }
        }
        return null;
    }

    private boolean startLoginWorkflow() {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return false;
        }

        // Then send a Start request.
        Log.i(TAG, "Starting new Login workflow");

        if (mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
            messageSender.sendMessage(Topic.CBL, Action.CBL.START, "");
        } else if (mAuthMode.equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            try {
                String payload = new JSONStringer()
                                         .object()
                                         .key(AUTH_SERVICE)
                                         .value(AUTH_PROVIDER_SERVICE_NAME)
                                         .key(AUTH_DATA)
                                         .value("")
                                         .endObject()
                                         .toString();
                messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.START_AUTHORIZATION, payload);
            } catch (JSONException e) {
                Log.e(TAG, "Fail to generate start authorization message.");
            }
        }

        return true;
    }

    private void cancelLoginWorkflow() {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return;
        }

        Log.i(TAG, "Cancelling the login workflow (if prior login workflow exists).");

        if (mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
            messageSender.sendMessage(Topic.CBL, Action.CBL.CANCEL, "");
        } else if (mAuthMode.equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            try {
                String payload = new JSONStringer()
                                         .object()
                                         .key(AUTH_SERVICE)
                                         .value(AUTH_PROVIDER_SERVICE_NAME)
                                         .endObject()
                                         .toString();
                messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.CANCEL_AUTHORIZATION, payload);
            } catch (JSONException e) {
                Log.e(TAG, "Fail to generate cancel authorization message.");
            }
        }
    }

    private Single<Optional<List<AuthorizationHandlerInterface>>> getExtraAuthorizationHandlerFactoryAsync() {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                Optional<List<AuthorizationHandlerInterface>> handlerFactoryOptional =
                        getExtraAuthorizationHandlerFactorySync(context.get());
                mMainThreadHandler.post(() -> emitter.onSuccess(handlerFactoryOptional));
            });
        });
    }

    private Optional<List<AuthorizationHandlerInterface>> getExtraAuthorizationHandlerFactorySync(Context context) {
        List<AuthorizationHandlerInterface> extraAuthorizationHandlerFactories = new ArrayList<>();
        try {
            String folderName = "auth-handler";
            String factoryKey = "authorizationHandlerFactory";
            String category = "name";
            String[] fileList = context.getAssets().list(folderName);
            for (String f : fileList) {
                InputStream is = context.getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONObject factoryKeyObj = obj.optJSONObject(factoryKey);
                    if (factoryKeyObj == null) {
                        continue;
                    }
                    String factoryName = factoryKeyObj.getString(category);
                    AuthorizationHandlerInterface instance =
                            (AuthorizationHandlerInterface) Class.forName(factoryName).newInstance();
                    extraAuthorizationHandlerFactories.add(instance);
                    Log.i(TAG, "getExtraAuthorizationHandlerFactory: load extra module:" + factoryName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "getExtraAuthorizationHandlerFactory: " + e.getMessage());
            return Optional.empty();
        }
        return Optional.of(extraAuthorizationHandlerFactories);
    }

    private void isCBLUserProfileEnabled(@NonNull String configs) {
        try {
            JSONObject config = new JSONObject(configs);
            String mUserProfileConfig = config.getJSONObject("aacs.cbl").getString("enableUserProfile");
            mEnableUserProfile = mUserProfileConfig.equals("true");
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse enableUserProfile config" + e);
        }
    }

    private void publishAuthFinishedStatus(ObservableEmitter emitter) {
        if (mAuthMode.equals(AuthMode.CBL_AUTHORIZATION)) {
            emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null));
        } else {
            emitter.onNext(
                    new AuthWorkflowData(AuthState.Auth_Provider_Authorized, null, null));
        }
        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));
    }

    /**
     * Subscribe Alexa client connection event during the app life cycle, user can choose to finish CBL login
     * with phone and skip the CBL login steps from the head unit app. When user finishes CBL login with phone,
     * and Alexa client connection state changes to CONNECTED, the Alexa client connected event will be sent and
     * captured by AlexaClientEventReceiver, and the remaining setup steps will be triggered from the app.
     */
    public void subscribeAlexaClientConnectionStatus() {
        if (mAlexaClientEventReceiver == null) {
            mAlexaClientEventReceiver = new AlexaClientEventReceiver();
            EventBus.getDefault().register(mAlexaClientEventReceiver);
        }
    }

    class AlexaClientEventReceiver {
        @Subscribe
        public void OnReceive(AuthWorkflowData data) {
            if (data.getAuthState().equals(AuthState.Alexa_Client_Connected)) {
                setAuthState(new AuthStatus(isAuthenticated(), getUserIdentity()));
                mIsAlexaConnected = true;
            } else if (data.getAuthState().equals(AuthState.Alexa_Client_Disconnected)) {
                mIsAlexaConnected = false;
            }
        }
    }
}
