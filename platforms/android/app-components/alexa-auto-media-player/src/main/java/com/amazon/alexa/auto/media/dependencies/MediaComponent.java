package com.amazon.alexa.auto.media.dependencies;

import com.amazon.alexa.auto.media.browse.AlexaMediaBrowseService;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Media Dependencies.
 */
@Component(modules = {AndroidModule.class, AACSModule.class, MediaModule.class})
@Singleton
public interface MediaComponent {
    /**
     * Inject dependencies for @c AlexaMediaBrowseService.
     *
     * @param service Service where dependencies are injected.
     */
    void injectMediaBrowseService(AlexaMediaBrowseService service);
}
