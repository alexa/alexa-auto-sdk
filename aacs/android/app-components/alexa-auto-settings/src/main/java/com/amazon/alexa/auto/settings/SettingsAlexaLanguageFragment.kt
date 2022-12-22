package com.amazon.alexa.auto.settings

import android.content.ComponentName
import android.content.Intent
import android.content.res.Resources
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.util.component1
import androidx.core.util.component2
import androidx.fragment.app.Fragment
import androidx.recyclerview.selection.*
import androidx.recyclerview.widget.RecyclerView
import com.amazon.aacsconstants.AACSConstants
import com.amazon.aacsconstants.AACSPropertyConstants
import com.amazon.aacsconstants.FeatureDiscoveryConstants
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil
import com.amazon.alexa.auto.apps.common.util.LocaleUtil
import com.amazon.alexa.auto.apps.common.util.ModuleProvider
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager
import com.amazon.alexa.auto.settings.config.PreferenceKeys
import com.amazon.alexa.auto.settings.databinding.FragmentSettingsAlexaLanguageBinding
import com.amazon.alexa.auto.settings.dependencies.AndroidModule
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent
import io.reactivex.rxjava3.disposables.CompositeDisposable
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import java.lang.ref.WeakReference
import javax.inject.Inject

data class Locale(
    val localeID: String,
    val language: String,
    val country: String
)

/**
 * A fragment representing a list of Items.
 */
class SettingsAlexaLanguageFragment : Fragment() {

    private var _binding: FragmentSettingsAlexaLanguageBinding? = null
    private val binding get() = _binding!!

    @set:Inject
    lateinit var mAlexaPropertyManager: AlexaPropertyManager

    @set:Inject
    var mLocalesProvider: AlexaLocalesProvider? = null

    private lateinit var mInFlightOperations: CompositeDisposable

    private lateinit var mAdapter: SettingsAlexaLanguageAdapter
    private lateinit var mSelectionTracker: SelectionTracker<String>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        arguments?.let {
        }
        DaggerSettingsComponent.builder().androidModule(
            AndroidModule(
                requireContext()
            )
        ).build().inject(this)
        mInFlightOperations = CompositeDisposable()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSettingsAlexaLanguageBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        if (AlexaApp.from(requireContext()).rootComponent.authController.isAuthenticated) {
            binding.selectionButton.text = getString(R.string.select_language_confirm_button)
        } else {
            binding.selectionButton.text = getString(R.string.select_language_continue_button)
        }

