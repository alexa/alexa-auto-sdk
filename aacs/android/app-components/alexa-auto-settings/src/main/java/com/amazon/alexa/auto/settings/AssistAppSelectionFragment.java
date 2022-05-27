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
package com.amazon.alexa.auto.settings;

import static android.provider.Settings.ACTION_VOICE_INPUT_SETTINGS;

import static androidx.navigation.Navigation.findNavController;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.ALEXA_IS_SELECTED_EVENT;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

public class AssistAppSelectionFragment extends Fragment {
    private static final String TAG = AssistAppSelectionFragment.class.getSimpleName();
    /**
     * Constructs an instance of AssistAppSelectionFragment.
     */
    public AssistAppSelectionFragment() {}

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.assist_app_selection, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();
        TextView instructionsText = fragmentView.findViewById(R.id.assist_selection_text_view);
        TextView buttonView = fragmentView.findViewById(R.id.set_alexa_default_button_view);
        TextView exitView = fragmentView.findViewById(R.id.exit_button_view);
        exitView.setText(R.string.confirm_exit_button);
        exitView.setOnClickListener(view -> { getActivity().finish(); });

        int isPermissionGranted = getContext().checkSelfPermission(Manifest.permission.WRITE_SECURE_SETTINGS);
        if (PackageManager.PERMISSION_GRANTED == isPermissionGranted) {
            instructionsText.setText(R.string.assist_selection_in_app);
            buttonView.setText(R.string.confirm_yes_button);
            buttonView.setOnClickListener(view -> {
                if (DefaultAssistantUtil.setAlexaAppAsDefault(getContext())) {
                    if (!AlexaApp.from(getContext()).getRootComponent().getAlexaSetupController().isSetupCompleted()) {
                        EventBus.getDefault().post(new WorkflowMessage(ALEXA_IS_SELECTED_EVENT));
                    } else {
                        Log.i(TAG, "Setup was completed. Skipping the following setup screens.");
                        NavController navController = findNavController(view);
                        navController.navigate(R.id.navigation_fragment_alexa_settings_home);
                    }
                } else {
                    EventBus.getDefault().post(new WorkflowMessage(SETUP_ERROR));
                }
            });
        } else {
            instructionsText.setText(getResources().getText(R.string.assist_selection_in_system_settings));
            buttonView.setText(R.string.settings_button);
            buttonView.setOnClickListener(view -> {
                Intent launchSettingsIntent = new Intent(ACTION_VOICE_INPUT_SETTINGS);
                launchSettingsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(launchSettingsIntent);
            });
        }
    }
}
