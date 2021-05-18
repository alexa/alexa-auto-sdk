package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import android.content.Context;
import android.view.ViewGroup;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom animation. A component can provide custom animation
 * that will be rendered in the layouts of the components that initialize it.
 */
public interface AnimationProvider extends ScopedComponent {
    /**
     * Initialize animation with the given context and view group.
     * @param context Android context.
     * @param viewGroup The layout that the animation will be rendered onto.
     */
    void initialize(Context context, ViewGroup viewGroup);

    /**
     * Un-initialize animation.
     */
    void uninitialize();

    /**
     * Provide custom layout from animation provider.
     * @return resource id for the custom layout.
     */
    int getCustomLayout();

    /**
     * Perform different animations depending on different signals.
     * @param signal Information for different animations to be rendered. For example, dialog state.
     */
    void doTakeOver(String signal);

    /**
     * Check if animation provider should take over the animation rendering.
     * @return true if animation provider should take over. Otherwise, return false.
     */
    boolean shouldTakeOver();

    /**
     * Prepare animation for PTT action.
     */
    void prepareAnimationForPTT();
}
