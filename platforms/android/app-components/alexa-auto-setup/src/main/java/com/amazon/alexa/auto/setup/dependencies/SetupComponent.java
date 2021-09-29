package com.amazon.alexa.auto.setup.dependencies;

import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;
import com.amazon.alexa.auto.setup.workflow.command.CheckLanguageCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLocationConsentCommand;
import com.amazon.alexa.auto.setup.workflow.fragment.CBLViewModel;
import com.amazon.alexa.auto.setup.workflow.fragment.LanguageSelectionFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.LocationConsentFragment;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Alexa Setup Dependencies.
 */
@Component(modules = {WorkflowModule.class, AndroidModule.class, ConfigModule.class})
@Singleton
public interface SetupComponent {
    /**
     * Inject dependencies for @c WorkflowController.
     *
     * @param workflowController workflow controller where dependencies are injected.
     */
    void injectWorkflowController(AlexaSetupWorkflowControllerImpl workflowController);

    /**
     * Inject dependencies for @c LanguageSelectionFragment.
     *
     * @param languageSelectionFragment language selection fragment where dependencies are injected.
     */
    void injectLanguageSettingsFragment(LanguageSelectionFragment languageSelectionFragment);

    /**
     * Inject dependencies for @c CheckLanguageCommand.
     *
     * @param checkLanguageCommand check language command where dependencies are injected.
     */
    void injectCheckLanguageCommand(CheckLanguageCommand checkLanguageCommand);

    /**
     * Inject dependencies for @c CheckLocationConsentCommand.
     *
     * @param checkLocationConsentCommand check location command where dependencies are injected.
     */
    void injectCheckLocationConsentCommand(CheckLocationConsentCommand checkLocationConsentCommand);

    /**
     * Inject dependencies for @c LocationConsentFragment.
     *
     * @param locationConsentFragment Location fragment where dependencies are injected.
     */
    void injectLocationConsentFragment(LocationConsentFragment locationConsentFragment);

    /**
     * Inject dependencies for @c CBLViewModel.
     *
     * @param cblViewModel CBL View Model where dependencies are injected.
     */
    void injectCBLViewModel(CBLViewModel cblViewModel);
}
