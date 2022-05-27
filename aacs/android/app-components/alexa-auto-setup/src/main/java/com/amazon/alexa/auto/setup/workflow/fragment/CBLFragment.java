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
import static com.amazon.alexa.auto.apps.common.util.LocaleUtil.getLocalizedDomain;

import android.graphics.Bitmap;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.os.LocaleList;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.core.content.res.ResourcesCompat;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;
import com.amazon.alexa.auto.setup.workflow.util.QRCodeGenerator;

import org.greenrobot.eventbus.EventBus;

import javax.inject.Inject;

/**
 * Fragment for CBL display screen.
 */
public class CBLFragment extends Fragment {
    private static final String TAG = CBLFragment.class.getSimpleName();

    CBLViewModel mViewModel;
    QRCodeGenerator mQRCodeGenerator;
    NavController mNavController;

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    /**
     * Constructs an instance of CBLFragment.
     */
    public CBLFragment() {
        mQRCodeGenerator = new QRCodeGenerator();
    }

    /**
     * Constructs an instance of CBLFragment.
     *
     * @param viewModel View Model for CBL.
     * @param generator QR Code Generator.
     */
    @VisibleForTesting
    CBLFragment(@NonNull CBLViewModel viewModel, @NonNull QRCodeGenerator generator) {
        this.mViewModel = viewModel;
        this.mQRCodeGenerator = generator;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mAlexaPropertyManager == null) {
            DaggerSetupComponent.builder()
                    .androidModule(new AndroidModule(getContext()))
                    .build()
                    .injectCBLFragment(this);
        }

        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(CBLViewModel.class);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.cbl_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mViewModel.loginWorkflowState().observe(getViewLifecycleOwner(), this::authWorkflowStateChanged);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setContentForCBLViewTitle();

        View fragmentView = requireView();
        mNavController = findNavController(fragmentView);

        mViewModel.startLogin();

        ProgressBar spinner = fragmentView.findViewById(R.id.login_progress_spinner);
        spinner.getIndeterminateDrawable().setColorFilter(
                ResourcesCompat.getColor(getResources(), R.color.Cyan, null), PorterDuff.Mode.MULTIPLY);
    }

    private void authWorkflowStateChanged(AuthWorkflowData loginData) {
        switch (loginData.getAuthState()) {
            case CBL_Auth_Started:
                updateQRCodeContainerVisibility(View.GONE);
                updateVisibilitySpinner(View.VISIBLE);
                updateLoginInContainerVisibility(View.VISIBLE);
                break;
            case CBL_Auth_CodePair_Received:
                Preconditions.checkNotNull(loginData.getCodePair());

                updateQRCodeContainerVisibility(View.VISIBLE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                updateCBLCodePair(loginData.getCodePair());
                break;
            case CBL_Auth_Finished:
                updateCBLLoginFinishedVisibility(View.VISIBLE);
                updateQRCodeContainerVisibility(View.GONE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                mAlexaPropertyManager.updateAlexaProperty(WAKEWORD_ENABLED, "true")
                        .doOnSuccess((succeeded) -> {
                            if (!succeeded) {
                                Log.d(TAG, " Wakeword enable failed ");
                            }
                        })
                        .subscribe();
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
                break;
            case CBL_Auth_Start_Failed:
                mNavController.navigate(R.id.navigation_fragment_cblLoginError);
                break;
        }
    }

    private void updateCBLCodePair(CodePair codePair) {
        String URL_PARAM = "?cbl-code=";
        Bitmap qrCodeBitmap =
                mQRCodeGenerator.generateQRCode(codePair.getValidationUrl() + URL_PARAM + codePair.getValidationCode());
        showQRCodeScreen(codePair.getValidationCode(), qrCodeBitmap);
    }

    private void updateVisibilitySpinner(int visible) {
        View view = requireView();
        ProgressBar spinner = view.findViewById(R.id.login_progress_spinner);
        spinner.setVisibility(visible);
    }

    private void updateQRCodeContainerVisibility(int visible) {
        View view = requireView();
        view.findViewById(R.id.login_display_cbl_code_layout).setVisibility(visible);
    }

    private void setCBLCodeText(String cblCode) {
        View view = requireView();
        TextView cblCodeTextView = view.findViewById(R.id.cbl_code);
        cblCodeTextView.setText(cblCode);
    }

    private void setQRCodeImage(Bitmap qrCode) {
        View view = requireView();
        ImageView qrCodeImageView = view.findViewById(R.id.qr_code);
        qrCodeImageView.setImageBitmap(qrCode);
    }

    private void updateLoginInContainerVisibility(int visible) {
        View view = requireView();
        view.findViewById(R.id.cbl_loading_layout).setVisibility(visible);
    }

    private void showQRCodeScreen(String cblCode, Bitmap qrCodeBitmap) {
        setCBLCodeText(cblCode);
        setQRCodeImage(qrCodeBitmap);
    }

    private void updateCBLLoginFinishedVisibility(int visible) {
        View view = requireView();
        view.findViewById(R.id.cbl_login_finished_layout).setVisibility(visible);
    }

    private void setContentForCBLViewTitle() {
        View view = requireView();

        // Make multicolor text for title.
        TextView titleTextView = view.findViewById(R.id.qr_code_title_textview);
        String titleText = getResources().getString(R.string.login_qr_code_message);
        titleText = String.format(titleText, getLocalizedDomain(LocaleList.getDefault().get(0)) + "/code");
        Spanned spanned = Html.fromHtml(titleText, Html.FROM_HTML_MODE_COMPACT);
        titleTextView.setText(spanned);
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
