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
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.LiveData;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.R;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import java.util.List;
import java.util.Set;


/**
 * Communication fragment to manage all connected bluetooth devices in a list.
 */
public class CommunicationFragment extends Fragment {
    private static final String TAG = CommunicationFragment.class.getSimpleName();

    private NavController mController;
    BluetoothManager mBluetoothManager;
    BluetoothAdapter mBluetoothAdapter;
    Bundle newBundle;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        newBundle = getArguments();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {

        if(newBundle.getString("device").equals("true")){
            return inflater.inflate(R.layout.communication_settings_fragment, container, false);
        }
        return inflater.inflate(R.layout.communication_settings_fragment_no_device, container, false);
    }

    @Override
    public void onViewCreated(View view1, Bundle bundle){
        super.onViewCreated(view1, bundle);
        View fragmentView = requireView();
        mController = findNavController(fragmentView);
        mBluetoothManager = (BluetoothManager) getContext().getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = mBluetoothManager.getAdapter();
        BTDeviceRepository BTDeviceRepo = BTDeviceRepository.getInstance(getContext());
        ConnectedBTDeviceRepository ConnectedBTDeviceRepo = ConnectedBTDeviceRepository.getInstance(getContext());

        // Dynamically observe all the paired bluetooth devices from bluetooth adapter bonded devices
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        // Save last device address to know when to add primary device to top and add pair new button

        ConstraintLayout rootLayout = (ConstraintLayout) fragmentView.findViewById(R.id.alexa_communication_fragment);

        LiveData<List<ConnectedBTDevice>> listData =
                ConnectedBTDeviceRepository.getInstance(getContext()).getConnectedDevices();

                listData.observe(getViewLifecycleOwner(), listObserver -> {
                    if (listData.getValue() != null && listData.getValue().size() > 0 && pairedDevices.size()>0) {
                        // Per Android telephony, it uses last paired bluetooth device for calling and handle messages.
                        ConnectedBTDevice primaryDevice = listData.getValue().get(listData.getValue().size() - 1);
                        String lastAddress = primaryDevice.getDeviceAddress();
                        String lastDeviceName = primaryDevice.getDeviceName();

                        TextView primaryDeviceName = (TextView) rootLayout.findViewById(R.id.deviceName);
                        String formattedString = String.format((String) primaryDeviceName.getText(), lastDeviceName);
                        primaryDeviceName.setText( formattedString);
                        Bundle deviceAddress = new Bundle();
                        deviceAddress.putString(Constants.COMMUNICATION_DEVICE_ADDRESS, lastAddress);
                        requireActivity().getSupportFragmentManager()
                                .beginTransaction()
                                .replace(R.id.settings, new CommunicationPreferenceFragment(deviceAddress))
                                .commit();
                        Button btSetting = (Button) rootLayout.findViewById(R.id.btSetting);
                        btSetting.setOnClickListener(view -> {
                            goToBluetoothSettings(getContext());
                        });
                    }else {
                        ConstraintLayout rootLayout1 = (ConstraintLayout) fragmentView.findViewById(R.id.alexa_communication_fragment);
                        Button btSetting = (Button) rootLayout1.findViewById(R.id.btSetting);
                        btSetting.setOnClickListener(view -> { goToBluetoothSettings(getContext());} );
                    }
                });
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
