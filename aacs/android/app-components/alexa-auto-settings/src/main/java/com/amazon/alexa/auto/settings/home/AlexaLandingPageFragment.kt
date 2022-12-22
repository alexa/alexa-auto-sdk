package com.amazon.alexa.auto.settings.home

import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.TypedArray
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.PagerSnapHelper
import com.amazon.aacsconstants.AACSPropertyConstants
import com.amazon.aacsconstants.AACSPropertyConstants.GEOLOCATION_ENABLED
import com.amazon.aacsconstants.FeatureDiscoveryConstants
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apis.auth.AuthController
import com.amazon.alexa.auto.apis.auth.AuthMode
import com.amazon.alexa.auto.apis.communication.ContactsController
import com.amazon.alexa.auto.app.common.ui.CirclePageIndicatorDecoration
import com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil
import com.amazon.alexa.auto.apps.common.util.ModuleProvider
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager
import com.amazon.alexa.auto.comms.ui.Constants
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository
import com.amazon.alexa.auto.settings.R
import com.amazon.alexa.auto.settings.SettingsActivityViewModel
import com.amazon.alexa.auto.settings.databinding.FragmentAlexaLandingPageBinding
import com.amazon.alexa.auto.settings.dependencies.AndroidModule
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent
import com.amazon.alexa.auto.setup.workflow.model.UserConsent
import io.reactivex.rxjava3.disposables.CompositeDisposable
import java.util.*
import java.util.function.Consumer
import javax.inject.Inject

enum class Priority {
    PROMOTION,
    ISSUE
}

typealias Index = Int

data class UpperContent(
    var priority: Priority,
    var rank: Int,
    var primaryText: String,
    var secondaryText: String,
    var drawable: Drawable?,
    var btnText: String,
    var btnCallback: (View) -> Unit
)

class AlexaLandingPageFragment : Fragment() {
    private var _binding: FragmentAlexaLandingPageBinding? = null

    @Inject
    lateinit var mAlexaPropertyManager: AlexaPropertyManager

    private val binding get() = _binding!!
    private val TAG = AlexaLandingPageFragment::class.java.canonicalName
    private val viewModel: SettingsActivityViewModel by activityViewModels()
    private lateinit var mAuthController: AuthController
    private var index: Index = 0
    private lateinit var mInFlightOperations: CompositeDisposable

    private lateinit var mAdapter: AlexaLandingPageCarouselAdapter

    private lateinit var mGettingStartedUtterances: Set<String>
    private lateinit var mLocationUtterances: Set<String>
    private lateinit var mContactsUtterances: Set<String>

    private val locationPermissionLauncher =
        registerForActivityResult(
            ActivityResultContracts.RequestMultiplePermissions()
        ) { permResultMap: Map<String, Boolean> ->
            val id =
                mAdapter.carouselList.indexOfFirst { it.primaryText == getString(R.string.lp_carousel_location_primary) }
            val uc: UpperContent =
                mAdapter.carouselList[id]

            if (permResultMap.containsValue(false)) {
                Log.e(TAG, "A location permission was not granted.")
            } else {
                Log.d(TAG, "All required location permissions were granted.")
                uc.rank = 50
                uc.secondaryText = resources.getString(
                    R.string.try_saying,
                    mLocationUtterances.iterator().next()
                )
                uc.btnText = ""
                uc.btnCallback = {}
                mAdapter.notifyItemChanged(id)
            }
        }

    private val contactsPermissionLauncher =
        registerForActivityResult(
            ActivityResultContracts.RequestPermission()
        ) { isGranted: Boolean ->
            val id =
                mAdapter.carouselList.indexOfFirst { it.primaryText == getString(R.string.lp_carousel_contacts_primary) }
            val uc: UpperContent =
                mAdapter.carouselList[id]

            if (!isGranted) {
                Log.e(TAG, "The contacts upload permission was not granted.")
            } else {
                Log.d(TAG, "Contacts upload permission was granted.")
                uc.rank = 51
                uc.secondaryText = resources.getString(
                    R.string.try_saying,
                    mContactsUtterances.iterator().next()
                )
                uc.btnText = ""
                uc.btnCallback = {}
                mAdapter.notifyItemChanged(id)
            }
        }

