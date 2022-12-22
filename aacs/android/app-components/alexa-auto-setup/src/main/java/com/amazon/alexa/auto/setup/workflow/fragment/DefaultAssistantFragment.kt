package com.amazon.alexa.auto.setup.workflow.fragment

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.Settings.ACTION_VOICE_INPUT_SETTINGS
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil
import com.amazon.alexa.auto.setup.R
import com.amazon.alexa.auto.setup.databinding.FragmentDefaultAssistantBinding
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent.ALEXA_IS_SELECTED_EVENT
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR
import org.greenrobot.eventbus.EventBus

/**
 * A simple [Fragment] subclass.
 * Use the [DefaultAssistantFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class DefaultAssistantFragment : Fragment() {
    private var _binding: FragmentDefaultAssistantBinding? = null
    private val binding get() = _binding!!

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {}
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentDefaultAssistantBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.exitButton.setOnClickListener {
            requireActivity().finish()
        }
        val isPermissionGranted =
            requireContext().checkSelfPermission(Manifest.permission.WRITE_SECURE_SETTINGS)
        if (PackageManager.PERMISSION_GRANTED === isPermissionGranted) {
            binding.yesButton.setOnClickListener {
                if (DefaultAssistantUtil.setAlexaAppAsDefault(requireContext())) {
                    if (!AlexaApp.from(requireContext()).rootComponent.alexaSetupController
                        .isSetupCompleted
                    ) {
                        EventBus.getDefault().post(WorkflowMessage(ALEXA_IS_SELECTED_EVENT))
                    }
                } else {
                    EventBus.getDefault().post(WorkflowMessage(SETUP_ERROR))
                }
            }
        } else {
            binding.bodyText.text = resources.getText(R.string.assist_selection_in_system_settings)
            binding.yesButton.setText(R.string.settings_button)
            binding.yesButton.setOnClickListener {
                val launchSettingsIntent =
                    Intent(ACTION_VOICE_INPUT_SETTINGS)
                launchSettingsIntent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
                startActivity(launchSettingsIntent)
            }
        }
    }

    companion object {
        private val TAG = DefaultAssistantFragment::class.java.canonicalName

        @JvmStatic
        fun newInstance() =
            DefaultAssistantFragment().apply {
                arguments = Bundle().apply {}
            }
    }
}
