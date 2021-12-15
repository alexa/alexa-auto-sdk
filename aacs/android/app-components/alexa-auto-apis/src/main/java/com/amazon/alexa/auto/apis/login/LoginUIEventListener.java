package com.amazon.alexa.auto.apis.login;

import com.amazon.alexa.auto.apis.app.ScopedComponent;
import com.amazon.alexa.auto.apis.auth.AuthMode;

/**
 * Listener interface to notify login UI events such as login finished
 * event, that can be used by the app to dismiss login view and launch main
 * view of the app.
 */
public interface LoginUIEventListener extends ScopedComponent {
    /**
     * Invoked by Login View to notify Login Host that Login is finished.
     */
    void loginFinished();

    /**
     * Invoked by Login View to notify Login Host that Login is switched.
     */
    void loginSwitched(AuthMode mode);
}
