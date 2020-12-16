package com.amazon.alexa.auto.app.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponents;
import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponentsActivator;
import com.amazon.alexa.auto.apis.app.DefaultScopedComponents;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.app.setup.AlexaSetupControllerImpl;
import com.amazon.alexa.auto.lwa.LWAAuthController;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;

/**
 * Module for providing {@link AlexaAppScope} dependencies of
 * {@link com.amazon.alexa.auto.apis.app.AlexaApp}.
 */
@Module
public class AlexaAppModule {
    /**
     * Provides {@link AlexaSetupController} implementation for Alexa App.
     *
     * @param context Android Context.
     * @return an instance of {@link AlexaSetupController}.
     */
    @Provides
    @AlexaAppScope
    public AlexaSetupController provideSetupController(WeakReference<Context> context) {
        return new AlexaSetupControllerImpl(context);
    }

    /**
     * Provides {@link AuthController} implementation for Alexa App.
     *
     * @param context Android Context.
     * @return an instance of {@link AuthController}.
     */
    @Provides
    @AlexaAppScope
    public AuthController provideAuthController(WeakReference<Context> context) {
        return new LWAAuthController(context);
    }

    /**
     * Provides {@link AlexaAppScopedComponents}.
     *
     * @return An instance of {@link AlexaAppScopedComponents}.
     */
    @Provides
    @AlexaAppScope
    public AlexaAppScopedComponents provideScopedComponents() {
        return new DefaultScopedComponents();
    }

    /**
     * Provides {@link AlexaAppScopedComponentsActivator} from implementation class.
     *
     * @param components Implementation of {@link AlexaAppScopedComponentsActivator}
     * @return An instance of {@link AlexaAppScopedComponentsActivator}.
     */
    @Provides
    @AlexaAppScope
    public AlexaAppScopedComponentsActivator provideComponentsActivator(AlexaAppScopedComponents components) {
        return (DefaultScopedComponents) components;
    }
}