    private val smsPermissionLauncher =
        registerForActivityResult(
            ActivityResultContracts.RequestMultiplePermissions()
        ) { permResultMap: Map<String, Boolean> ->
            val id =
                mAdapter.carouselList.indexOfFirst { it.primaryText == resources.getString(R.string.lp_carousel_sms_primary) }
            val uc: UpperContent =
                mAdapter.carouselList[id]

            if (permResultMap.containsValue(false)) {
                Log.e(TAG, "SMS permissions were not all granted.")
            } else {
                Log.d(TAG, "All SMS permissions were granted.")
                uc.rank = 52
                uc.secondaryText = resources.getString(
                    R.string.try_saying,
                    mContactsUtterances.iterator().next()
                )
                uc.btnText = ""
                uc.btnCallback = {}
                mAdapter.notifyItemChanged(id)
            }
        }

    companion object {
        fun newInstance() = AlexaLandingPageFragment()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mAuthController = AlexaApp.from(requireContext()).rootComponent.authController
        DaggerSettingsComponent.builder().androidModule(
            AndroidModule(
                requireContext()
            )
        ).build().inject(this)
    }

    override fun onResume() {
        super.onResume()
        Log.d(TAG, "resumed")
        // We need to clear the ConnectedBTRepository if bluetooth is disabled meaning there are no device
        // connected
        if (!(requireContext().getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager).adapter.isEnabled) {
            Log.d(TAG, "This Bluetooth is disabled")
            ConnectedBTDeviceRepository.getInstance(context).nukeTable()
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentAlexaLandingPageBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        mInFlightOperations = CompositeDisposable()

        binding.settingsButton.setOnClickListener {
            findNavController().navigate(R.id.navigation_fragment_alexa_settings_home)
        }

        mInFlightOperations.add(
            mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                .filter { obj: Optional<String?> -> obj.isPresent }
                .map { obj: Optional<String?> -> obj.get() }
                .doOnError { err -> Log.e(TAG, err.message!!) }
                .subscribe { alexaLocale: String? ->
                    mGettingStartedUtterances = FeatureDiscoveryUtil.getFeaturesByTag(
                        requireContext(),
                        FeatureDiscoveryUtil.createTag(
                            alexaLocale,
                            FeatureDiscoveryConstants.Domain.GETTING_STARTED,
                            FeatureDiscoveryConstants.EventType.THINGS_TO_TRY
                        )
                    )
                    if (mGettingStartedUtterances.isEmpty()) {
                        val tmp: TypedArray =
                            resources.obtainTypedArray(R.array.things_to_try_utterances)
                        mGettingStartedUtterances = tmp.getTextArray(0).toSet() as Set<String>
                        tmp.recycle()
                    }
                    mLocationUtterances = FeatureDiscoveryUtil.getFeaturesByTag(
                        requireContext(),
                        FeatureDiscoveryUtil.createTag(
                            alexaLocale,
                            FeatureDiscoveryConstants.Domain.NAVIGATION,
                            FeatureDiscoveryConstants.EventType.THINGS_TO_TRY
                        )
                    )
                    if (mLocationUtterances.isEmpty()) {
                        val tmp: TypedArray =
                            resources.obtainTypedArray(R.array.things_to_try_utterances)
                        mLocationUtterances = tmp.getTextArray(7).toSet() as Set<String>
                        tmp.recycle()
                    }
                    mContactsUtterances = FeatureDiscoveryUtil.getFeaturesByTag(
                        requireContext(),
                        FeatureDiscoveryUtil.createTag(
                            alexaLocale,
                            FeatureDiscoveryConstants.Domain.COMMS,
                            FeatureDiscoveryConstants.EventType.THINGS_TO_TRY
                        )
                    )
                    if (mContactsUtterances.isEmpty()) {
                        val tmp: TypedArray =
                            resources.obtainTypedArray(R.array.things_to_try_utterances)
                        mContactsUtterances = tmp.getTextArray(7).toSet() as Set<String>
                        tmp.recycle()
                    }

                    setupWakewordToggle()
                    configureLowerContent()

                    mAdapter = AlexaLandingPageCarouselAdapter(
                        arrayListOf(
                            UpperContent(
                                priority = Priority.PROMOTION,
                                rank = 4,
                                primaryText = resources.getString(R.string.lp_carousel_ttt_primary),
                                secondaryText = "",
                                drawable = AppCompatResources.getDrawable(
                                    requireContext(),
                                    R.drawable.information
                                ),
                                btnText = resources.getString(R.string.cta_label_things_to_try),
                                btnCallback = {
                                    findNavController().navigate(R.id.navigation_fragment_alexa_settings_things_to_try)
                                }
                            ),

                            UpperContent(
                                priority = Priority.ISSUE,
                                rank = 5,
                                primaryText = resources.getString(R.string.lp_carousel_previewmode_primary),
                                secondaryText = resources.getString(R.string.lp_carousel_previewmode_secondary),
                                drawable = AppCompatResources.getDrawable(
                                    requireContext(),
                                    R.drawable.information
                                ),
                                btnText = resources.getString(R.string.cta_label_sign_in),
                                btnCallback = {
                                    viewModel.transitionToSwitchCBLStartState()
                                }
                            ),
                            UpperContent(
                                priority = Priority.ISSUE,
                                rank = -1,
                                primaryText = resources.getString(R.string.lp_carousel_location_primary),
                                secondaryText = resources.getString(
                                    R.string.try_saying,
                                    mLocationUtterances.iterator().next()
                                ),
                                drawable = AppCompatResources.getDrawable(
                                    requireContext(),
                                    R.drawable.information
                                ),
                                btnText = "",
                                btnCallback = {}
                            )
                        )
                    )
                    index = 0
                    if (mAuthController.authMode != AuthMode.AUTH_PROVIDER_AUTHORIZATION) {
                        mAdapter.carouselList.removeAt(1)
                        mAdapter.notifyItemRemoved(1)
                        mAdapter.notifyItemRangeChanged(1, mAdapter.carouselList.size)
                    }
                    setupInvocationItem()
                    setupLocationItem()
                    setupCommsItems()

                    binding.upperContentRecyclerview.layoutManager =
                        LinearLayoutManager(context, LinearLayoutManager.HORIZONTAL, false)
                    binding.upperContentRecyclerview.adapter = mAdapter
                    PagerSnapHelper().attachToRecyclerView(binding.upperContentRecyclerview)
                    binding.upperContentRecyclerview.addItemDecoration(CirclePageIndicatorDecoration())

                    binding.leftButton.setOnClickListener {
                        if (index == 0) {
                            index = mAdapter.carouselList.lastIndex
                            binding.upperContentRecyclerview.scrollToPosition(index)
                        } else {
                            index--
                            binding.upperContentRecyclerview.smoothScrollToPosition(index)
                        }
                    }

                    binding.rightButton.setOnClickListener {
                        if (index == mAdapter.carouselList.lastIndex) {
                            index = 0
                            binding.upperContentRecyclerview.scrollToPosition(index)
                        } else {
                            index++
                            binding.upperContentRecyclerview.smoothScrollToPosition(index)
                        }
                    }
//                    binding.upperContentRecyclerview.scrollToPosition(0)
                    binding.upperContentRecyclerview.jumpDrawablesToCurrentState()
                }
        )
    }

    private fun setupCommsItems() {
        val connectedDevices =
            ConnectedBTDeviceRepository.getInstance(context).connectedDevices
        connectedDevices.observe(viewLifecycleOwner) { connectedDevicesList ->
            if (connectedDevicesList != null && connectedDevicesList.size > 0 && mAuthController.authMode != AuthMode.AUTH_PROVIDER_AUTHORIZATION) {
                val primaryDevice = connectedDevicesList[connectedDevicesList.size - 1]
                // only add the comms items one time, if they don't already exist in the carousel
                if (!mAdapter.carouselList.any { it.primaryText == resources.getString(R.string.lp_carousel_contacts_primary) }) {
                    setupContactsItem(primaryDevice)
                }
                if (!mAdapter.carouselList.any { it.primaryText == resources.getString(R.string.lp_carousel_sms_primary) }) {
                    setupSMSItem(primaryDevice)
                }
            } else {
                Log.d(
                    TAG,
                    "No primary device found. Removing carousel items if they exist..."
                )
                val textSet = mutableSetOf(resources.getString(R.string.lp_carousel_sms_primary))
                if (mAuthController.authMode != AuthMode.AUTH_PROVIDER_AUTHORIZATION) {
                    setupContactsItem(null)
                } else {
                    textSet.add(resources.getString(R.string.lp_carousel_contacts_primary))
                }
                mAdapter.carouselList.removeIf { item ->
                    textSet.contains(item.primaryText)
                }
                mAdapter.carouselList.sortBy { it.rank }
                mAdapter.notifyDataSetChanged()
            }
        }
    }

    private fun setupInvocationItem() {
        val it: UpperContent = mAdapter.carouselList[0]
        val pttAvailable: Boolean =
            DefaultAssistantUtil.isAlexaAppDefaultAssist(requireContext())

        mInFlightOperations.add(
            mAlexaPropertyManager.getAlexaPropertyBoolean(AACSPropertyConstants.WAKEWORD_SUPPORTED)
                .filter { obj: Optional<Boolean?> -> obj.isPresent }
                .map { obj: Optional<Boolean?> -> obj.get() }
                .doOnError { err -> Log.e(TAG, err.message!!) }
                .subscribe { wakeWordSupported: Boolean? ->
                    if (wakeWordSupported == true) {
                        if (pttAvailable) {
                            it.secondaryText =
                                resources.getString(R.string.lp_carousel_ttt_secondary_ww_ptt)
                        } else {
                            it.secondaryText =
                                resources.getString(R.string.lp_carousel_ttt_secondary_ww)
                        }
                    } else {
                        if (pttAvailable) {
                            it.secondaryText =
                                resources.getString(R.string.lp_carousel_ttt_secondary_ptt)
                        }
                    }
                    mAdapter.notifyItemChanged(0)
                }
        )
    }

    private fun setupLocationItem() {
        if (!ModuleProvider.containsModule(
                requireContext(),
                ModuleProvider.ModuleName.GEOLOCATION
            )
        ) {
            Log.e(
                TAG,
                "Tried to setup location item in the landing page, but the module is not available..."
            )
            return
        }
        val id =
            mAdapter.carouselList.indexOfFirst { it.primaryText == getString(R.string.lp_carousel_location_primary) }
        val uc: UpperContent =
            mAdapter.carouselList[id]

        val devicePermission: Boolean = ContextCompat.checkSelfPermission(
            requireContext(),
            android.Manifest.permission.ACCESS_FINE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED && ContextCompat.checkSelfPermission(
            requireContext(),
            android.Manifest.permission.ACCESS_COARSE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED

        mInFlightOperations.add(
            mAlexaPropertyManager.getAlexaProperty(GEOLOCATION_ENABLED)
                .filter { obj: Optional<String?> -> obj.isPresent }
                .map { obj: Optional<String?> -> obj.get() }
                .doOnError { err -> Log.e(TAG, err.message!!) }
                .subscribe { currentConsent: String ->
                    Log.d(TAG, "Geolocation current value:$currentConsent")
                    if (currentConsent == UserConsent.ENABLED.value) {
                        if (!devicePermission) {
                            Log.e(
                                TAG,
                                "device permission not given for accessing location. need to surface system permission request"
                            )
                            uc.rank = 7
                            uc.secondaryText =
                                resources.getString(R.string.lp_carousel_location_secondary_system)
                            uc.btnText =
                                resources.getString(R.string.cta_label_allow)
                            uc.btnCallback = {
                                locationPermissionLauncher.launch(
                                    arrayOf(
                                        android.Manifest.permission.ACCESS_COARSE_LOCATION,
                                        android.Manifest.permission.ACCESS_FINE_LOCATION
                                    )
                                )
                            }
                            val newID =
                                mAdapter.carouselList.indexOfFirst { it.primaryText == getString(R.string.lp_carousel_location_primary) }
                            mAdapter.notifyItemChanged(newID)
                        }
                    } else {
                        uc.rank = 8
                        uc.secondaryText =
                            resources.getString(R.string.lp_carousel_location_secondary_alexa_system)
                        uc.btnText =
                            resources.getString(R.string.cta_label_enable)
                        uc.btnCallback = {
                            mAlexaPropertyManager.updateAlexaProperty(
                                GEOLOCATION_ENABLED,
                                UserConsent.ENABLED.value
                            )
                                .doOnSuccess {
                                    if (!devicePermission) {
                                        locationPermissionLauncher.launch(
                                            arrayOf(
                                                android.Manifest.permission.ACCESS_COARSE_LOCATION,
                                                android.Manifest.permission.ACCESS_FINE_LOCATION
                                            )
                                        )
                                    } else {
                                        uc.secondaryText = resources.getString(
                                            R.string.try_saying,
                                            mLocationUtterances.iterator().next()
                                        )
                                        uc.btnText = ""
                                        uc.btnCallback = {}
                                        val newID = mAdapter.carouselList.indexOfFirst {
                                            it.primaryText == getString(R.string.lp_carousel_location_primary)
                                        }
                                        mAdapter.notifyItemChanged(newID)
                                    }
                                }.doOnError {
                                    Log.e(TAG, it.message.toString())
                                }.subscribe()
                            val newID =
                                mAdapter.carouselList.indexOfFirst { it.primaryText == getString(R.string.lp_carousel_location_primary) }
                            mAdapter.notifyItemChanged(newID)
                        }
                    }
                }
        )
    }

    private fun setupContactsItem(primaryDevice: ConnectedBTDevice?) {
        val uc = UpperContent(
            priority = Priority.ISSUE,
            rank = -1,
            primaryText = resources.getString(R.string.lp_carousel_contacts_primary),
            secondaryText = "",
            drawable = AppCompatResources.getDrawable(
                requireContext(),
                R.drawable.information
            ),
            btnText = "",
            btnCallback = {}
        )

        if (primaryDevice == null) {
            uc.rank = 11
            uc.secondaryText = resources.getString(R.string.lp_carousel_contacts_secondary_no_phone)
            uc.btnText = resources.getString(R.string.cta_label_connect_phone)
            uc.btnCallback = {
                val intent = Intent(Settings.ACTION_BLUETOOTH_SETTINGS)
                intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
                requireContext().startActivity(intent)
            }
        } else {
            val devicePermission: Boolean = ContextCompat.checkSelfPermission(
                requireContext(),
                android.Manifest.permission.READ_CONTACTS
            ) == PackageManager.PERMISSION_GRANTED

            if (primaryDevice.contactsUploadPermission.equals(Constants.CONTACTS_PERMISSION_YES)) {
                if (!devicePermission) {
                    Log.e(
                        TAG,
                        "device permission not given for contact consent. need to surface system permission request"
                    )
                    uc.rank = 10
                    uc.secondaryText =
                        resources.getString(R.string.lp_carousel_contacts_secondary_system)
                    uc.btnText =
                        resources.getString(R.string.cta_label_allow)
                    uc.btnCallback =
                        { contactsPermissionLauncher.launch(android.Manifest.permission.READ_CONTACTS) }
                } else {
                    uc.rank = 51
                    uc.secondaryText = resources.getString(
                        R.string.try_saying,
                        mContactsUtterances.iterator()
                            .next()
                    )
                    uc.btnText = ""
                    uc.btnCallback = {}
                }
            } else {
                uc.rank = 9
                uc.secondaryText =
                    resources.getString(
                        R.string.lp_carousel_contacts_secondary_alexa_system,
                        primaryDevice.deviceName
                    )
                uc.btnText =
                    resources.getString(R.string.cta_label_enable)
                uc.btnCallback = {
                    AlexaApp.from(requireContext()).rootComponent
                        .getComponent(ContactsController::class.java)
                        .ifPresent(
                            Consumer { contactsController: ContactsController ->
                                contactsController.setContactsUploadPermission(
                                    primaryDevice.deviceAddress,
                                    Constants.CONTACTS_PERMISSION_YES
                                )
                                contactsController.uploadContacts(primaryDevice.deviceAddress)
                            }.andThen {
                                if (!devicePermission) {
                                    contactsPermissionLauncher.launch(android.Manifest.permission.READ_CONTACTS)
                                } else {
                                    uc.rank = 51
                                    uc.secondaryText = resources.getString(
                                        R.string.try_saying,
                                        mContactsUtterances.iterator()
                                            .next()
                                    )
                                    uc.btnText = ""
                                    uc.btnCallback = {}
                                }
                                mAdapter.notifyDataSetChanged()
                            }
                        )
                }
            }
        }

        mAdapter.carouselList.add(uc)
        mAdapter.carouselList.sortBy { it.rank }
        mAdapter.notifyDataSetChanged()
    }

    private fun setupSMSItem(primaryDevice: ConnectedBTDevice) {
        val uc =
            UpperContent(
                priority = Priority.ISSUE,
                rank = -1,
                primaryText = resources.getString(R.string.lp_carousel_sms_primary),
                secondaryText = "",
                drawable = AppCompatResources.getDrawable(
                    requireContext(),
                    R.drawable.information
                ),
                btnText = "",
                btnCallback = {}
            )

        val devicePermission: Boolean = ContextCompat.checkSelfPermission(
            requireContext(),
            android.Manifest.permission.READ_SMS
        ) == PackageManager.PERMISSION_GRANTED && ContextCompat.checkSelfPermission(
            requireContext(),
            android.Manifest.permission.SEND_SMS
        ) == PackageManager.PERMISSION_GRANTED && ContextCompat.checkSelfPermission(
            requireContext(),
            android.Manifest.permission.RECEIVE_SMS
        ) == PackageManager.PERMISSION_GRANTED

        if (primaryDevice.messagingPermission.equals(Constants.CONTACTS_PERMISSION_YES)) {
            if (!devicePermission) {
                Log.e(
                    TAG,
                    "device permission not given for sms. need to surface system permission request"
                )
                uc.rank = 10
                uc.secondaryText =
                    resources.getString(R.string.lp_carousel_sms_secondary_system)
                uc.btnText =
                    resources.getString(R.string.cta_label_allow)
                uc.btnCallback =
                    {
                        smsPermissionLauncher.launch(
                            arrayOf(
                                android.Manifest.permission.READ_SMS,
                                android.Manifest.permission.SEND_SMS,
                                android.Manifest.permission.RECEIVE_SMS
                            )
                        )
                    }
            } else {
                uc.rank = 52
                uc.secondaryText =
                    resources.getString(
                        R.string.try_saying,
                        mContactsUtterances.iterator().next()
                    )
                uc.btnText = ""
                uc.btnCallback = {}
            }
        } else {
            uc.rank = 9
            uc.secondaryText =
                resources.getString(
                    R.string.lp_carousel_sms_secondary_alexa_system,
                    primaryDevice.deviceName
                )
            uc.btnText =
                resources.getString(R.string.cta_label_enable)
            uc.btnCallback = {
                AlexaApp.from(requireContext()).rootComponent
                    .getComponent(ContactsController::class.java)
                    .ifPresent(
                        Consumer { contactsController: ContactsController ->
                            contactsController.setMessagingPermission(
                                primaryDevice.deviceAddress,
                                Constants.CONTACTS_PERMISSION_YES
                            )
                        }.andThen {
                            if (!devicePermission) {
                                smsPermissionLauncher.launch(
                                    arrayOf(
                                        android.Manifest.permission.READ_SMS,
                                        android.Manifest.permission.SEND_SMS,
                                        android.Manifest.permission.RECEIVE_SMS
                                    )
                                )
                            } else {
                                uc.rank = 52
                                uc.secondaryText =
                                    resources.getString(
                                        R.string.try_saying,
                                        mContactsUtterances.iterator().next()
                                    )
                                uc.btnText = ""
                                uc.btnCallback = {}
                            }
                            mAdapter.notifyDataSetChanged()
                        }
                    )
            }
        }
        mAdapter.carouselList.add(uc)
        mAdapter.carouselList.sortBy { it.rank }
        mAdapter.notifyDataSetChanged()
    }

    private fun setupWakewordToggle() {
        mInFlightOperations.add(
            mAlexaPropertyManager.getAlexaPropertyBoolean(AACSPropertyConstants.WAKEWORD_SUPPORTED)
                .filter { obj: Optional<Boolean?> -> obj.isPresent }
                .map { obj: Optional<Boolean?> -> obj.get() }
                .doOnError { err -> Log.e(TAG, err.message!!) }
                .subscribe { wakeWordSupported: Boolean? ->
                    if (!wakeWordSupported!!) {
                        Log.e(
                            TAG,
                            "Alexa wake word is disabled, removing the configuration for handsfree"
                        )
                        return@subscribe
                    }
                    mInFlightOperations.add(
                        mAlexaPropertyManager.getAlexaPropertyBoolean(AACSPropertyConstants.WAKEWORD_ENABLED)
                            .filter { obj: Optional<Boolean?> -> obj.isPresent }
                            .map { obj: Optional<Boolean?> -> obj.get() }
                            .doOnError { err -> Log.e(TAG, err.message!!) }
                            .subscribe { checked: Boolean? ->
                                binding.wakewordSwitch.isChecked = checked!!
                                binding.wakewordSwitch.jumpDrawablesToCurrentState()
                            }
                    )
                    binding.wakewordToggleContainer.setOnClickListener {
                        mInFlightOperations.add(
                            mAlexaPropertyManager.updateAlexaPropertyBoolean(
                                AACSPropertyConstants.WAKEWORD_ENABLED,
                                !binding.wakewordSwitch.isChecked
                            ).subscribe { success->
                                if (success) {
                                    binding.wakewordSwitch.isChecked = !binding.wakewordSwitch.isChecked
                                } else {
                                    Log.e(TAG, "error updating property.")
                                }
                            }
                        )
                    }
                }
        )
    }

    private fun configureLowerContent() {
        // TODO: have this pull from any random domain
        // TODO: but for now, have it pull from getting started
        val iterator: Iterator<String> = mGettingStartedUtterances.iterator()
        binding.sampleUtterance1.text = iterator.next()
        binding.sampleUtterance2.text = iterator.next()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        mInFlightOperations.dispose()
        _binding = null
    }
}
