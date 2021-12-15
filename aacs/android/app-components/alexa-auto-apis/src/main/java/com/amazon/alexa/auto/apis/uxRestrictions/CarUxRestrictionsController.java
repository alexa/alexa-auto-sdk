package com.amazon.alexa.auto.apis.uxRestrictions;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import io.reactivex.rxjava3.core.Observable;

/**
 * API interface to let Alexa App know about the current car UX restrictions state and
 * to observe car UX restriction updates.
 */
public interface CarUxRestrictionsController extends ScopedComponent {
    /**
     * Get active car UX restriction.
     *
     * @return CarUxRestriction Car UX restriction.
     */
    CarUxRestriction getActiveCarUxRestriction();

    /**
     * Fetch observable that can be used to monitor the car UX restrictions changes.
     *
     * @return Observable of car UX restriction status.
     */
    Observable<CarUxRestrictionStatus> observeCarUxRestrictionChanged();

    /**
     * Register listener for car UX restrictions changes.
     */
    void registerListener();

    /**
     * Unregister listener for car UX restrictions changes.
     */
    void unregisterListener();
}
