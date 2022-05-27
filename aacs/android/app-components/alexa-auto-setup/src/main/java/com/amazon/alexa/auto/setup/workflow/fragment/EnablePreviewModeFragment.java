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
import static com.amazon.alexa.auto.apps.common.util.LocaleUtil.getLocalizedDomain;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

import android.graphics.Color;
import android.os.Bundle;
import android.os.LocaleList;
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

import java.util.Locale;

import javax.inject.Inject;

/**
 * Fragment for enabling Preview Mode
 */
public class EnablePreviewModeFragment extends Fragment {
    private static final String TAG = EnablePreviewModeFragment.class.getSimpleName();

    private static final String CONDITIONS_OF_USE_URL =
            "https://www.%s/gp/help/customer/display.html?nodeId=201909000&pop-up=1";
    private static final String TERMS_OF_USE_URL =
            "https://www.%s/gp/help/customer/display.html?nodeId=201566380&pop-up=1";
    private static final String PRIVACY_INFO_URL =
            "https://www.%s/gp/help/customer/display.html?nodeId=468496&pop-up=1";
    private static final String PRIVACY_INFO_URL_JA_JP =
            "https://www.%s/gp/help/customer/display.html?nodeId=643000&pop-up=1";

    private static final Locale JA_JP_LOCALE = new Locale("ja");

    private EnablePreviewModeViewModel mViewModel;

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

        mAlexaPropertyManager.updateAlexaLocaleWithPersistentConfig();
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
            Locale locale = LocaleList.getDefault().get(0);
            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[0]),
                    spannedString.getSpanEnd(spans[0]),
                    String.format(CONDITIONS_OF_USE_URL, getLocalizedDomain(locale)), Color.CYAN);

            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[1]),
                    spannedString.getSpanEnd(spans[1]), String.format(TERMS_OF_USE_URL, getLocalizedDomain(locale)),
                    Color.CYAN);

            String privacyInfoUrl = String.format(PRIVACY_INFO_URL, getLocalizedDomain(locale));

            if (JA_JP_LOCALE.getLanguage().equals(locale.getLanguage())) {
                privacyInfoUrl = String.format(PRIVACY_INFO_URL_JA_JP, getLocalizedDomain(locale));
            }

            embedUrlInPopupDialog(getContext(), enablePreviewModeDisclaimer, spannedString.getSpanStart(spans[2]),
                    spannedString.getSpanEnd(spans[2]), privacyInfoUrl, Color.CYAN);
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
