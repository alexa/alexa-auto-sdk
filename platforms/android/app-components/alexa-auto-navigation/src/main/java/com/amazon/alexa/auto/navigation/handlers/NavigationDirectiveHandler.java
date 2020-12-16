package com.amazon.alexa.auto.navigation.handlers;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.NavigationMessages;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;

/**
 * Handler for Navigation directives coming from AACS.
 */
public class NavigationDirectiveHandler {
    private final static String TAG = NavigationDirectiveHandler.class.getSimpleName();
    private final NavigationProvider mNavigationProvider;

    /**
     * Constructs the Navigation commands handler.
     */
    public NavigationDirectiveHandler(NavigationProvider navigationProvider) {
        mNavigationProvider = navigationProvider;
    }

    /**
     * Handle navigation directive coming from AACS.
     *
     * @param message AACS Message.
     */
    public void handleNavigationCommand(@NonNull AACSMessage message) {
        switch (message.action) {
            case Action.Navigation.START_NAVIGATION:
                NavigationMessages.parseStartNavigationDirective(message.payload)
                        .ifPresent(mNavigationProvider::startNavigation);
                break;
            case Action.Navigation.CANCEL_NAVIGATION:
                mNavigationProvider.cancelNavigation();
                break;
            default:
                throw new RuntimeException(String.format("Unrecognized navigation command %s", message.action));
        }
    }
}
