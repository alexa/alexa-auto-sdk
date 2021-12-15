package com.amazon.alexa.auto.apis.module;

import android.content.Context;

/**
 * ModuleInterface can be utilized by features that require their specific business
 * logic and UI/UX to be hosted within their own modules.
 */

public interface ModuleInterface {
    /**
     * Initialize the module.
     * @param context Android context.
     */
    void initialize(Context context);

    /**
     * Un-initialize the module.
     * @param context Android context.
     */
    void uninitialize(Context context);
}
