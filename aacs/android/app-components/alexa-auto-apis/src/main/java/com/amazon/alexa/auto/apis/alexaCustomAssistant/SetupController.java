package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import io.reactivex.rxjava3.core.Observable;

/**
 * An interface that allow components to interact with setup flow related to Alexa Custom Assistant.
 */

public interface SetupController extends ScopedComponent {
    /**
     * Get the current flow in progress.
     * @return The current flow in progress.
     */
    String getCurrentSetupFlow();

    /**
     * Set the current setup flow to both-assistant, Alexa only or Non-Alexa only flow.
     * @param currentFlow The current flow in progress.
     */
    void setCurrentSetupFlow(String currentFlow);
}
