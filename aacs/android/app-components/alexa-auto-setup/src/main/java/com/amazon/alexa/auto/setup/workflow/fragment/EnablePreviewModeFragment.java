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

import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;
import static com.amazon.alexa.auto.app.common.util.PopupDialogUtil.embedUrlInPopupDialog;
import static com.amazon.alexa.auto.app.common.util.ViewUtils.toggleViewVisibility;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

import android.graphics.Color;
import android.os.Bundle;
import android.text.Annotation;
import android.text.SpannedString;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import javax.inject.Inject;

/**
 * Fragment for enabling Preview Mode
 */
public class EnablePreviewModeFragment extends Fragment {
    private static final String TAG = EnablePreviewModeFragment.class.getSimpleName();

    private static final String POP_UP = "&pop-up=1";

    private EnablePreviewModeViewModel mViewModel;

    private static final int HYPERLINK_TEXT_COLOR = Color.parseColor("#00A8E1");

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    /**
     * Constructs an instance of EnablePreviewModeFragment.
     */
    public EnablePreviewModeFragment() {}

    @VisibleForTesting
    EnablePreviewModeFragment(@NonNull EnablePreviewModeViewModel mViewModel) {
        this.mViewModel = mViewModel;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mAlexaPropertyManager == null) {
            DaggerSetupComponent.builder()
                    .androidModule(new AndroidModule(getContext()))
                    .build()
                    .injectEnablePreviewModeFragment(this);
        }
        mViewModel =
                mViewModel == null ? new ViewModelProvider(this).get(EnablePreviewModeViewModel.class) : mViewModel;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.enable_preview_mode, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mViewModel.loginWorkflowState().observe(getViewLifecycleOwner(), this::authWorkflowStateChanged);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();

        TextView enablePreviewModeButton = fragmentView.findViewById(R.id.enable_preview_mode_action_button);
        enablePreviewModeButton.setOnClickListener(view -> mViewModel.enablePreviewMode());

        TextView enablePreviewModeDisclaimer = fragmentView.findViewById(R.id.enable_preview_mode_disclaimer);
        SpannedString spannedString = new SpannedString(getText(R.string.enable_preview_mode_disclaimer));
        Annotation[] spans = spannedString.getSpans(0, spannedString.length(), Annotation.class);

        if (spans.length > 0) {
            String termsOfUseUrl = getResources().getString(R.string.terms_of_use_url) + POP_UP;
            String allTermsOfUseUrl = getResources().getString(R.string.all_terms_of_use_url) + POP_UP;
            String privacyInfoUrl = getResources().getString(R.string.privacy_url) + POP_UP;

            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[0]),
                    spannedString.getSpanEnd(spans[0]), termsOfUseUrl, HYPERLINK_TEXT_COLOR);

            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[1]),
                    spannedString.getSpanEnd(spans[1]), allTermsOfUseUrl, HYPERLINK_TEXT_COLOR);

            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[2]),
                    spannedString.getSpanEnd(spans[2]), privacyInfoUrl, HYPERLINK_TEXT_COLOR);
        }
    }

    private void authWorkflowStateChanged(AuthWorkflowData loginData) {
        switch (loginData.getAuthState()) {
            case Auth_Provider_Auth_Started:
                updateSpinnerVisibility(View.VISIBLE);
                break;
            case Auth_Provider_Authorized:
                mAlexaPropertyManager.updateAlexaProperty(WAKEWORD_ENABLED, "true")
                        .doOnSuccess((succeeded) -> {
                            if (!succeeded) {
                                Log.d(TAG, " Wakeword disable failed ");
                            }
                        })
                        .subscribe();
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.PREVIEW_MODE_ENABLED));
                break;
            case Auth_Provider_Authorization_Error:
                EventBus.getDefault().post(new WorkflowMessage(SETUP_ERROR));
                break;
        }
    }

    private void updateSpinnerVisibility(int visible) {
        View view = requireView();
        ProgressBar spinner = view.findViewById(R.id.enable_preview_mode_progress_spinner);
        spinner.setVisibility(visible);
        TextView enablePreviewModeButton = view.findViewById(R.id.enable_preview_mode_action_button);
        toggleViewVisibility(enablePreviewModeButton, visible);
    }
}
