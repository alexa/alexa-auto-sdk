package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom setup related navigation graph and resources.
 * A component can provide custom setup navigation graph that will override
 * the default setup navigation graph.
 */
public interface AlexaSetupProvider extends ScopedComponent {
    /**
     * Provide custom navigation graph from alexa setup workflow provider.
     * @return resource for the custom navigation graph.
     */
    int getCustomSetupNavigationGraph();

    /**
     * Provide start destination for the navigation graph from alexa setup workflow provider.
     * @return resource id for the start destination.
     */
    int getSetupWorkflowStartDestination();

    /**
     * Provide setup resource id by key.
     * @param key The key to map to a specific setup resource id.
     * @return The resource id. Return 0 if there is no resource id.
     */
    int getSetupResId(String key);
}
