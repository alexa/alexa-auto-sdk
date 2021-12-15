package com.amazon.alexa.auto.apis.session;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import androidx.fragment.app.Fragment;
import io.reactivex.rxjava3.core.Observable;

/**
 * Component that handles getting the session activity and vending
 * it out to components that want to inflate its voice fragment onto that activity.
 */
public interface SessionActivityController extends ScopedComponent {

    /**
     * Add voice fragment into voice session activity.
     * @param fragment voice fragment
     */
    void addFragment(Fragment fragment);

    /**
     * Remove any existing voice fragment from voice session activity.
     */
    void removeFragment();

    /**
     * Get voice fragment which is added into voice session activity.
     * @return voice fragment
     */
    Fragment getFragment();

    /**
     * Check if voice fragment has been added.
     * @return true if added
     */
    boolean isFragmentAdded();

    /**
     * Gets an observable that tells us if a voice fragment is inflated.
     * @return an observable that informs us whether a voice fragment is inflated.
     */
    Observable<Boolean> getFragmentAddedObservable();
}
