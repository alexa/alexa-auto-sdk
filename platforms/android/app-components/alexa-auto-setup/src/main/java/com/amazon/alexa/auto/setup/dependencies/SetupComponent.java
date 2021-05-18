package com.amazon.alexa.auto.setup.dependencies;

import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;
import com.amazon.alexa.auto.setup.workflow.command.CheckLanguageCommand;
import com.amazon.alexa.auto.setup.workflow.fragment.LanguageSelectionFragment;

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
}
