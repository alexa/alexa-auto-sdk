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
package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.LOCATION_CONSENT_COMPLETED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.DISABLED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.ENABLED;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

import javax.inject.Inject;

/**
 * Fragment to support displaying the location consent screen
 */
public class LocationConsentFragment extends Fragment {
    private static final String TAG = LocationConsentFragment.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    /**
     * Constructs an instance of LocationConsentFragment.
     */
    public LocationConsentFragment() {}

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mAlexaPropertyManager == null) {
            DaggerSetupComponent.builder()
                    .androidModule(new AndroidModule(getContext()))
                    .build()
                    .injectLocationConsentFragment(this);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.location_consent, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();

        TextView skipButtonView = fragmentView.findViewById(R.id.setup_skip_button_view);
        skipButtonView.setOnClickListener(view -> { updateAACSPropertyAndNavigate(DISABLED.getValue()); });
        TextView useLocationButtonView = fragmentView.findViewById(R.id.use_location_button_view);
        useLocationButtonView.setOnClickListener(view -> { updateAACSPropertyAndNavigate(ENABLED.getValue()); });

        if (ModuleProvider.isAlexaCustomAssistantEnabled(fragmentView.getContext())) {
            ImageView alexaImage = fragmentView.findViewById(R.id.alexa_img_view);
            alexaImage.setVisibility(View.GONE);

            TextView locationPermissionText = fragmentView.findViewById(R.id.location_permission_text_view);
            locationPermissionText.setText(R.string.location_permission_body_text_with_alexa_custom_assistant);

            TextView hintText = fragmentView.findViewById(R.id.alexa_hint2);
            hintText.setVisibility(View.GONE);
        }
    }

    private void updateAACSPropertyAndNavigate(String value) {
        mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.GEOLOCATION_ENABLED, value)
                .doOnSuccess((succeeded) -> {
                    if (succeeded) {
                        Log.d(TAG, "Successfully updated geolocation value to: " + value);
                        EventBus.getDefault().post(new WorkflowMessage(LOCATION_CONSENT_COMPLETED));
                    } else {
                        Log.e(TAG, "Failed to update geolocation value to: " + value);
                        EventBus.getDefault().post(new WorkflowMessage(SETUP_ERROR));
                    }
                })
                .subscribe();
    }
}
