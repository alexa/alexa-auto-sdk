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

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.constraintlayout.widget.ConstraintSet
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.navigation.NavController
import androidx.navigation.fragment.findNavController
import com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil
import com.amazon.alexa.auto.setup.databinding.BlockersBinding

/**
 * Surface this fragment when there are blocker issues (i.e. core permissions not granted)
 */
class BlockersFragment : Fragment() {
    private var _binding: BlockersBinding? = null
    private val binding get() = _binding!!
    private var mAssistAppBoolean: Boolean = false
    private var mMicPermissionBoolean: Boolean = false

    private val micPermissionLauncher =
        registerForActivityResult(
            ActivityResultContracts.RequestPermission()
        ) { isGranted: Boolean ->
            if (!isGranted) {
                Log.e(TAG, "Mic permission was not granted.")
            } else {
                mMicPermissionBoolean = false
                Log.d(TAG, "All mic permissions granted.")
                binding.micLogo?.visibility = View.INVISIBLE
                binding.micTitle?.visibility = View.INVISIBLE
                binding.micBody?.visibility = View.INVISIBLE
                binding.micButton?.visibility = View.INVISIBLE

                if (mAssistAppBoolean) {
                    val cs = ConstraintSet()
                    cs.clone(binding.blockersLayout)
                    cs.connect(
                        binding.assistLogo!!.id,
                        ConstraintSet.TOP,
                        binding.listTop!!.id,
                        ConstraintSet.BOTTOM
                    )
                    cs.applyTo(binding.blockersLayout)
                }

                navToLandingPage()
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (!DefaultAssistantUtil.shouldSkipAssistAppSelectionScreen(requireContext())) {
            mAssistAppBoolean = true
        }
        if (ContextCompat.checkSelfPermission(
                requireContext(),
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            mMicPermissionBoolean = true
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = BlockersBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        if (mMicPermissionBoolean) {
            Log.e(TAG, "Mic permission is not set.")
            binding.micButton?.setOnClickListener {
                micPermissionLauncher.launch(Manifest.permission.RECORD_AUDIO)
            }
        } else {
            binding.micLogo?.visibility = View.INVISIBLE
            binding.micTitle?.visibility = View.INVISIBLE
            binding.micBody?.visibility = View.INVISIBLE
            binding.micButton?.visibility = View.INVISIBLE

            if (mAssistAppBoolean) {
                val cs = ConstraintSet()
                cs.clone(binding.blockersLayout)
                cs.connect(
                    binding.assistLogo!!.id,
                    ConstraintSet.TOP,
                    binding.listTop!!.id,
                    ConstraintSet.BOTTOM
                )
                cs.applyTo(binding.blockersLayout)
            } else {
                navToLandingPage()
            }
        }

        if (mAssistAppBoolean) {
            binding.assistButton?.setOnClickListener {
                mAssistAppBoolean = DefaultAssistantUtil.setAlexaAppAsDefault(requireContext())
                if (!mAssistAppBoolean) {
                    binding.assistLogo?.visibility = View.INVISIBLE
                    binding.assistTitle?.visibility = View.INVISIBLE
                    binding.assistBody?.visibility = View.INVISIBLE
                    binding.assistButton?.visibility = View.INVISIBLE
                    navToLandingPage()
                }
            }
        } else {
            binding.assistLogo?.visibility = View.INVISIBLE
            binding.assistTitle?.visibility = View.INVISIBLE
            binding.assistBody?.visibility = View.INVISIBLE
            binding.assistButton?.visibility = View.INVISIBLE
            navToLandingPage()
        }
    }

    /**
     * navigates to the landing page if no blockers are active
     *
     * @return Unit
     */
    private fun navToLandingPage() {
        if (!mAssistAppBoolean && !mMicPermissionBoolean) {
            val navController: NavController = findNavController()
            navController.navigate(R.id.navigation_fragment_alexa_landing_page)
        }
    }

    companion object {
        private val TAG = BlockersFragment::class.java.simpleName
    }
}