        mInFlightOperations = CompositeDisposable()
        EventBus.getDefault().register(this)
        setupLocaleListAdapter()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        mInFlightOperations.dispose()
        EventBus.getDefault().unregister(this)
        _binding = null
    }

    private fun setupLocaleListAdapter() {
        val localeList: ArrayList<Locale> = arrayListOf()
        mInFlightOperations.add(
            mLocalesProvider!!.fetchAlexaSupportedLocales()
                .subscribe { supportedLocalesMap ->
                    supportedLocalesMap.forEach {
                        localeList.add(
                            Locale(
                                localeID = it.component1(),
                                language = it.component2().component1(),
                                country = it.component2().component2()
                            )
                        )
                    }
                    if (ModuleProvider.isAlexaCustomAssistantEnabled(requireContext())) {
                        val removedACALocales: Set<String> = setOf(
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA_FR_CA,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN_HI_IN,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US_EN_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA_EN_CA,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN_EN_IN,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_FR_FR,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR_EN_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_DE_DE,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE_EN_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_JA_JP,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_IT_IT,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP_EN_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT_EN_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_US,
                            PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES_EN_US
                        )
                        localeList.removeIf { removedACALocales.contains(it.localeID) }
                    }

                    localeList.sortBy { it.language }

                    mAdapter = SettingsAlexaLanguageAdapter(localeList)
                    binding.alexaLanguageList.adapter = mAdapter

                    mSelectionTracker = SelectionTracker.Builder(
                        "settings_alexa_language",
                        binding.alexaLanguageList,
                        AlexaLanguageItemKeyProvider(mAdapter),
                        AlexaLanguageItemDetailsLookup(binding.alexaLanguageList),
                        StorageStrategy.createStringStorage()
                    ).withSelectionPredicate(
                        SingleSelectionPredicate()
                    ).build()
                    mSelectionTracker.addObserver(
                        object : SelectionTracker.SelectionObserver<String>() {
                            override fun onSelectionChanged() {
                                super.onSelectionChanged()
                                mInFlightOperations.add(
                                    mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                                        .filter { obj -> obj.isPresent }
                                        .map { obj -> obj.get() }
                                        .subscribe { alexaLocale ->
                                            Log.d(
                                                TAG,
                                                "Selected Locale: ${mSelectionTracker.selection.first()} | Previously selected as Alexa's language: $alexaLocale"
                                            )
                                            if (mSelectionTracker.selection.first() == alexaLocale && LocaleUtil.parseAndroidLocaleListToAlexaLocaleString(AppCompatDelegate.getApplicationLocales()) == alexaLocale) {
                                                // hide button
                                                binding.selectionButton.visibility = View.GONE
                                            } else {
                                                // show button
                                                binding.selectionButton.visibility = View.VISIBLE
                                                binding.selectionButton.isClickable = true
                                                binding.selectionButton.alpha = 1.0f
                                            }
                                        }
                                )
                            }
                        }
                    )
                    mAdapter.tracker = mSelectionTracker

                    binding.selectionButton.setOnClickListener {
                        Log.d(
                            TAG,
                            "A locale has been manually selected. Disabling locale syncing with system setting."
                        )
                        val intent =
                            Intent(AACSConstants.IntentAction.DISABLE_SYNC_SYSTEM_PROPERTY_CHANGE)
                        intent.addCategory(AACSConstants.IntentCategory.SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT)
                        intent.putExtra(
                            AACSPropertyConstants.PROPERTY,
                            AACSPropertyConstants.LOCALE
                        )
                        intent.component = ComponentName(
                            AACSConstants.getAACSPackageName(
                                WeakReference(
                                    requireContext()
                                )
                            ),
                            AACSConstants.AACS_CLASS_NAME
                        )
                        AACSServiceController.checkAndroidVersionAndStartService(
                            requireContext(),
                            intent
                        )

                        val selectedLocale = mSelectionTracker.selection.first()
                        mInFlightOperations.add(
                            mAlexaPropertyManager.updateAlexaProperty(
                                AACSPropertyConstants.LOCALE,
                                selectedLocale
                            )
                                .subscribe { succeeded ->
                                    if (!succeeded) {
                                        Log.w(TAG, "Failed to update locale to: $selectedLocale")
                                    } else {
                                        Log.d(
                                            TAG,
                                            "Locale changed. Updating things to try local cache."
                                        )
                                        FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(
                                            requireContext(),
                                            FeatureDiscoveryUtil.SUPPORTED_DOMAINS,
                                            FeatureDiscoveryConstants.EventType.THINGS_TO_TRY
                                        )
                                        val currentAppLanguage =
                                            AppCompatDelegate.getApplicationLocales()[0]!!.displayName
                                        val systemLanguage =
                                            Resources.getSystem().configuration.locales[0].displayName
                                        val alexaLanguage =
                                            LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                                                selectedLocale
                                            )[0]!!.displayName
                                        Log.d(
                                            TAG,
                                            "Device Language: $currentAppLanguage | Alexa's Language: $alexaLanguage"
                                        )
                                        if (currentAppLanguage != alexaLanguage) {
                                            Log.d(
                                                TAG,
                                                "Detected that App language update is needed!"
                                            )
                                            binding.selectionButton.isClickable = false
                                            binding.selectionButton.alpha = 0.5f

                                            if (systemLanguage != alexaLanguage && AlexaApp.from(
                                                    requireContext()
                                                ).rootComponent.alexaSetupController.isSetupCompleted
                                            ) {
                                                val format1 = resources.getString(
                                                    R.string.setting_language_mismatch_content_1
                                                )
                                                val bodyString1 =
                                                    String.format(format1, systemLanguage)
                                                val format2 = resources.getString(
                                                    R.string.setting_language_mismatch_content_2
                                                )
                                                val bodyString2 =
                                                    String.format(format2, alexaLanguage)
                                                val dialog: TwoChoiceDialog =
                                                    TwoChoiceDialog.createDialog(
                                                        TwoChoiceDialog.Params(
                                                            LANGUAGE_MISMATCH_DIALOG_KEY,
                                                            resources.getString(
                                                                R.string.setting_language_mismatch_title
                                                            ),
                                                            bodyString1 + bodyString2,
                                                            resources.getString(
                                                                R.string.confirm_yesno_no
                                                            ),
                                                            resources.getString(
                                                                R.string.confirm_yes_button
                                                            )
                                                        )
                                                    )
                                                val fragmentManager =
                                                    requireActivity().supportFragmentManager
                                                dialog.show(fragmentManager, TAG)
                                            } else {
                                                Handler(Looper.getMainLooper()).postDelayed(
                                                    {
                                                        disableLocaleSyncWithSystem()
                                                        AppCompatDelegate.setApplicationLocales(
                                                            LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                                                                selectedLocale
                                                            )
                                                        )
                                                        if (AlexaApp.from(requireContext()).rootComponent.alexaSetupController.isSetupCompleted) {
                                                            binding.selectionButton.isClickable =
                                                                true
                                                            binding.selectionButton.alpha = 1.0f
                                                            binding.selectionButton.visibility =
                                                                View.GONE
                                                        }
                                                    },
                                                    REFRESH_SCREEN_WAIT_TIME.toLong()
                                                )
                                            }
                                        } else {
                                            Log.d(TAG, "No need to update app language.")
                                            binding.selectionButton.visibility = View.GONE
                                        }
                                        if (!AlexaApp.from(requireContext()).rootComponent.authController.isAuthenticated) {
                                            Log.d(
                                                TAG,
                                                "notifying workflow that language was selected..."
                                            )
                                            EventBus.getDefault().post(
                                                WorkflowMessage(
                                                    LoginEvent.LANGUAGE_SELECTION_COMPLETE_EVENT
                                                )
                                            )
                                        }
                                    }
                                }
                        )
                    }
                    mInFlightOperations.add(
                        mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                            .filter { obj -> obj.isPresent }
                            .map { obj -> obj.get() }
                            .subscribe { alexaLocale ->
                                if (AlexaApp.from(requireContext()).rootComponent.authController.isAuthenticated) {
                                    mAdapter.tracker!!.select(mAdapter.localesList.first { it.localeID == alexaLocale }.localeID)
                                }
                            }
                    )
                }
        )
    }

    @Subscribe
    fun onTwoChoiceDialogEvent(event: TwoChoiceDialog.Button1Clicked?) {
        if (event != null) {
            when (event.dialogKey) {
                LANGUAGE_MISMATCH_DIALOG_KEY -> {
                    binding.selectionButton.isClickable = true
                    binding.selectionButton.alpha = 1.0f
                }
            }
        }
    }

    @Subscribe
    fun onTwoChoiceDialogEvent(event: TwoChoiceDialog.Button2Clicked?) {
        if (event != null) {
            when (event.dialogKey) {
                LANGUAGE_MISMATCH_DIALOG_KEY -> {
                    mInFlightOperations.add(
                        mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                            .filter { obj -> obj.isPresent }
                            .map { obj -> obj.get() }
                            .subscribe { alexaLocale ->
                                disableLocaleSyncWithSystem()
                                Log.d(
                                    TAG,
                                    "Language mismatch and new alexa language, updating app language."
                                )
                                Handler(Looper.getMainLooper()).postDelayed(
                                    {
                                        disableLocaleSyncWithSystem()
                                        AppCompatDelegate.setApplicationLocales(
                                            LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                                                alexaLocale
                                            )
                                        )
                                        binding.selectionButton.isClickable =
                                            true
                                        binding.selectionButton.alpha = 1.0f
                                        binding.selectionButton.visibility =
                                            View.GONE
                                        if (!AlexaApp.from(requireContext()).rootComponent.authController.isAuthenticated) {
                                            EventBus.getDefault().post(
                                                WorkflowMessage(
                                                    LoginEvent.LANGUAGE_SELECTION_COMPLETE_EVENT
                                                )
                                            )
                                        }
                                    },
                                    REFRESH_SCREEN_WAIT_TIME.toLong()
                                )
                            }
                    )
                }
            }
        }
    }

    private fun disableLocaleSyncWithSystem() {
        Log.d(
            TAG,
            "A locale has been manually selected. Disabling locale syncing with system setting."
        )
        val intent = Intent(AACSConstants.IntentAction.DISABLE_SYNC_SYSTEM_PROPERTY_CHANGE)
        intent.addCategory(AACSConstants.IntentCategory.SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT)
        intent.putExtra(AACSPropertyConstants.PROPERTY, AACSPropertyConstants.LOCALE)
        intent.component = ComponentName(
            AACSConstants.getAACSPackageName(
                WeakReference(
                    requireContext()
                )
            ),
            AACSConstants.AACS_CLASS_NAME
        )
        AACSServiceController.checkAndroidVersionAndStartService(requireContext(), intent)
    }

    inner class SingleSelectionPredicate : SelectionTracker.SelectionPredicate<String>() {
        override fun canSetStateForKey(key: String, nextState: Boolean): Boolean = nextState
        override fun canSetStateAtPosition(position: Int, nextState: Boolean): Boolean = true
        override fun canSelectMultiple(): Boolean = false
    }

    inner class AlexaLanguageItemKeyProvider(private val adapter: SettingsAlexaLanguageAdapter) :
        ItemKeyProvider<String>(
            SCOPE_CACHED
        ) {
        override fun getPosition(key: String): Int =
            adapter.localesList.indexOfFirst { it.localeID == key }

        override fun getKey(position: Int): String = adapter.localesList[position].localeID
    }

    inner class AlexaLanguageItemDetailsLookup(private val recyclerView: RecyclerView) :
        ItemDetailsLookup<String>() {
        override fun getItemDetails(e: MotionEvent): ItemDetails<String>? {
            val view = recyclerView.findChildViewUnder(e.x, e.y)
            if (view != null) {
                return (recyclerView.getChildViewHolder(view) as SettingsAlexaLanguageAdapter.ViewHolder)
                    .getItemDetails()
            }
            return null
        }
    }

    companion object {
        @JvmStatic
        fun newInstance() =
            SettingsAlexaLanguageFragment().apply {
                arguments = Bundle().apply {
                }
            }

        private val TAG: String = SettingsAlexaLanguageFragment::class.java.canonicalName as String
        private const val REFRESH_SCREEN_WAIT_TIME = 300
        private const val LANGUAGE_MISMATCH_DIALOG_KEY = "language-mismatch"
    }
}
