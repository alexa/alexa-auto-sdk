package com.amazon.alexa.auto.lwa;

import android.content.Context;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.auth.CBLAuthState;
import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Auth controller for Login With Amazon.
 */
public class LWAAuthController implements AuthController {
    private static final String TAG = LWAAuthController.class.getSimpleName();

    private WeakReference<Context> context;
    private BehaviorSubject<AuthStatus> authStatusColdStream;
    private AACSMessageSender messageSender;

    /**
     * Constructs the LWAAuthController.
     */
    public LWAAuthController(WeakReference<Context> context) {
        this.context = context;
        authStatusColdStream = BehaviorSubject.create();
        messageSender = new AACSMessageSender(context, new AACSSender());

        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), null));
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
    public Observable<CBLAuthWorkflowData> newAuthenticationWorkflow() {
        return Observable.create(emitter -> {
            class CBLEventReceiver {
                @Subscribe
                public void OnReceive(CBLAuthWorkflowData data) {
                    emitter.onNext(data);
                    if (CBLAuthState.CBL_Auth_Finished.equals(data.getAuthState())) {
                        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), null));
                    }
                }
            }

            CBLEventReceiver eventReceiver = new CBLEventReceiver();
            EventBus.getDefault().register(eventReceiver);

            emitter.onNext(new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Not_Started, null));
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

        messageSender.sendMessage(Topic.CBL, Action.CBL.RESET, "");

        TokenStore.resetRefreshToken(strongContext);

        authStatusColdStream.onNext(new AuthStatus(false, null));
    }

    private boolean startLoginWorkflow() {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return false;
        }

        // First send a cancel+logout request to get AutoSDK CBL implementation into
        // ground 0.
        cancelLoginWorkflow();
        logOut();

        // Then send a Start request.
        Log.i(TAG, "Starting new Login workflow");

        messageSender.sendMessage(Topic.CBL, Action.CBL.START, "");

        return true;
    }

    private void cancelLoginWorkflow() {
        Context strongContext = context.get();
        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return;
        }

        Log.i(TAG, "Cancelling the login workflow (if prior login workflow exists).");

        messageSender.sendMessage(Topic.CBL, Action.CBL.CANCEL, "");
    }
}
