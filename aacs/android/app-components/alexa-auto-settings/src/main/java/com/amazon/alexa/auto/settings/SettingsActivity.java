package com.amazon.alexa.auto.settings;

import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
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

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SettingsProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.settings.databinding.SettingsActivityLayoutBinding;
import com.amazon.alexa.auto.setup.receiver.NetworkStateChangeReceiver;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;

import java.util.Optional;

import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.aacsconstants.NetworkConstants.ANDROID_CONNECTIVITY_CHANGE_ACTION;
import static com.amazon.alexa.auto.apps.common.Constants.CBL_START;
import static com.amazon.alexa.auto.apps.common.Constants.SETUP_DONE;
import static com.amazon.alexa.auto.apps.common.Constants.VOICE_ASSISTANCE;

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

    private AssistantManager mAssistantManager;
    private AlexaSetupController mAlexaSetupController;
    private boolean isAlexaCustomAssistantEnabled;

    // State.
    private boolean mShouldExitAfterFinishingLogin = false;
    private NetworkStateChangeReceiver networkStateChangeReceiver;

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

        if (ModuleProvider.isAlexaCustomAssistantEnabled(getApplicationContext())) {
            if (mAssistantManager == null) {
                mAssistantManager = fetchAssistantManager(this);
            }

            if (mAlexaSetupController == null) {
                mAlexaSetupController = fetchAlexaSetupController(this);
            }

            isAlexaCustomAssistantEnabled = true;
        }

        startObservingAuthEvents();
        startObservingNavigationEvents();

        mShouldExitAfterFinishingLogin = getIntent().getBooleanExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, false);
        requestAacsExtraModules();
    }

    @Override
    protected void onStart() {
        super.onStart();

        IntentFilter intentFilter = new IntentFilter(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        networkStateChangeReceiver = new NetworkStateChangeReceiver();
        this.registerReceiver(networkStateChangeReceiver, intentFilter);

        setupNavigationBarListener();
    }

    @Override
    protected void onStop() {
        super.onStop();
        this.unregisterReceiver(networkStateChangeReceiver);
    }

    /**
     * Start observation of auth events to switch between login and settings
     * navigation destinations.
     */
    private void startObservingAuthEvents() {
        mViewModel.getAuthState().observe(this, state -> {
            Log.d(TAG, "auth state changed");
            boolean currentNavDestinationIsLogin = isCurrentNavigationDestinationLogin();
            boolean isSetupCompleted = mAlexaSetupController != null && mAlexaSetupController.isSetupCompleted();

            Log.d(TAG,
                    "Auth State changed. state:" + state
                            + " is-current-destination-login:" + currentNavDestinationIsLogin
                            + " is-setup-completed:" + isSetupCompleted);

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
                    if (!isSetupCompleted && !currentNavDestinationIsLogin) {
                        mViewModel.disableWakeWord();
                        resetNavigationGraphWithLoginAsInitialDestination();
                    }
                    break;
                case CBL_START:
                    // When user starts cbl login, preview mode auth gets deactivated in AACS
                    // This is a temp logic to enable wakeword until fixed in AACS layer
                    mViewModel.disableWakeWord();
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
            int id = getNavigationController().getCurrentDestination().getId();
            if (id == R.id.navigation_fragment_network
                    || id == R.id.navigation_fragment_startLanguageSelection
                    || id == R.id.navigation_fragment_communication
                    || id == R.id.navigation_fragment_login
                    || !getNavigationController().popBackStack()) {
                finish();
            } else if (id == R.id.navigation_fragment_setupNotComplete
                    || id == R.id.navigation_fragment_enablePreviewMode
                    || id == R.id.navigation_fragment_cblLoginError) {
                resetSetupWorkflow();
            } else if (id == R.id.navigation_fragment_cblStart
                    || id == R.id.cbl_loading_layout
                    || (isAlexaCustomAssistantEnabled &&
                        id == mApp.getRootComponent()
                                .getComponent(SetupProvider.class)
                                .get()
                                .getSetupResId(CBL_START))) {
                // When user starts cbl login, preview mode auth gets deactivated in AACS
                // This is a temp logic until fixed in AACS layer
                mViewModel.transitionToLoggedOutState();
            } else if (id == R.id.navigation_fragment_authProviderAuthenticatedFinish
                    || id == R.id.navigation_fragment_cblLoginFinish) {
                // When the user clicks on back on these screens, it should behave the same as clicking on done.
                mViewModel.loginFinished();
                resetNavigationGraphToOriginalState();
            }
        });
    }

    /**
     * Reset setup workflow to go back to the start
     */
    private void resetSetupWorkflow() {
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.stopSetupWorkflow();
                    alexaSetupWorkflowController.startSetupWorkflow(getApplicationContext(),
                            getNavigationController(), null);
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

        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.d(TAG, "Checking destination provided by SetupProvider");
            SetupProvider setupProvider =
                    mApp.getRootComponent().getComponent(SetupProvider.class).get();
            int setupDoneDestination = setupProvider.getSetupResId(SETUP_DONE);
            if (currentDestination != null && currentDestination.getId() == setupDoneDestination) {
                return true;
            }
        }

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

        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider");
            SetupProvider setupProvider =
                    mApp.getRootComponent().getComponent(SetupProvider.class).get();
            graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
            graph.setStartDestination(setupProvider.getSetupWorkflowStartDestinationByKey(VOICE_ASSISTANCE));
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

        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider");
            SetupProvider setupProvider =
                    mApp.getRootComponent().getComponent(SetupProvider.class).get();
            graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
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

        if (mApp.getRootComponent().getComponent(SettingsProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by SettingsProvider");
            SettingsProvider settingsProvider =
                    mApp.getRootComponent().getComponent(SettingsProvider.class).get();
            graph = controller.getNavInflater().inflate(settingsProvider.getCustomSettingNavigationGraph());
            graph.setStartDestination(settingsProvider.getSettingStartDestination());
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

    /**
     * Send intent to AACS requesting Service Metadata which includes extra modules. Reply handled by
     * @see AACSMetadataReceiver class
     */
    private void requestAacsExtraModules() {
        Intent intent = new Intent();
        intent.setAction(AACSConstants.IntentAction.GET_SERVICE_METADATA);
        intent.addCategory(AACSConstants.IntentCategory.GET_SERVICE_METADATA);
        intent.putExtra(AACSConstants.REPLY_TYPE, AACSConstants.REPLY_TYPE_RECEIVER);
        intent.putExtra(AACSConstants.REPLY_TO_PACKAGE, getApplicationContext().getPackageName());
        intent.putExtra(AACSConstants.REPLY_TO_CLASS, AACSMetadataReceiver.class.getName());

        getApplicationContext().sendBroadcast(intent);
        Log.d(TAG, "Sending broadcast to get AACS service metadata");
    }

    @VisibleForTesting
    AlexaSetupController fetchAlexaSetupController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAlexaSetupController();
    }

    @VisibleForTesting
    AssistantManager fetchAssistantManager(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        Optional<AssistantManager> assistantManager =
                app.getRootComponent().getComponent(AssistantManager.class);
        return assistantManager.orElse(null);
    }
}
