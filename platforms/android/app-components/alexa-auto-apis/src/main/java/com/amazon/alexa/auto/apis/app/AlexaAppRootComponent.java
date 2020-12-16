package com.amazon.alexa.auto.apis.app;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;

/**
 * Application wide Component Registry interface that is shared among
 * all modules of Alexa App. The lifetime of components under this registry
 * is bound to the lifetime of Alexa App.
 *
 * The registry enables library components such as alexa-auto-login,
 * alexa-auto-media-player etc. to fetch common App Scope dependencies
 * from main app.
 */
public interface AlexaAppRootComponent {
    /**
     * Provides the setup controller for Alexa.
     *
     * @return Alexa setup controller.
     */
    AlexaSetupController getAlexaSetupController();

    /**
     * Provides the {@link AuthController}.
     *
     * @return Auth Controller.
     */
    AuthController getAuthController();

    /**
     * Provides the {@link AlexaAppScopedComponents}
     *
     * @return Scoped Components.
     */
    AlexaAppScopedComponents getScopedComponents();

    /**
     * Provides the {@link AlexaAppScopedComponentsActivator}.
     *
     * @return {@link AlexaAppScopedComponentsActivator} to activate/deactivate
     * various scoped components.
     */
    AlexaAppScopedComponentsActivator getScopedComponentsActivator();
}
