/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.settings;

import static com.amazon.aacsconstants.NetworkConstants.ANDROID_CONNECTIVITY_CHANGE_ACTION;
import static com.amazon.alexa.auto.apps.common.Constants.ALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.CBL_START;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.SETUP_DONE;
import static com.amazon.alexa.auto.apps.common.Constants.VOICE_ASSISTANCE;
import static com.amazon.alexa.auto.apps.common.Constants.WORK_TOGETHER;
import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.getModuleAsync;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.NavController;
import androidx.navigation.NavDestination;
import androidx.navigation.NavGraph;
import androidx.navigation.fragment.NavHostFragment;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SettingsProvider;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.Constants;
import com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil;
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.settings.databinding.SettingsActivityLayoutBinding;
import com.amazon.alexa.auto.setup.receiver.NetworkStateChangeReceiver;
import com.amazon.alexa.auto.setup.receiver.UXRestrictionsChangeReceiver;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;
import com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

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
    private AuthController mAuthController;
    private boolean isAlexaCustomAssistantEnabled;

    // State.
    private boolean mShouldExitAfterFinishingLogin = false;
    private NetworkStateChangeReceiver networkStateChangeReceiver;
    private UXRestrictionsChangeReceiver uxRestrictionsChangeReceiver;

    // holds all of the fragment layout IDs in a set
    private Set<Integer> fragmentIDSet;
    private Handler mHandler;
    private Set<String> loginEventSet;

    private long mLastLoginEventTimestamp;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        mViewBinding = SettingsActivityLayoutBinding.inflate(getLayoutInflater());

        setContentView(mViewBinding.root);

        if (mViewModel == null) {
            // If not injected by test, obtain view model from view model provider.
            mViewModel = new ViewModelProvider(this).get(SettingsActivityViewModel.class);
        }

        mApp = AlexaApp.from(this);
        mLastLoginEventTimestamp = -1;

        if (mAlexaSetupController == null) {
            mAlexaSetupController = fetchAlexaSetupController(this);
        }

        if (mAuthController == null) {
            mAuthController = fetchAuthController(this);
        }

        if (mHandler == null) {
            mHandler = new Handler();
        }

        mAlexaSetupController.observeAACSReadiness().subscribe(isAACSReady -> {
            if (isAACSReady) {
                requestAacsExtraModules();
            }
        });

        // Add any new setting/setup fragments inside this set
        fragmentIDSet = new HashSet<Integer>(Arrays.asList(R.id.navigation_fragment_login,
                R.id.navigation_fragment_startLanguageSelection, R.id.navigation_fragment_locationConsent,
                R.id.navigation_fragment_communication, R.id.navigation_fragment_authProviderAuthenticatedFinish,
                R.id.navigation_fragment_cblLoginFinish, R.id.navigation_fragment_cblLoginError,
                R.id.navigation_fragment_network, R.id.navigation_fragment_setupNotComplete,
                R.id.navigation_fragment_blockSetupDrive, R.id.navigation_fragment_naviFavoritesConsent,
                R.id.assist_app_selection));

        if (ModuleProvider.isAlexaCustomAssistantEnabled(getApplicationContext())) {
            if (mAssistantManager == null) {
                mAssistantManager = fetchAssistantManager(this);
            }
            isAlexaCustomAssistantEnabled = true;
        }
        startObservingAuthEvents();
        startObservingNavigationEvents();

        loginEventSet = new HashSet<>(Arrays.asList(LoginEvent.PREVIEW_MODE_ENABLED, LoginEvent.CBL_AUTH_FINISHED,
                VoiceAssistanceEvent.ALEXA_CBL_AUTH_FINISHED));

        mShouldExitAfterFinishingLogin = getIntent().getBooleanExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, false);
    }

    @Subscribe(threadMode = ThreadMode.ASYNC)
    public void onSetupWorkflowChange(WorkflowMessage message) {
        if (loginEventSet.contains(message.getWorkflowEvent())) {
            long currLoginEventTimestamp = System.currentTimeMillis();
            if (currLoginEventTimestamp - mLastLoginEventTimestamp
                    < FeatureDiscoveryUtil.GET_FEATURE_MIN_INTERVAL_IN_MS) {
                Log.w(TAG,
                        "Login event detected. The cache was refreshed within "
                                + FeatureDiscoveryUtil.GET_FEATURE_MIN_INTERVAL_IN_MS
                                + " ms ago. Aborting the feature request.");
            } else {
                Log.d(TAG, "Login event detected. Requesting utterances from cloud.");
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(getApplicationContext(),
                        FeatureDiscoveryConstants.Domain.GETTING_STARTED, FeatureDiscoveryConstants.EventType.SETUP);
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(getApplicationContext(),
                        FeatureDiscoveryUtil.SUPPORTED_DOMAINS, FeatureDiscoveryConstants.EventType.THINGS_TO_TRY);
            }
            mLastLoginEventTimestamp = currLoginEventTimestamp;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        EventBus.getDefault().register(this);
        IntentFilter intentFilter = new IntentFilter(ANDROID_CONNECTIVITY_CHANGE_ACTION);
        networkStateChangeReceiver = new NetworkStateChangeReceiver();
        this.registerReceiver(networkStateChangeReceiver, intentFilter);
        uxRestrictionsChangeReceiver = new UXRestrictionsChangeReceiver();
        this.registerReceiver(
                uxRestrictionsChangeReceiver, new IntentFilter(Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_ACTION));

        setupNavigationBarListener();
        setupBackButtonVisibility();

        boolean isSetupCompleted = mAlexaSetupController != null && mAlexaSetupController.isSetupCompleted();
        if (isSetupCompleted) {
            resetNavigationGraphToSettings();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        // TODO: Temporary fix to reset the CBL code whenever we quit the AACS Sample App
        if (!mAuthController.isAuthenticated())
            mAuthController.cancelLogin(null);

        this.unregisterReceiver(networkStateChangeReceiver);
        this.unregisterReceiver(uxRestrictionsChangeReceiver);
        EventBus.getDefault().unregister(this);
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
                    "Auth State changed. state:" + state + " is-current-destination-login:"
                            + currentNavDestinationIsLogin + " is-setup-completed:" + isSetupCompleted);

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
     * user's current tab bar selection when the App's back button is pressed
     */
    private void setupNavigationBarListener() {
        mViewBinding.navigationBar.navigateBackButton.setOnClickListener(view -> {
            int id = getNavigationController().getCurrentDestination().getId();
            handleBackButtonPress(id);
        });
    }

    /**
     * Handle for when the android back button is pressed. It should behave the same way as the
     * App's back button. On screens where the App back button doesn't exist, it should exit the app.
     */
    @Override
    public void onBackPressed() {
        int id = getNavigationController().getCurrentDestination().getId();
        if (disableBackButtonForResource(id)) {
            finish();
        } else {
            handleBackButtonPress(id);
        }
    }

    private void handleBackButtonPress(int id) {
        if (!getNavigationController().popBackStack()) {
            finish();
        } else if (id == R.id.navigation_fragment_login) {
            resetSetupWorkflow();
        } else if (id == R.id.navigation_fragment_cblStart || id == R.id.cbl_loading_layout
                || (isAlexaCustomAssistantEnabled
                        && id
                                == mApp.getRootComponent()
                                           .getComponent(SetupProvider.class)
                                           .get()
                                           .getSetupResId(CBL_START))) {
            // When user starts cbl login, preview mode auth gets deactivated in AACS
            // This is a temp logic until fixed in AACS layer
            mViewModel.transitionToLoggedOutState();
        }
    }

    /**
     * Hide the navigation back button on screens where it's not required
     */
    private void setupBackButtonVisibility() {
        getNavigationController().addOnDestinationChangedListener((controller, destination, arguments) -> {
            int id = destination.getId();
            mViewBinding.navigationBar.settingNavHostLayout.setVisibility(View.VISIBLE);
            if (disableBackButtonForResource(id)) {
                if (id == R.id.navigation_fragment_communication) {
                    mViewBinding.navigationBar.settingNavHostLayout.setVisibility(View.GONE);
                } else {
                    mViewBinding.navigationBar.navigateBackButton.setVisibility(View.INVISIBLE);
                }
            } else {
                mViewBinding.navigationBar.navigateBackButton.setVisibility(View.VISIBLE);
            }
        });
    }

    private boolean disableBackButtonForResource(int id) {
        return fragmentIDSet.contains(id) || disabledBackButtonOnCustomAssistantResource(id);
    }

    /**
     * Reset setup workflow to go back to the start
     */
    private void resetSetupWorkflow() {
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.stopSetupWorkflow();
                    alexaSetupWorkflowController.startSetupWorkflow(
                            getApplicationContext(), getNavigationController(), null);
                });
    }

    /**
     * Remove the back button from certain screens that are exclusive to custom assistant
     */
    private boolean disabledBackButtonOnCustomAssistantResource(int id) {
        return isAlexaCustomAssistantEnabled
                && (id == mApp.getRootComponent().getComponent(SetupProvider.class).get().getSetupResId(SETUP_DONE)
                        || id
                                == mApp.getRootComponent()
                                           .getComponent(SetupProvider.class)
                                           .get()
                                           .getSetupResId(WORK_TOGETHER)
                        || id
                                == mApp.getRootComponent()
                                           .getComponent(SetupProvider.class)
                                           .get()
                                           .getSetupWorkflowStartDestinationByKey(VOICE_ASSISTANCE)
                        || id == mApp.getRootComponent().getComponent(SetupProvider.class).get().getSetupResId(ALEXA)
                        || id
                                == mApp.getRootComponent()
                                           .getComponent(SetupProvider.class)
                                           .get()
                                           .getSetupResId(NONALEXA));
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
            SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
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
            SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
            graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
            graph.setStartDestination(setupProvider.getSetupWorkflowStartDestinationByKey(VOICE_ASSISTANCE));
        }

        overrideStartDestinationToAssistAppSelection(graph);
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
            SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
            graph = controller.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());
        }

        controller.setGraph(graph);

        Log.d(TAG, "Activate Alexa setup workflow controller");
        mApp.getRootComponent().activateScope(new AlexaSetupWorkflowControllerImpl(getApplicationContext()));
        mApp.getRootComponent()
                .getComponent(AlexaSetupWorkflowController.class)
                .ifPresent(alexaSetupWorkflowController -> {
                    alexaSetupWorkflowController.startSetupWorkflow(getApplicationContext(), controller, "CBL_Start");
                });
    }

    private void resetNavigationGraphToSettings() {
        Log.i(TAG, "resetting navigation graph to settings.");
        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.settings_navigation);

        if (mApp.getRootComponent().getComponent(SetupProvider.class).isPresent()) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider");
            SettingsProvider settingsProvider = mApp.getRootComponent().getComponent(SettingsProvider.class).get();
            graph = controller.getNavInflater().inflate(settingsProvider.getCustomSettingNavigationGraph());
        }

        overrideStartDestinationToAssistAppSelection(graph);
        controller.setGraph(graph);
    }

    private void overrideStartDestinationToAssistAppSelection(NavGraph navGraph) {
        if (navGraph == null)
            return;
        if (DefaultAssistantUtil.shouldSkipAssistAppSelectionScreen(getApplicationContext())) {
            return;
        }
        navGraph.setStartDestination(R.id.navigation_fragment_assistAppSelection);
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
            SettingsProvider settingsProvider = mApp.getRootComponent().getComponent(SettingsProvider.class).get();
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

    AlexaSetupController fetchAlexaSetupController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAlexaSetupController();
    }

    AuthController fetchAuthController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getAuthController();
    }

    @VisibleForTesting
    AssistantManager fetchAssistantManager(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        Optional<AssistantManager> assistantManager = app.getRootComponent().getComponent(AssistantManager.class);
        return assistantManager.orElse(null);
    }
}
