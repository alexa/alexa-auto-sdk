package com.amazon.alexa.auto.login;

import android.graphics.Bitmap;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.text.Html;
import android.text.Spanned;
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

import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

/**
 * Fragment for displaying Login screen and different options to login.
 */
public class CBLLoginFragment extends Fragment {
    private static final String TAG = CBLLoginFragment.class.getSimpleName();

    private CBLLoginViewModel mViewModel;
    private QRCodeGenerator mQRCodeGenerator;

    /**
     * Constructs an instance of LoginFragment.
     */
    public CBLLoginFragment() {
        mQRCodeGenerator = new QRCodeGenerator();
    }

    /**
     * Constructs an instance of LoginFragment.
     *
     * @param viewModel View Model for Login.
     * @param generator QR Code Generator.
     */
    @VisibleForTesting
    CBLLoginFragment(@NonNull CBLLoginViewModel viewModel, @NonNull QRCodeGenerator generator) {
        this.mViewModel = viewModel;
        this.mQRCodeGenerator = generator;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(CBLLoginViewModel.class);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.login_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mViewModel.loginWorkflowState().observe(getViewLifecycleOwner(), this::cblWorkflowStateChanged);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setContentForCBLViewTitle();

        View fragmentView = requireView();

        TextView getStartedButtonText = fragmentView.findViewById(R.id.get_started_action_button);
        getStartedButtonText.setOnClickListener(view -> mViewModel.starLogin());

        TextView finishLoginButtonText = fragmentView.findViewById(R.id.login_finished_btn);
        finishLoginButtonText.setOnClickListener(view -> mViewModel.userFinishedLogin());

        ProgressBar spinner = fragmentView.findViewById(R.id.login_progress_spinner);
        spinner.getIndeterminateDrawable().setColorFilter(
                ResourcesCompat.getColor(getResources(), R.color.Cyan, null), PorterDuff.Mode.MULTIPLY);
    }

    private void cblWorkflowStateChanged(CBLAuthWorkflowData loginData) {
        switch (loginData.getAuthState()) {
            case CBL_Auth_Not_Started:
                updateLoginFinishedVisibility(View.GONE);
                updateQRCodeContainerVisibility(View.GONE);
                updateLoginInContainerVisibility(View.VISIBLE);
                break;
            case CBL_Auth_Started:
                updateLoginFinishedVisibility(View.GONE);
                updateQRCodeContainerVisibility(View.GONE);
                updateVisibilitySpinner(View.VISIBLE);
                updateLoginInContainerVisibility(View.VISIBLE);
                break;
            case CBL_Auth_CodePair_Received:
                Preconditions.checkNotNull(loginData.getCodePair());

                updateLoginFinishedVisibility(View.GONE);
                updateQRCodeContainerVisibility(View.VISIBLE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                updateCBLCodePair(loginData.getCodePair());
                break;
            case CBL_Auth_Finished:
                updateLoginFinishedVisibility(View.VISIBLE);
                updateQRCodeContainerVisibility(View.GONE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
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
        view.findViewById(R.id.login_start_layout).setVisibility(visible);
    }

    private void showQRCodeScreen(String cblCode, Bitmap qrCodeBitmap) {
        setCBLCodeText(cblCode);
        setQRCodeImage(qrCodeBitmap);
    }

    private void updateLoginFinishedVisibility(int visible) {
        View view = requireView();
        view.findViewById(R.id.login_finished_layout).setVisibility(visible);
    }

    private void setContentForCBLViewTitle() {
        View view = requireView();

        // Make multicolor text for title.
        TextView titleTextView = view.findViewById(R.id.qr_code_title_textview);
        String titleText = getResources().getString(R.string.login_qr_code_message);
        Spanned spanned = Html.fromHtml(titleText, Html.FROM_HTML_MODE_COMPACT);
        titleTextView.setText(spanned);
    }
}
