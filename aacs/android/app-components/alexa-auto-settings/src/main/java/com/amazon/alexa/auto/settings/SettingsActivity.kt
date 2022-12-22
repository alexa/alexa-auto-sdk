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
package com.amazon.alexa.auto.settings

import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import androidx.annotation.VisibleForTesting
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintSet
import androidx.core.content.ContextCompat
import androidx.core.view.isVisible
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.NavController
import androidx.navigation.NavDestination
import androidx.navigation.NavGraph
import androidx.navigation.fragment.NavHostFragment
import com.amazon.aacsconstants.AACSConstants
import com.amazon.aacsconstants.FeatureDiscoveryConstants
import com.amazon.aacsconstants.NetworkConstants
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SettingsProvider
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apis.auth.AuthController
import com.amazon.alexa.auto.apis.setup.AlexaSetupController
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog.Button1Clicked
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog.Button2Clicked
import com.amazon.alexa.auto.apps.common.Constants
import com.amazon.alexa.auto.apps.common.util.*
import com.amazon.alexa.auto.settings.config.PopupKeys
import com.amazon.alexa.auto.settings.databinding.SettingsActivityLayoutBinding
import com.amazon.alexa.auto.setup.receiver.NetworkStateChangeReceiver
import com.amazon.alexa.auto.setup.receiver.UXRestrictionsChangeReceiver
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent
import com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import java.util.*

/**
 * Activity to Alexa Auto Settings.
 */
class SettingsActivity : AppCompatActivity() {
    // View Model
    private var mViewModel: SettingsActivityViewModel? = null
    private var mViewBinding: SettingsActivityLayoutBinding? = null
    private var mApp: AlexaApp? = null
    private var mAssistantManager: AssistantManager? = null
    private var mAlexaSetupController: AlexaSetupController? = null
    private var mAuthController: AuthController? = null
    private var isAlexaCustomAssistantEnabled = false

    // State.
    private var mShouldExitAfterFinishingLogin = false
    private var networkStateChangeReceiver: NetworkStateChangeReceiver? = null
    private var uxRestrictionsChangeReceiver: UXRestrictionsChangeReceiver? = null

    // holds all of the fragment layout IDs in a set
    private var fragmentIDSet: Set<Int>? = null
    private var mHandler: Handler? = null
    private var loginEventSet: Set<String>? = null
    private var mLastLoginEventTimestamp: Long = 0

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.d(TAG, "onCreate")
        mViewBinding = SettingsActivityLayoutBinding.inflate(
            layoutInflater
        )

        setContentView(mViewBinding!!.root)
        if (mViewModel == null) {
            // If not injected by test, obtain view model from view model provider.
            mViewModel = ViewModelProvider(this)[SettingsActivityViewModel::class.java]
        }
        mApp = AlexaApp.from(this)
        mLastLoginEventTimestamp = -1
        if (mAlexaSetupController == null) {
            mAlexaSetupController = fetchAlexaSetupController(this)
        }
        if (mAuthController == null) {
            mAuthController = fetchAuthController(this)
        }
        if (mHandler == null) {
            mHandler = Handler(Looper.getMainLooper())
        }
        mAlexaSetupController!!.observeAACSReadiness().subscribe { isAACSReady: Boolean ->
            if (isAACSReady) {
                requestAacsExtraModules()
            }
        }

        // Add any new setting/setup fragments inside this set
        fragmentIDSet = HashSet(
            listOf(
                R.id.navigation_fragment_login,
                R.id.navigation_fragment_startLanguageSelection,
                R.id.navigation_fragment_locationConsent,
                R.id.navigation_fragment_communication,
                R.id.navigation_fragment_authProviderAuthenticatedFinish,
                R.id.navigation_fragment_cblLoginFinish,
                R.id.navigation_fragment_cblLoginError,
                R.id.navigation_fragment_network,
                R.id.navigation_fragment_setupNotComplete,
                R.id.navigation_fragment_blockSetupDrive,
                R.id.navigation_fragment_naviFavoritesConsent,
                R.id.blockers_layout
            )
        )

