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
package com.amazon.alexa.auto.comms.ui.fragment.setup;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.R;
import com.amazon.alexa.auto.comms.ui.RequestContactsConsentActivity;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import java.util.List;

/**
 * Fragment for displaying Communications consent screen.
 */
public class CommunicationConsentFragment extends Fragment {
    private static final String TAG = CommunicationConsentFragment.class.getSimpleName();

    private CommunicationConsentViewModel mViewModel;

    /**
     * Constructs an instance of CommunicationsFragment.
     */
    public CommunicationConsentFragment() {}

    /**
     * Constructs an instance of CommunicationsFragment.
     *
     * @param viewModel View Model for Communications consent.
     */
    @VisibleForTesting
    CommunicationConsentFragment(@NonNull CommunicationConsentViewModel viewModel) {
        this.mViewModel = viewModel;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(CommunicationConsentViewModel.class);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.communication_setup_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        observePrimaryConnectedDevice();
    }

    /**
     * Observe primary connected device, which we use for calling and handling messages.
     */
    private void observePrimaryConnectedDevice() {
        LiveData<List<ConnectedBTDevice>> listData =
                ConnectedBTDeviceRepository.getInstance(getContext()).getConnectedDevices();
        listData.observe(getViewLifecycleOwner(), listObserver -> {
            if (listData.getValue() != null && listData.getValue().size() > 0) {
                // Per Android telephony, it uses last paired bluetooth device for calling and handle messages.
                String deviceAddress = listData.getValue().get(listData.getValue().size() - 1).getDeviceAddress();
                Log.d(TAG, "Primary device is found.");
                observeContactUploadPermission(deviceAddress);
            } else {
                Log.i(TAG, "There is no connected device found.");
            }
        });
    }

    /**
     * Observe contact upload consent, if user consents to upload contacts, we will automatically upload contacts with
     * the synced phone book. If not, we display the consent screen again to ask for the permission.
     * @param deviceAddress bluetooth device address.
     */
    private void observeContactUploadPermission(String deviceAddress) {
        LiveData<BTDevice> device = BTDeviceRepository.getInstance(getContext()).getBTDeviceByAddress(deviceAddress);
        device.observe(getViewLifecycleOwner(), observer -> {
            if (device.getValue() != null) {
                if (device.getValue().getContactsUploadPermission().equals(Constants.CONTACTS_PERMISSION_YES)) {
                    Log.d(TAG, "Device's contacts upload permission is YES, uploading contacts.");
                    mViewModel.uploadContacts(deviceAddress);
                } else {
                    View fragmentView = requireView();
                    TextView consentPermissionHeading =
                            fragmentView.findViewById(R.id.contacts_permission_consent_heading);
                    TextView getYesButtonText = fragmentView.findViewById(R.id.contacts_upload_yes_action_button);
                    TextView getSkipButtonText = fragmentView.findViewById(R.id.contacts_upload_skip_action_button);
                    ImageView alexaImage = fragmentView.findViewById(R.id.alexa_img_view);

                    String format = "";
                    if (ModuleProvider.isAlexaCustomAssistantEnabled(fragmentView.getContext())) {
                        // Remove Alexa logo placeholder

                        alexaImage.setVisibility(View.GONE);

                        // Update text content
                        format = getResources().getString(
                                R.string.contacts_permission_consent_body_with_alexa_custom_assistant);
                        getYesButtonText.setText(R.string.contacts_consent_yes_with_alexa_custom_assistant);
                        TextView alexaContactsHint = fragmentView.findViewById(R.id.alexa_contacts_hint1);
                        alexaContactsHint.setVisibility(View.GONE);
                    } else {
                        format = getResources().getString(R.string.contacts_permission_consent_title);
                    }
                    String headingString = String.format(format, device.getValue().getDeviceName());
                    consentPermissionHeading.setText(headingString);
                    String actName = String.valueOf(getActivity());
                    Log.d(TAG, "Device's contacts upload permission is NO, showing contacts consent card.");
                    getYesButtonText.setOnClickListener(view -> {
                        mViewModel.setContactsUploadPermission(
                                device.getValue().getDeviceAddress(), Constants.CONTACTS_PERMISSION_YES);
                        if (actName.contains(Constants.CONTACTS_ACTIVITY)) {
                            popupActivity();
                        }
                    });

                    getSkipButtonText.setOnClickListener(view -> {
                        mViewModel.setContactsUploadPermission(
                                device.getValue().getDeviceAddress(), Constants.CONTACTS_PERMISSION_NO);
                        if (actName.contains(Constants.CONTACTS_ACTIVITY)) {
                            popupActivity();
                        }
                    });
                }
            } else {
                Log.d(TAG, "Connected device is not found");
            }
        });
    }
    /**
     * Starts the RequestContactsConsentActivity
     */
    private void popupActivity() {
        Intent intent = new Intent(getActivity(), RequestContactsConsentActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        intent.putExtra("keep", "false");
        startActivity(intent);
    }
}
