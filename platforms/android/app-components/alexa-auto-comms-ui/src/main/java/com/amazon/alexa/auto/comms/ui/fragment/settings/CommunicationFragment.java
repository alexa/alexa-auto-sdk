package com.amazon.alexa.auto.comms.ui.fragment.settings;

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
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDevice;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;
import com.amazon.alexa.auto.setup.workflow.fragment.NetworkFragment;

import java.util.List;

/**
 * Communication fragment to manage all connected bluetooth devices in a list.
 */
public class CommunicationFragment extends Fragment {
    private static final String TAG = NetworkFragment.class.getSimpleName();

    private NavController mController;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.communication_settings_fragment, container, false);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();
        mController = findNavController(fragmentView);

        LinearLayout rootLayout = (LinearLayout) fragmentView.findViewById(R.id.alexa_communication_fragment);

        // Dynamically observe all the connected bluetooth devices from bluetooth device database,
        // add the communication layout for each device.
        LiveData<List<ConnectedBTDevice>> listData =
                ConnectedBTDeviceRepository.getInstance(getContext()).getConnectedDevices();
        listData.observe(getViewLifecycleOwner(), listObserver -> {
            if (listData.getValue() != null && listData.getValue().size() > 0) {
                for (int i = listData.getValue().size() - 1; i >= 0; i--) {
                    String address = listData.getValue().get(i).getDeviceAddress();
                    String name = listData.getValue().get(i).getDeviceName();
                    String contactsUploadPermission = listData.getValue().get(i).getContactsUploadPermission();

                    View to_add = getLayoutInflater().inflate(R.layout.communication_consent_layout, rootLayout, false);

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
                }
            } else {
                Log.d(TAG, "There is no device found.");
            }

            View pairNewOne = getLayoutInflater().inflate(R.layout.communication_pair_new_layout, rootLayout, false);
            pairNewOne.setOnClickListener(view -> { goToBluetoothSettings(getContext()); });
            rootLayout.addView(pairNewOne);
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
