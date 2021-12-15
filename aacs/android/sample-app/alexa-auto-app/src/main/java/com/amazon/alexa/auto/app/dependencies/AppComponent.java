package com.amazon.alexa.auto.app.dependencies;

import com.amazon.alexa.auto.app.AutoApplication;

import dagger.Component;

/**
 * Dagger root component for Alexa App.
 */
@AlexaAppScope
@Component(modules = {AndroidAppModule.class, AlexaAppModule.class, AACSModule.class})
public interface AppComponent {
    /**
     * Create Audio IO sub-component.
     *
     * @return an instance of {@link AudioIOComponent}.
     */
    AudioIOComponent createAudioIOComponent();

    /**
     * Resolve the dependencies for {@link AutoApplication}.
     *
     * @param application Object whose dependencies needs to be resolved.
     */
    void inject(AutoApplication application);
}