        if (ModuleProvider.isAlexaCustomAssistantEnabled(applicationContext)) {
            if (mAssistantManager == null) {
                mAssistantManager = fetchAssistantManager(this)
            }
            isAlexaCustomAssistantEnabled = true
        }
        startObservingAuthEvents()
        startObservingNavigationEvents()
        loginEventSet = HashSet(
            listOf(
                LoginEvent.PREVIEW_MODE_ENABLED,
                LoginEvent.CBL_AUTH_FINISHED,
                VoiceAssistanceEvent.ALEXA_CBL_AUTH_FINISHED
            )
        )
        mShouldExitAfterFinishingLogin = intent.getBooleanExtra(
            EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN,
            false
        )
    }

    public override fun onNewIntent(intent: Intent?) {
        super.onNewIntent(intent)
        when (intent?.getStringExtra(PopupKeys.popupType)) {
            PopupKeys.languageMismatch -> {
                // popup dialog
                Log.d(TAG, "Popping up mismatch dialog")
                val appLocale = LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                    intent.getStringExtra(PopupKeys.appLocale)!!
                )
                val alexaLocale = LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                    intent.getStringExtra(PopupKeys.alexaLocale)!!
                )

                val dialog: TwoChoiceDialog = TwoChoiceDialog.createDialog(
                    TwoChoiceDialog.Params(
                        PopupKeys.languageMismatch,
                        getString(R.string.setting_alexa_language_mismatch) + " " +
                            appLocale[0]!!.displayName,
                        String.format(
                            getString(R.string.setting_alexa_current_language),
                            alexaLocale[0]!!.displayName
                        ),
                        getString(R.string.setting_alexa),
                        getString(R.string.setting_continue)
                    )
                )
                dialog.show(supportFragmentManager, TAG)
                dialog.isCancelable = false
            }
        }
    }

    @Subscribe
    fun onTwoChoiceDialogEvent(event: Button2Clicked?) {
        if (event != null) {
            when (event.dialogKey) {
                PopupKeys.languageMismatch -> {
                    finishAffinity()
                }
            }
        }
    }

    @Subscribe
    fun onTwoChoiceDialogEvent(event: Button1Clicked?) {
        if (event != null) {
            when (event.dialogKey) {
                PopupKeys.languageMismatch -> {
                    navigationController.navigate(R.id.navigation_fragment_settings_alexa_languages)
                }
            }
        }
    }

    @Subscribe(threadMode = ThreadMode.ASYNC)
    fun onSetupWorkflowChange(message: WorkflowMessage) {
        if (loginEventSet!!.contains(message.workflowEvent)) {
            val currLoginEventTimestamp = System.currentTimeMillis()
            if (currLoginEventTimestamp - mLastLoginEventTimestamp
                < FeatureDiscoveryUtil.GET_FEATURE_MIN_INTERVAL_IN_MS
            ) {
                Log.w(
                    TAG,
                    "Login event detected. The cache was refreshed within " +
                        FeatureDiscoveryUtil.GET_FEATURE_MIN_INTERVAL_IN_MS +
                        " ms ago. Aborting the feature request."
                )
            } else {
                Log.d(TAG, "Login event detected. Requesting utterances from cloud.")
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(
                    applicationContext,
                    FeatureDiscoveryConstants.Domain.GETTING_STARTED,
                    FeatureDiscoveryConstants.EventType.SETUP
                )
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(
                    applicationContext,
                    FeatureDiscoveryUtil.SUPPORTED_DOMAINS,
                    FeatureDiscoveryConstants.EventType.THINGS_TO_TRY
                )
            }
            mLastLoginEventTimestamp = currLoginEventTimestamp
        }
    }

    override fun onStart() {
        super.onStart()
        Log.d(TAG, "onStart")

        EventBus.getDefault().register(this)
        val intentFilter = IntentFilter(NetworkConstants.ANDROID_CONNECTIVITY_CHANGE_ACTION)
        networkStateChangeReceiver = NetworkStateChangeReceiver()
        this.registerReceiver(networkStateChangeReceiver, intentFilter)
        uxRestrictionsChangeReceiver = UXRestrictionsChangeReceiver()
        this.registerReceiver(
            uxRestrictionsChangeReceiver,
            IntentFilter(Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_ACTION)
        )
        setupToolbarListeners()
        setupToolbarWidgetsVisibility()

        val isSetupCompleted =
            mAlexaSetupController != null && mAlexaSetupController!!.isSetupCompleted
        if (isSetupCompleted) {
            resetNavigationGraphToSettings()
        }
    }

    override fun onStop() {
        super.onStop()
        // TODO: Temporary fix to reset the CBL code whenever we quit the AACS Sample App
        if (!mAuthController!!.isAuthenticated) mAuthController!!.cancelLogin(null)
        unregisterReceiver(networkStateChangeReceiver)
        unregisterReceiver(uxRestrictionsChangeReceiver)
        EventBus.getDefault().unregister(this)
    }

    /**
     * Start observation of auth events to switch between login and settings
     * navigation destinations.
     */
    private fun startObservingAuthEvents() {
        mViewModel!!.authState.observe(this) { state: SettingsActivityViewModel.AuthState ->
            Log.d(TAG, "auth state changed")
            val isSetupCompleted =
                mAlexaSetupController != null && mAlexaSetupController!!.isSetupCompleted
            Log.d(TAG, "Auth State changed. state:$state; is-setup-completed:$isSetupCompleted")
            when (state) {
                SettingsActivityViewModel.AuthState.LOGGED_IN -> {
                    if (mShouldExitAfterFinishingLogin) {
                        Log.i(TAG, "Finishing the main activity after login")
                        finish()
                        return@observe
                    }
                    resetNavigationGraphToOriginalState()
                }
                SettingsActivityViewModel.AuthState.LOGGED_OUT -> if (!isSetupCompleted) {
                    mViewModel!!.disableWakeWord()
                    resetNavigationGraphWithLoginAsInitialDestination()
                }
                SettingsActivityViewModel.AuthState.CBL_START -> {
                    // When user starts cbl login, preview mode auth gets deactivated in AACS
                    // This is a temp logic to enable wakeword until fixed in AACS layer
                    mViewModel!!.disableWakeWord()
                    resetNavigationGraphWithCBLAsInitialDestination()
                }
            }
        }
    }

    /**
     * Start observing navigation events and change the navigation bar selection
     * to reflect current navigation destination.
     */
    private fun startObservingNavigationEvents() {
        val navController = navigationController

        val set = ConstraintSet()
        set.clone(mViewBinding!!.linearLayout)

        navController.addOnDestinationChangedListener { controller: NavController?, destination: NavDestination, args: Bundle? ->
            // We do not have the same level of control over where PreferenceFragments begin to render, as with other fragments
            // so we need to adjust for these PreferenceFragments accordingly.

            var connectSide: Int
            var connectId: Int

            when (destination.id) {
                // attach the content screen directly underneath the toolbar
                R.id.navigation_fragment_alexa_settings_home, R.id.navigation_fragment_aacs_preferences -> {
                    connectSide = ConstraintSet.BOTTOM
                    connectId = mViewBinding!!.toolbarBottom.id
                }
                else -> {
                    // for screens not a PreferenceFragment, we attach the top of their content page to the top of the parent container instead
                    connectSide = ConstraintSet.TOP
                    connectId = mViewBinding!!.linearLayout.id
                }
            }

            // ACA extension also defines its own pages, so we need to account for those content anchor adjustments as well
            if (mApp!!.rootComponent.getComponent(SetupProvider::class.java).isPresent) {
                for (item in setOf(Constants.ALEXA, Constants.NONALEXA, Constants.PUSH_TO_TALK)) {
                    if (destination.id == mApp!!.rootComponent
                        .getComponent(SettingsProvider::class.java)
                        .get()
                        .getSettingResId(item)
                    ) {
                        connectSide = ConstraintSet.BOTTOM
                        connectId = mViewBinding!!.toolbarBottom.id
                    }
                }
            }

            set.connect(
                mViewBinding!!.content.id,
                ConstraintSet.TOP,
                connectId,
                connectSide
            )
            set.applyTo(mViewBinding!!.linearLayout)
        }
    }

    /**
     * Get Navigation controller for the activity.
     *
     * @return Navigation controller.
     */
    @get:VisibleForTesting
    private val navigationController: NavController
        get() {
            val navHostFragment =
                supportFragmentManager.findFragmentById(R.id.content) as NavHostFragment?
            Preconditions.checkNotNull(navHostFragment)
            return navHostFragment!!.navController
        }

    /**
     * Setup event listener to navigate to intended destination based on
     * user's current tab bar selection when the App's back button is pressed
     */
    private fun setupToolbarListeners() {
        mViewBinding!!.backButton.setOnClickListener {
            val id = navigationController.currentDestination!!.id
            handleBackButtonPress(id)
        }
        mViewBinding!!.signOut.setOnClickListener {
            if (it.isVisible) {
                // Result of dialog selection are delivered through event bus to survive config
                // changes such as screen rotation. Look for onTwoChoiceDialogEvent method to
                // see how we handle the event.
                val dialog: TwoChoiceDialog = TwoChoiceDialog.createDialog(
                    TwoChoiceDialog.Params(
                        "logout",
                        it.resources.getString(R.string.logout),
                        it.resources.getString(R.string.logout_confirm),
                        it.resources.getString(R.string.confirm_yesno_no),
                        it.resources.getString(R.string.confirm_yesno_yes)
                    )
                )
                dialog.show(supportFragmentManager, TAG)
            }
        }
    }

    private fun setupToolbarWidgetsVisibility() {
        navigationController.addOnDestinationChangedListener { controller: NavController?, destination: NavDestination, arguments: Bundle? ->
            val id = destination.id
            mViewBinding!!.signOut.visibility =
                if (destination.id == R.id.navigation_fragment_alexa_settings_home) View.VISIBLE else View.GONE
            if (disableBackButtonForResource(id)) mViewBinding!!.backButton.visibility =
                View.GONE else mViewBinding!!.backButton.visibility = View.VISIBLE
        }
    }

    /**
     * Handle for when the android back button is pressed. It should behave the same way as the
     * App's back button. On screens where the App back button doesn't exist, it should exit the app.
     */
    override fun onBackPressed() {
        val id = navigationController.currentDestination!!.id
        if (disableBackButtonForResource(id)) {
            finish()
        } else {
            handleBackButtonPress(id)
        }
    }

    private fun handleBackButtonPress(id: Int) {
        if (!navigationController.popBackStack()) {
            finish()
        } else if (id == R.id.navigation_fragment_login) {
            resetSetupWorkflow()
        } else if (id == R.id.navigation_fragment_cblStart || id == R.id.cbl_loading_layout || (
            isAlexaCustomAssistantEnabled &&
                id
                == mApp!!.rootComponent
                    .getComponent(SetupProvider::class.java)
                    .get()
                    .getSetupResId(Constants.CBL_START)
            )
        ) {
            // When user starts cbl login, preview mode auth gets deactivated in AACS
            // This is a temp logic until fixed in AACS layer
            mViewModel!!.transitionToLoggedOutState()
        }
    }

    private fun disableBackButtonForResource(id: Int): Boolean {
        return fragmentIDSet!!.contains(id) || disabledBackButtonOnCustomAssistantResource(id)
    }

    /**
     * Reset setup workflow to go back to the start
     */
    private fun resetSetupWorkflow() {
        mApp!!.rootComponent
            .getComponent(AlexaSetupWorkflowController::class.java)
            .ifPresent { alexaSetupWorkflowController: AlexaSetupWorkflowController ->
                alexaSetupWorkflowController.stopSetupWorkflow()
                alexaSetupWorkflowController.startSetupWorkflow(
                    applicationContext,
                    navigationController,
                    null
                )
            }
    }

    /**
     * Remove the back button from certain screens that are exclusive to custom assistant
     */
    private fun disabledBackButtonOnCustomAssistantResource(id: Int): Boolean {
        val acaConstantsSet = setOf(
            Constants.SETUP_DONE,
            Constants.WORK_TOGETHER,
            Constants.VOICE_ASSISTANCE,
            Constants.ALEXA,
            Constants.NONALEXA
        )
        if (isAlexaCustomAssistantEnabled) {
            for (item in acaConstantsSet) {
                if (id == mApp!!
                    .rootComponent
                    .getComponent(SetupProvider::class.java)
                    .get()
                    .getSetupResId(item)
                ) {
                    return true
                }
            }
        }
        return false
    }

    /**
     * Reset the navigation graph and make the initial destination as login view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private fun resetNavigationGraphWithLoginAsInitialDestination() {
        Log.i(TAG, "Switching navigation graph's destination to Login")
        val controller = navigationController
        var graph = controller.navInflater.inflate(R.navigation.setup_navigation)
        if (mApp!!.rootComponent.getComponent(SetupProvider::class.java).isPresent) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider")
            val setupProvider = mApp!!.rootComponent.getComponent(
                SetupProvider::class.java
            ).get()
            graph = controller.navInflater.inflate(setupProvider.customSetupNavigationGraph)
            graph.setStartDestination(setupProvider.getSetupWorkflowStartDestinationByKey(Constants.VOICE_ASSISTANCE))
        }
        determineBlockers(graph)
        controller.graph = graph
        Log.d(TAG, "Activate Alexa setup workflow controller")
        mApp!!.rootComponent.activateScope(
            AlexaSetupWorkflowControllerImpl(
                applicationContext
            )
        )
        mApp!!.rootComponent
            .getComponent(AlexaSetupWorkflowController::class.java)
            .ifPresent { alexaSetupWorkflowController: AlexaSetupWorkflowController ->
                alexaSetupWorkflowController.startSetupWorkflow(
                    applicationContext,
                    controller,
                    null
                )
            }
    }

    /**
     * Reset the navigation graph and make the initial destination as CBL view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private fun resetNavigationGraphWithCBLAsInitialDestination() {
        Log.i(TAG, "Switching navigation graph's destination to CBL view")
        val controller = navigationController
        var graph = controller.navInflater.inflate(R.navigation.setup_navigation)
        if (mApp!!.rootComponent.getComponent(SetupProvider::class.java).isPresent) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider")
            val setupProvider = mApp!!.rootComponent.getComponent(
                SetupProvider::class.java
            ).get()
            graph = controller.navInflater.inflate(setupProvider.customSetupNavigationGraph)
        }
        controller.graph = graph
        Log.d(TAG, "Activate Alexa setup workflow controller")
        mApp!!.rootComponent.activateScope(
            AlexaSetupWorkflowControllerImpl(
                applicationContext
            )
        )
        mApp!!.rootComponent
            .getComponent(AlexaSetupWorkflowController::class.java)
            .ifPresent { alexaSetupWorkflowController: AlexaSetupWorkflowController ->
                alexaSetupWorkflowController.startSetupWorkflow(
                    applicationContext,
                    controller,
                    "CBL_Start"
                )
            }
    }

    private fun resetNavigationGraphToSettings() {
        Log.i(TAG, "resetting navigation graph to settings.")
        val controller = navigationController
        var graph = controller.navInflater.inflate(R.navigation.settings_navigation)
        if (mApp!!.rootComponent.getComponent(SetupProvider::class.java).isPresent) {
            Log.d(TAG, "Using navigation graph provided by SetupProvider")
            val settingsProvider = mApp!!.rootComponent.getComponent(
                SettingsProvider::class.java
            ).get()
            graph = controller.navInflater.inflate(settingsProvider.customSettingNavigationGraph)
        }
        determineBlockers(graph)
        controller.graph = graph
    }

    private fun determineBlockers(navGraph: NavGraph?) {
        var bool = true
        if (navGraph == null) return
        if (!DefaultAssistantUtil.shouldSkipAssistAppSelectionScreen(applicationContext)) {
            bool = false
        }
        if (ContextCompat.checkSelfPermission(
                applicationContext,
                android.Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            bool = false
        }

        if (!bool && mAlexaSetupController!!.isSetupCompleted) {
            navGraph.setStartDestination(R.id.navigation_fragment_blockers)
        }
    }

    /**
     * Reset the navigation graph to original destination as recorded in
     * main_navigation.xml.
     * Also clears up the back-stack.
     */
    private fun resetNavigationGraphToOriginalState() {
        Log.i(TAG, "Resetting navigation graph to original state.")
        val controller = navigationController
        var graph = controller.navInflater.inflate(R.navigation.settings_navigation)
        graph.setStartDestination(R.id.navigation_fragment_alexa_landing_page)
        if (mApp!!.rootComponent.getComponent(SettingsProvider::class.java).isPresent) {
            Log.d(TAG, "Using navigation graph provided by SettingsProvider")
            val settingsProvider = mApp!!.rootComponent.getComponent(
                SettingsProvider::class.java
            ).get()
            graph = controller.navInflater.inflate(settingsProvider.customSettingNavigationGraph)
            graph.setStartDestination(settingsProvider.settingStartDestination)
        }
        determineBlockers(graph)
        controller.graph = graph
        Log.d(TAG, "Deactivate Alexa setup workflow controller")
        mApp!!.rootComponent
            .getComponent(AlexaSetupWorkflowController::class.java)
            .ifPresent { alexaSetupWorkflowController: AlexaSetupWorkflowController ->
                alexaSetupWorkflowController.stopSetupWorkflow()
                mApp!!.rootComponent.deactivateScope(
                    AlexaSetupWorkflowController::class.java
                )
            }
    }

    /**
     * Send intent to AACS requesting Service Metadata which includes extra modules. Reply handled by
     * @see AACSMetadataReceiver class
     */
    private fun requestAacsExtraModules() {
        val intent = Intent()
        intent.action = AACSConstants.IntentAction.GET_SERVICE_METADATA
        intent.addCategory(AACSConstants.IntentCategory.GET_SERVICE_METADATA)
        intent.putExtra(AACSConstants.REPLY_TYPE, AACSConstants.REPLY_TYPE_RECEIVER)
        intent.putExtra(AACSConstants.REPLY_TO_PACKAGE, applicationContext.packageName)
        intent.putExtra(AACSConstants.REPLY_TO_CLASS, AACSMetadataReceiver::class.java.name)
        applicationContext.sendBroadcast(intent)
        Log.d(TAG, "Sending broadcast to get AACS service metadata")
    }

    private fun fetchAlexaSetupController(context: Context): AlexaSetupController {
        val app = AlexaApp.from(context)
        return app.rootComponent.alexaSetupController
    }

    private fun fetchAuthController(context: Context): AuthController {
        val app = AlexaApp.from(context)
        return app.rootComponent.authController
    }

    @VisibleForTesting
    fun fetchAssistantManager(context: Context): AssistantManager? {
        val app = AlexaApp.from(context)
        val assistantManager = app.rootComponent.getComponent(
            AssistantManager::class.java
        )
        return assistantManager.orElse(null)
    }

    companion object {
        private val TAG = SettingsActivity::class.java.simpleName
        private const val EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN = "exitAfterLogin"
    }
}
