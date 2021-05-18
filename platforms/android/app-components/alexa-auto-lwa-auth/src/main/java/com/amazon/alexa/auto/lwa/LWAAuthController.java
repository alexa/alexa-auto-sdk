package com.amazon.alexa.auto.lwa;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

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

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.reactivex.rxjava3.core.Observable;
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
    private CBLAuthEventReceiver mCBLEventReceiver;

    /**
     * Constructs the LWAAuthController.
     */
    public LWAAuthController(WeakReference<Context> context) {
        this.context = context;
        authStatusColdStream = BehaviorSubject.create();
        messageSender = new AACSMessageSender(context, new AACSSender());

        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), null));
        mAuthMode = AuthMode.CBL_AUTHORIZATION;

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

        subscribeCBLAuthFinishStatus();
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

        return TokenStore.getRefreshToken(contextStrong).isPresent();
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
                        case Auth_Provider_Authorized:
                            authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), null));
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

        TokenStore.resetRefreshToken(strongContext);

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

    /**
     * Subscribe CBL Auth finish event during the app life cycle, user can choose to finish CBL login
     * with phone and skip the CBL login steps from the head unit app. When user finishes CBL login with phone,
     * CBL auth finish event will be sent and captured by CBLAuthEventReceiver, and the remaining setup steps
     * will be triggered from the app.
     */
    public void subscribeCBLAuthFinishStatus() {
        if (mCBLEventReceiver == null) {
            mCBLEventReceiver = new CBLAuthEventReceiver();
            EventBus.getDefault().register(mCBLEventReceiver);
        }
    }

    class CBLAuthEventReceiver {
        @Subscribe
        public void OnReceive(AuthWorkflowData data) {
            if (data.getAuthState().equals(AuthState.CBL_Auth_Finished)) {
                setAuthState(new AuthStatus(isAuthenticated(), null));
            }
        }
    }
}
