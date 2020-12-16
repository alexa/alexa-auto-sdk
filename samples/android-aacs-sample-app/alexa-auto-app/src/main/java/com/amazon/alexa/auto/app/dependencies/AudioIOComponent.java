package com.amazon.alexa.auto.app.dependencies;

import com.amazon.alexa.auto.app.audio.AudioIOService;

import dagger.Subcomponent;

@ServiceScope
@Subcomponent(modules = AudioIOModule.class)
public interface AudioIOComponent {
    /**
     * Inject to @{link AudioIOService}.
     *
     * @param service Object whose dependencies are resolved by dagger.
     */
    void inject(AudioIOService service);
}
