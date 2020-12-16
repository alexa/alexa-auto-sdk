package com.amazon.alexa.auto.apis.session;

import android.view.ViewGroup;

import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponent;

import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;

/**
 * Component that handles getting the session view and vending
 * it out to components that want to inflate onto that view.
 */
public interface SessionViewController extends AlexaAppScopedComponent {
    /**
     * Returns whether there is a current voice interaction session active.
     * @return is there is a voice interaction session active.
     */
    boolean isSessionActive();

    /**
     * Sets the session view container. This should be set when a voice interaction starts
     * and then set to null when the voice interaction is finished.
     * @param viewGroup container of VIS session.
     */
    void setSessionView(ViewGroup viewGroup);

    /**
     * Returns the view container to inflate onto. Note that only one template runtime template
     * will be inflated onto this container. AutoSDK handles that contract of only allowing one
     * template to be shown. It will make sure to call ClearTemplate if a new template needs to
     * be shown.
     * @return the view container.
     */
    Optional<ViewGroup> getTemplateRuntimeViewContainer();

    /**
     * Clears any existing template runtime template.
     */
    void clearTemplate();

    /**
     * Gets an observable that tells us if a template runtime template is inflated.
     * @return an observable that informs us whether a template runtime template is inflated.
     */
    Observable<Boolean> getTemplateDisplayedObservable();
}
