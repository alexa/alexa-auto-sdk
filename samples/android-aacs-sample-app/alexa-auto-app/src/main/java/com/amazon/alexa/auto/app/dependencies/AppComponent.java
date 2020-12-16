package com.amazon.alexa.auto.app.dependencies;

import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;

import dagger.Component;

/**
 * Dagger root component for Alexa App. Also implements {@link
 * AlexaAppRootComponent} that is queried by other libraries to fetch
 * their dependencies.
 *
 * Note {@link AlexaAppRootComponent} is implemented implicitly by
 * dagger through Modules resolving these dependencies.
 */
@AlexaAppScope
@Component(modules = {AndroidAppModule.class, AlexaAppModule.class, AACSModule.class})
public interface AppComponent extends AlexaAppRootComponent {
    /**
     * Create Audio IO sub-component.
     *
     * @return an instance of {@link AudioIOComponent}.
     */
    AudioIOComponent createAudioIOComponent();
}
