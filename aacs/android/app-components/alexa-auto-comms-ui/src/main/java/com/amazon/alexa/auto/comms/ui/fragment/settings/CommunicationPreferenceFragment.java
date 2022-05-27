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

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.LiveData;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.PreferenceKeys;
import com.amazon.alexa.auto.comms.ui.R;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;

/**
 * Communication preference fragment to manage all the communication consents status.
 */
public class CommunicationPreferenceFragment extends PreferenceFragmentCompat {
    private static final String TAG = CommunicationPreferenceFragment.class.getSimpleName();

    private String mDeviceAddress;
    private AlexaApp mApp;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.communication_preferences, rootKey);

        Context context = getContext();
        if (context != null) {
            PreferenceManager.setDefaultValues(context, R.xml.communication_preferences, false);
        }

        if (getArguments() != null) {
            if (getArguments().getString(Constants.COMMUNICATION_DEVICE_ADDRESS) != null) {
                mDeviceAddress = getArguments().getString(Constants.COMMUNICATION_DEVICE_ADDRESS);
            } else {
                Log.d(TAG, "device ID is null.");
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        requireView();

        setCommunicationConsentPermission();
        monitorContactUploadConsentPreferenceChanges();
    }

    private void monitorContactUploadConsentPreferenceChanges() {
        SwitchPreferenceCompat contactUploadConsent = findPreference(PreferenceKeys.CONTACTS_CONSENT_SETTINGS);
        Preconditions.checkNotNull(contactUploadConsent);

        contactUploadConsent.setOnPreferenceChangeListener((preference, newValue) -> {
            Context context = getContext();
            Preconditions.checkNotNull(context);

            mApp = AlexaApp.from(context);

            Boolean consentValue = (Boolean) newValue;
            if (consentValue) {
                uploadContacts(mDeviceAddress);
            } else {
                removeContacts(mDeviceAddress);
            }

            return true;
        });
    }

    private void setCommunicationConsentPermission() {
        Context context = getContext();
        Preconditions.checkNotNull(context);

        LiveData<BTDevice> device = BTDeviceRepository.getInstance(getContext()).getBTDeviceByAddress(mDeviceAddress);
        device.observe(getViewLifecycleOwner(), observer -> {
            if (device.getValue() != null) {
                SwitchPreferenceCompat contactUploadConsent = findPreference(PreferenceKeys.CONTACTS_CONSENT_SETTINGS);

                Preconditions.checkNotNull(contactUploadConsent);

                if (ModuleProvider.isAlexaCustomAssistantEnabled(context)) {
                    contactUploadConsent.setSummary(
                            R.string.contacts_upload_consent_summary_with_alexa_custom_assistant);
                }

                if ((device.getValue().getContactsUploadPermission().equals(Constants.CONTACTS_PERMISSION_YES))) {
                    contactUploadConsent.setChecked(true);
                } else {
                    contactUploadConsent.setChecked(false);
                }

            } else {
                Log.d(TAG, "There is no device found.");
            }
        });
    }

    /**
     * Upload contacts with device address.
     * @param deviceAddress bluetooth device address.
     */
    public void uploadContacts(String deviceAddress) {
        mApp.getRootComponent().getComponent(ContactsController.class).ifPresent(contactsController -> {
            Log.d(TAG, "Uploading contacts...");
            contactsController.setContactsUploadPermission(deviceAddress, Constants.CONTACTS_PERMISSION_YES);
            contactsController.uploadContacts(deviceAddress);
        });
    }

    /**
     * Remove contacts with device address.
     * @param deviceAddress bluetooth device address.
     */
    public void removeContacts(String deviceAddress) {
        mApp.getRootComponent().getComponent(ContactsController.class).ifPresent(contactsController -> {
            Log.d(TAG, "Removing contacts");
            contactsController.setContactsUploadPermission(deviceAddress, Constants.CONTACTS_PERMISSION_NO);
            contactsController.removeContacts(deviceAddress);
        });
    }
}
