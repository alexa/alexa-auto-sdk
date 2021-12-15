package com.amazon.alexa.auto.templateruntime.dependencies;

import com.amazon.alexa.auto.templateruntime.receiver.TemplateRuntimeReceiver;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Template Runtime Dependencies.
 */
@Component(modules = {AACSModule.class, AndroidModule.class})
@Singleton
public interface TemplateRuntimeComponent {
    /**
     * Inject dependencies for @c TemplateRuntimeReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectTemplateRuntimeReceiver(TemplateRuntimeReceiver receiver);
}
