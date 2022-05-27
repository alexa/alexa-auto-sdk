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
package com.amazon.alexa.auto.comms.ui.fragment.settings;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.LiveData;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.R;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;

import java.util.Set;

/**
 * Communication fragment to manage all connected bluetooth devices in a list.
 */
public class CommunicationFragment extends Fragment {
    private static final String TAG = CommunicationFragment.class.getSimpleName();

    private NavController mController;
    BluetoothManager mBluetoothManager;
    BluetoothAdapter mBluetoothAdapter;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.communication_settings_fragment, container, false);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        View fragmentView = requireView();
        mController = findNavController(fragmentView);
        mBluetoothManager = (BluetoothManager) getContext().getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = mBluetoothManager.getAdapter();
        LinearLayout rootLayout = (LinearLayout) fragmentView.findViewById(R.id.alexa_communication_fragment);
        // Dynamically observe all the paired bluetooth devices from bluetooth adapter bonded devices
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                String name = device.getName();
                String address = device.getAddress();
                LiveData<BTDevice> btDevice =
                        BTDeviceRepository.getInstance(getContext()).getBTDeviceByAddress(address);
                btDevice.observe(getViewLifecycleOwner(), listObserver -> {
                    if (btDevice.getValue() != null) {
                        String contactsUploadPermission = btDevice.getValue().getContactsUploadPermission();

                        // add the communication layout for each device.
                        View to_add =
                                getLayoutInflater().inflate(R.layout.communication_consent_layout, rootLayout, false);

                        TextView deviceName = (TextView) to_add.findViewById(R.id.deviceName);
                        deviceName.setText(name);

                        TextView contactConsent = (TextView) to_add.findViewById(R.id.consentStatus);
                        String communicationConsentStatus = Constants.COMMUNICATION_PERMISSION_DISABLED;

                        if (contactsUploadPermission.equals(Constants.CONTACTS_PERMISSION_YES)) {
                            communicationConsentStatus = Constants.COMMUNICATION_PERMISSION_ENABLED;
                        }

                        String format = getResources().getString(R.string.comms_permission_status);
                        String bodyString = String.format(format, communicationConsentStatus);
                        contactConsent.setText(bodyString);

                        Bundle deviceAddress = new Bundle();
                        deviceAddress.putString(Constants.COMMUNICATION_DEVICE_ADDRESS, address);
                        ImageView checkMore = to_add.findViewById(R.id.checkMore);
                        checkMore.setOnClickListener(view -> {
                            mController.navigate(R.id.navigation_fragment_communication_consent, deviceAddress);
                        });
                        rootLayout.addView(to_add);
                    } else {
                        Log.d(TAG, "There is no device found.");
                    }
                });
            }
        }
        View pairNewOne = getLayoutInflater().inflate(R.layout.communication_pair_new_layout, rootLayout, false);
        pairNewOne.setOnClickListener(view -> { goToBluetoothSettings(getContext()); });
        rootLayout.addView(pairNewOne);
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }

    /**
     * Navigate to Android bluetooth settings.
     * @param context Android context.
     */
    private void goToBluetoothSettings(Context context) {
        Intent intent = new Intent(Settings.ACTION_BLUETOOTH_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.getApplicationContext().startActivity(intent);
    }
}
