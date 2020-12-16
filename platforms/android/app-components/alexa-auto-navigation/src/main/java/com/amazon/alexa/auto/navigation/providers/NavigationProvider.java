package com.amazon.alexa.auto.navigation.providers;

import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.StartNavigation;

/**
 * Interface for performing various map actions.
 */
public interface NavigationProvider {
    /**
     * Map provider starts navigation to the specified location.
     */
    void startNavigation(StartNavigation startNavigation);

    /**
     * Map provider starts navigation to a specified poi.
     * @param poi point of interest.
     */
    void startNavigation(PointOfInterest poi);

    /**
     * Map provider starts navigation to a specified coordinate.
     * @param coordinate point of interest.
     */
    void startNavigation(Coordinate coordinate);

    /**
     * Map provider cancels navigation.
     */
    void cancelNavigation();

    /**
     * Shows a preview of the the route to the POI
     * @param poi point of interest.
     */
    void previewRoute(PointOfInterest poi);

    /**
     * Zooms over to the POI
     * @param poi point of interest.
     */
    void zoomToPOI(PointOfInterest poi);
}
