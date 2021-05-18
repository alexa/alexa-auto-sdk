package com.amazon.alexa.auto.settings;

import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.NavController;
import androidx.navigation.NavDestination;
import androidx.navigation.NavGraph;
import androidx.navigation.fragment.NavHostFragment;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AlexaSettingsProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AlexaSetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.settings.databinding.SettingsActivityLayoutBinding;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;

/**
 * Activity to Alexa Auto Settings.
 */
public class SettingsActivity extends AppCompatActivity {
    private static final String TAG = SettingsActivity.class.getSimpleName();

    private static final String EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN = "exitAfterLogin";

    // View Model
    private SettingsActivityViewModel mViewModel;

    private SettingsActivityLayoutBinding mViewBinding;

    private AlexaApp mApp;

    // State.
    private boolean mShouldExitAfterFinishingLogin = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViewBinding = SettingsActivityLayoutBinding.inflate(getLayoutInflater());

        setContentView(mViewBinding.root);

        if (mViewModel == null) {
            // If not injected by test, obtain view model from view model provider.
            mViewModel = new ViewModelProvider(this).get(SettingsActivityViewModel.class);
        }

        mApp = AlexaApp.from(this);

        startObservingAuthEvents();
        startObservingNavigationEvents();

        mShouldExitAfterFinishingLogin = getIntent().getBooleanExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, false);
    }

    @Override
    protected void onStart() {
        super.onStart();

        setupNavigationBarListener();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    /**
     * Start observation of auth events to switch between login and settings
     * navigation destinations.
     */
    private void startObservingAuthEvents() {
        mViewModel.getAuthState().observe(this, state -> {
            boolean currentNavDestinationIsLogin = isCurrentNavigationDestinationLogin();
            Log.d(TAG,
                    "Auth State changed. state:" + state
                            + " is-current-destination-login:" + currentNavDestinationIsLogin);

            switch (state) {
                case LOGGED_IN:
                    if (mShouldExitAfterFinishingLogin) {
                        Log.i(TAG, "Finishing the main activity after login");
                        finish();
                        return;
                    }
                    if (currentNavDestinationIsLogin) {
                        resetNavigationGraphToOriginalState();
                    }
                    break;
                case LOGGED_OUT:
                    if (!currentNavDestinationIsLogin) {
                        resetNavigationGraphWithLoginAsInitialDestination();
                    }
                    break;
                case CBL_START:
                    resetNavigationGraphWithCBLAsInitialDestination();
                    break;
            }
        });
    }

    /**
     * Start observing navigation events and change the navigation bar selection
     * to reflect current navigation destination.
     */
    private void startObservingNavigationEvents() {
        NavController navController = getNavigationController();

        navController.addOnDestinationChangedListener((controller, destination, args) -> {
            mViewBinding.navigationBar.navigationTitleTxtview.setText(destination.getLabel());
        });
    }

    /**
     * Get Navigation controller for the activity.
     *
     * @return Navigation controller.
     */
    @NonNull
    @VisibleForTesting
    private NavController getNavigationController() {
        NavHostFragment navHostFragment =
                (NavHostFragment) getSupportFragmentManager().findFragmentById(R.id.settings_nav_host_fragment);
        Preconditions.checkNotNull(navHostFragment);
        return navHostFragment.getNavController();
    }

    /**
     * Setup event listener to navigate to intended destination based on
     * user's current tab bar selection.
     */
    private void setupNavigationBarListener() {
        // TODO: the actual BACK behavior will be confirmed with UX team, current solution is a place
        // holder
        mViewBinding.navigationBar.navigateBackButton.setOnClickListener(view -> {
            if (getNavigationController().getCurrentDestination().getId() == R.id.navigation_fragment_login
                    || getNavigationController().getCurrentDestination().getId() == R.id.navigation_fragment_network
                    || getNavigationController().getCurrentDestination().getId()
                            == R.id.navigation_fragment_startLanguageSelection
                    || getNavigationController().getCurrentDestination().getId()
                            == R.id.navigation_fragment_communication
                    || getNavigationController().getCurrentDestination().getId()
                            == R.id.navigation_fragment_authProviderAuthenticatedFinish
                    || getNavigationController().getCurrentDestination().getId()
                            == R.id.navigation_fragment_cblLoginFinish
                    || !getNavigationController().popBackStack()) {
                finish();
            }
        });
    }

    /**
     * Check if the current navigation destination is Login View.
     *
     * @return true if current navigation destination is Login View.
     */
    private boolean isCurrentNavigationDestinationLogin() {
        NavController navController = getNavigationController();
        NavDestination currentDestination = navController.getCurrentDestination();

        return currentDestination != null
                && (currentDestination.getId() == R.id.navigation_fragment_login
                        || currentDestination.getId() == R.id.navigation_fragment_cblLoginFinish
                        || currentDestination.getId() == R.id.navigation_fragment_authProviderAuthenticatedFinish);
    }

    /**
     * Reset the navigation graph and make the initial destination as login view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private void resetNavigationGraphWithLoginAsInitialDestination() {
        Log.i(TAG, "Switching navigation graph's destination to Login");

        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.setup_navigation);

        if (mApp.getRootComponent().getComponent(AlexaSetupProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by AlexaSetupProvider");
            AlexaSetupProvider alexaSetupProvider =
                    mApp.getRootComponent().getComponent(AlexaSetupProvider.class).get();
            graph = controller.getNavInflater().inflate(alexaSetupProvider.getCustomSetupNavigationGraph());
            graph.setStartDestination(alexaSetupProvider.getSetupWorkflowStartDestination());
        }

        controller.setGraph(graph);

        Log.d(TAG, "Activate Alexa setup workflow controller");
        mApp.getRootComponent().activateScope(new AlexaSetupWorkflowControllerImpl(getApplicationContext()));
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.startSetupWorkflow(getApplicationContext(), controller, null);
                });
    }

    /**
     * Reset the navigation graph and make the initial destination as CBL view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private void resetNavigationGraphWithCBLAsInitialDestination() {
        Log.i(TAG, "Switching navigation graph's destination to CBL view");

        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.setup_navigation);

        if (mApp.getRootComponent().getComponent(AlexaSetupProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by AlexaSetupProvider");
            AlexaSetupProvider alexaSetupProvider =
                    mApp.getRootComponent().getComponent(AlexaSetupProvider.class).get();
            graph = controller.getNavInflater().inflate(alexaSetupProvider.getCustomSetupNavigationGraph());
        }

        graph.setStartDestination(R.id.navigation_fragment_cblStart);
        controller.setGraph(graph);

        Log.d(TAG, "Activate Alexa setup workflow controller");
        mApp.getRootComponent().activateScope(new AlexaSetupWorkflowControllerImpl(getApplicationContext()));
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.startSetupWorkflow(getApplicationContext(), controller, "CBL_Start");
                });
    }

    /**
     * Reset the navigation graph to original destination as recorded in
     * main_navigation.xml.
     * Also clears up the back-stack.
     */
    private void resetNavigationGraphToOriginalState() {
        Log.i(TAG, "Resetting navigation graph to original state.");

        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.settings_navigation);
        graph.setStartDestination(R.id.navigation_fragment_alexa_settings_home);

        if (mApp.getRootComponent().getComponent(AlexaSettingsProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by AlexaSettingsProvider");
            AlexaSettingsProvider alexaSettingsProvider =
                    mApp.getRootComponent().getComponent(AlexaSettingsProvider.class).get();
            graph = controller.getNavInflater().inflate(alexaSettingsProvider.getCustomSettingNavigationGraph());
            graph.setStartDestination(alexaSettingsProvider.getSettingStartDestination());
        }

        controller.setGraph(graph);

        Log.d(TAG, "Deactivate Alexa setup workflow controller");
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.stopSetupWorkflow();
                    mApp.getRootComponent().deactivateScope(AlexaSetupWorkflowController.class);
                });
    }
}
