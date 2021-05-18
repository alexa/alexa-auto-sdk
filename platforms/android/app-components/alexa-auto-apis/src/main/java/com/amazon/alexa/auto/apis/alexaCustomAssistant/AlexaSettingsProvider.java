package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom setting related navigation graph and resources.
 * A component can provide custom setting navigation graph that will override
 * the default setting navigation graph.
 */
public interface AlexaSettingsProvider extends ScopedComponent {
    /**
     * Provide custom navigation graph from alexa settings provider.
     * @return resource for the custom navigation graph.
     */
    int getCustomSettingNavigationGraph();

    /**
     * Provide start destination for the navigation graph from alexa settings provider.
     * @return resource id for the start destination.
     */
    int getSettingStartDestination();

    /**
     * Provide setting resource id by key.
     * @param key The key to map to a specific setting resource id.
     * @return The setting resource id. Return 0 if there is no resource id.
     */
    int getSettingResId(String key);
}
