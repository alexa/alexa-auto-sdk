package com.amazon.alexa.auto.apis.login;

/**
 * Listener interface to notify login UI events such as login finished
 * event, that can be used by the app to dismiss login view and launch main
 * view of the app.
 */
public interface LoginUIEventListener {
    /**
     * Invoked by Login View to notify Login Host that Login is finished.
     */
    void loginFinished();
}
