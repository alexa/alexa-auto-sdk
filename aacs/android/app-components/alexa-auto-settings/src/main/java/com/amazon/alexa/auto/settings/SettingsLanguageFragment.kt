package com.amazon.alexa.auto.settings

import android.content.Intent
import android.content.res.Resources
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import com.amazon.aacsconstants.AACSPropertyConstants
import com.amazon.alexa.auto.apps.common.util.LocaleUtil
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager
import com.amazon.alexa.auto.settings.databinding.FragmentSettingsLanguageBinding
import com.amazon.alexa.auto.settings.dependencies.AndroidModule
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent
import io.reactivex.rxjava3.disposables.CompositeDisposable
import java.util.*
import javax.inject.Inject

/**
 * A simple [Fragment] subclass.
 * Use the [SettingsLanguageFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class SettingsLanguageFragment : Fragment() {

    private var _binding: FragmentSettingsLanguageBinding? = null
    private val binding get() = _binding!!

    @Inject
    lateinit var mAlexaPropertyManager: AlexaPropertyManager

    @Inject
    lateinit var mAlexaLocalesProvider: AlexaLocalesProvider

    private lateinit var mInFlightOperations: CompositeDisposable

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
        }
        DaggerSettingsComponent.builder().androidModule(
            AndroidModule(
                requireContext()
            )
        ).build().inject(this)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSettingsLanguageBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.listSystemLanguageItem.setOnClickListener {
            val intent = Intent(Settings.ACTION_LOCALE_SETTINGS)
            intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
            requireContext().startActivity(intent)
        }
        binding.listAlexaLanguageItem.setOnClickListener {
            findNavController().navigate(R.id.navigation_fragment_settings_alexa_languages)
        }
    }

    override fun onResume() {
        super.onResume()
        mInFlightOperations = CompositeDisposable()
        binding.listSystemLanguageItemSummary.text =
            Resources.getSystem().configuration.locales[0].displayLanguage
        mInFlightOperations.add(
            mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                .filter { obj -> obj.isPresent }
                .map { obj -> obj.get() }
                .subscribe { propertyLocale: String ->
                    mInFlightOperations.add(
                        mAlexaLocalesProvider.fetchAlexaSupportedLocaleWithId(propertyLocale).subscribe { localeMap ->
                            binding.listAlexaLanguageItemSummary.text = localeMap.get().first
                        }
                    )
                }
        )
    }

    override fun onPause() {
        super.onPause()
        mInFlightOperations.dispose()
    }

    companion object {
        val TAG: String = SettingsLanguageFragment::class.java.canonicalName

        @JvmStatic
        fun newInstance() =
            SettingsLanguageFragment().apply {
                arguments = Bundle().apply {
                }
            }
    }
}
