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

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;

import javax.inject.Inject;

/**
 * Fragment for Alexa Language Selection.
 */
public class LanguageSelectionFragment extends Fragment {
    private static final String TAG = LanguageSelectionFragment.class.getSimpleName();

    @Inject
    AlexaLocalesProvider mLocalesProvider;

    private NavController mController;

    @VisibleForTesting
    Bundle mShowContinueButton;

    /**
     * Constructs an instance of LanguageSelectionFragment.
     */
    public LanguageSelectionFragment() {}

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mShowContinueButton = new Bundle();
        mShowContinueButton.putBoolean("showContinueButton", true);

        DaggerSetupComponent.builder()
                .androidModule(new AndroidModule(getContext()))
                .build()
                .injectLanguageSettingsFragment(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.start_language_selection, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();
        mController = findNavController(fragmentView);

        String currentLocale = LocaleUtil.getCurrentDeviceLocaleDisplayName(getContext());
        TextView languageSelectionBodyText = fragmentView.findViewById(R.id.language_selection_body_text_view);
        String format = getResources().getString(R.string.select_language_body);
        String bodyString = String.format(format, currentLocale);
        languageSelectionBodyText.setText(bodyString);

        TextView getStartedButtonText = fragmentView.findViewById(R.id.select_language_action_button);

        getStartedButtonText.setOnClickListener(
                view -> { mController.navigate(R.id.navigation_fragment_languageSelection, mShowContinueButton); });
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
