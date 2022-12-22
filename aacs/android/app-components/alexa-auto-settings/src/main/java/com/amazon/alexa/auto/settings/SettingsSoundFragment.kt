package com.amazon.alexa.auto.settings

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.*
import com.amazon.alexa.auto.apps.common.util.ModuleProvider
import com.amazon.alexa.auto.settings.databinding.FragmentSettingsSoundBinding
import io.reactivex.rxjava3.disposables.CompositeDisposable

class SettingsSoundFragment : Fragment() {

    private var _binding: FragmentSettingsSoundBinding? = null
    private val binding get() = _binding!!

    private lateinit var mInFlightOperations: CompositeDisposable

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
        }
        mInFlightOperations = CompositeDisposable()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSettingsSoundBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.listStartSoundSwitch.isChecked = isStartEarconSettingEnabled(requireContext())
        binding.listEndSoundSwitch.isChecked = isEndEarconSettingEnabled(requireContext())

        if (ModuleProvider.isAlexaCustomAssistantEnabled(requireContext())) {
            binding.listStartSoundItemSummary.text = resources.getString(R.string.setting_voice_assistance_sounds_request_start_summary)
            binding.listEndSoundItemSummary.text = resources.getString(R.string.setting_voice_assistance_sounds_request_end_summary)
        }

        binding.listStartSoundItem.setOnClickListener {
            setStartEarconSetting(requireContext(), !binding.listStartSoundSwitch.isChecked)
            binding.listStartSoundSwitch.isChecked = !binding.listStartSoundSwitch.isChecked
        }
        binding.listEndSoundItem.setOnClickListener {
            setEndEarconSetting(requireContext(), !binding.listEndSoundSwitch.isChecked)
            binding.listEndSoundSwitch.isChecked = !binding.listEndSoundSwitch.isChecked
        }


        binding.listStartSoundSwitch.setOnCheckedChangeListener { buttonView, isChecked ->
            if (isChecked) {
                setStartEarconSetting(requireContext(), true)
            } else {
                setStartEarconSetting(requireContext(), false)
            }
        }
        binding.listEndSoundSwitch.setOnCheckedChangeListener { buttonView, isChecked ->
            if (isChecked) {
                setEndEarconSetting(requireContext(), true)
            } else {
                setEndEarconSetting(requireContext(), false)
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        mInFlightOperations.dispose()
    }

    companion object {
        @JvmStatic
        fun newInstance() =
            SettingsLanguageFragment().apply {
                arguments = Bundle().apply {
                }
            }
    }
}
