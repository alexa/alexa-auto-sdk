package com.amazon.alexa.auto.navigation.dependencies;

import com.amazon.alexa.auto.navigation.receiver.LocalSearchTemplateRuntimeReceiver;
import com.amazon.alexa.auto.navigation.receiver.NavigationReceiver;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Navigation Dependencies.
 */
@Component(modules = {AACSModule.class, GoogleMapsModule.class, AndroidModule.class})
@Singleton
public interface NavigationComponent {
    /**
     * Inject dependencies for @c NavigationReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectNavigationReceiver(NavigationReceiver receiver);

    /**
     * Inject dependencies for @c POIReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectPOIReceiver(LocalSearchTemplateRuntimeReceiver receiver);
}
