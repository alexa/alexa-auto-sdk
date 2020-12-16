package com.amazon.alexa.auto.apis.auth;

import io.reactivex.rxjava3.core.Observable;

/**
 * API interface to let Alexa App know about the current auth state and
 * to start authentication related workflow such as new login, profile
 * selection etc.
 */
public interface AuthController {
    /**
     * Tells if the user has authenticated within the current app.
     *
     * @return true, if user has been authenticated with current app.
     */
    boolean isAuthenticated();

    /**
     * Provides the observable for New Authentication workflow.
     * The authentication workflow will be started only after the client
     * subscribes to the returned Observable.
     * The workflow can ba cancelled anytime by simply un-subscribing from
     * the Observable
     *
     * @return Observable to monitor the progress of authentication workflow.
     */
    Observable<CBLAuthWorkflowData> newAuthenticationWorkflow();

    /**
     * Fetch observable that can be used to monitor the auth changes.
     *
     * @return Observable of auth status.
     */
    Observable<AuthStatus> observeAuthChangeOrLogOut();

    /**
     * Logs out the currently logged in user.
     */
    void logOut();
}
