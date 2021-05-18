package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.alexa.auto.apps.common.Constants.AUTH_PROVIDER_MESSAGE_TEXT;
import static com.amazon.alexa.auto.apps.common.Constants.LOGIN_ACTION_BUTTON_TEXT;

import android.app.Application;
import android.app.Dialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.core.content.res.ResourcesCompat;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AlexaSetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;
import com.amazon.alexa.auto.setup.workflow.util.QRCodeGenerator;

import org.greenrobot.eventbus.EventBus;

/**
 * Fragment for displaying Login screen and different options to login.
 */
public class LoginFragment extends Fragment {
    private static final String TAG = LoginFragment.class.getSimpleName();

    private static final int START_SERVICE_DELAY_MS = 15000;

    // URLs that are used to create webview for Alexa term of use and privacy notice (en-US).
    private static final String TERMS_OF_USE_URL =
            "https://www.amazon.com/gp/help/customer/display.html?nodeId=201809740&pop-up=1";
    private static final String PRIVACY_NOTICE_URL =
            "https://www.amazon.com/gp/help/customer/display.html?nodeId=468496&pop-up=1";

    private LoginViewModel mViewModel;
    private QRCodeGenerator mQRCodeGenerator;
    private Handler mHandler;
    private AlexaApp mApp;

    /**
     * Constructs an instance of LoginFragment.
     */
    public LoginFragment() {
        mQRCodeGenerator = new QRCodeGenerator();
    }

    /**
     * Constructs an instance of LoginFragment.
     *
     * @param viewModel View Model for Login.
     * @param generator QR Code Generator.
     * @param application Application object from where the fragment will
     *                    fetch dependencies.
     */
    @VisibleForTesting
    LoginFragment(
            @NonNull LoginViewModel viewModel, @NonNull QRCodeGenerator generator, @NonNull Application application) {
        this.mViewModel = viewModel;
        this.mQRCodeGenerator = generator;
        mApp = AlexaApp.from(application);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(LoginViewModel.class);
        }

        mHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.login_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        if (mApp == null) { // It would be non-null for test injected dependencies.
            mApp = AlexaApp.from(getContext());
        }

        mViewModel.loginWorkflowState().observe(getViewLifecycleOwner(), this::authWorkflowStateChanged);

        // User can finish CBL login with phone before using head unit app, we want to subscribe CBL auth
        // status when login screen is shown and decide whether skipping CBL login flow or not.
        mViewModel.subscribeCBLAuthChange();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setContentForCBLViewTitle();

        View fragmentView = requireView();

        if (getArguments() != null) {
            String supportedFeature = getArguments().getString(ModuleProvider.MODULES);
            if (supportedFeature != null) {
                if (supportedFeature.contains(ModuleProvider.ModuleName.PREVIEW_MODE.name())) {
                    updateUIForPreviewMode();
                }
            }
        }

        TextView getStartedButtonText = fragmentView.findViewById(R.id.get_started_action_button);
        getStartedButtonText.setOnClickListener(view -> mViewModel.startLogin());

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

                updateLoginErrorMessageVisibility(View.GONE);
                updateQRCodeContainerVisibility(View.VISIBLE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                updateCBLCodePair(loginData.getCodePair());
                break;
            case CBL_Auth_Finished:
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
                break;
            case CBL_Auth_Start_Failed:
                updateLoginErrorMessageVisibility(View.VISIBLE);
                break;
            case Auth_Provider_Not_Authorized:
                updateLoginInContainerVisibility(View.VISIBLE);
                break;
            case Auth_Provider_Authorizing:
                updateLoginErrorMessageVisibility(View.GONE);
                updateQRCodeContainerVisibility(View.VISIBLE);
                updateVisibilitySpinner(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                break;
            case Auth_Provider_Authorized:
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.AUTH_PROVIDER_AUTH_FINISHED));
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

    private void updateLoginErrorMessageVisibility(int visible) {
        View view = requireView();
        view.findViewById(R.id.login_error_text_view).setVisibility(visible);

        TextView getStartedButton = (TextView) view.findViewById(R.id.get_started_action_button);
        getStartedButton.setText(R.string.login_action_retry_button);
    }

    private void setContentForCBLViewTitle() {
        View view = requireView();

        // Make multicolor text for title.
        TextView titleTextView = view.findViewById(R.id.qr_code_title_textview);
        String titleText = getResources().getString(R.string.login_qr_code_message);
        Spanned spanned = Html.fromHtml(titleText, Html.FROM_HTML_MODE_COMPACT);
        titleTextView.setText(spanned);
    }

    @VisibleForTesting
    void updateUIForPreviewMode() {
        Log.d(TAG, "Update UI components for preview mode");
        View fragmentView = requireView();

        TextView loginButtonText = fragmentView.findViewById(R.id.get_started_action_button);
        loginButtonText.setText(R.string.auth_provider_login_action_button);

        TextView signedInLaterButtonText = fragmentView.findViewById(R.id.skip_cbl_action_button);
        signedInLaterButtonText.setVisibility(View.VISIBLE);
        signedInLaterButtonText.setOnClickListener(view -> mViewModel.signInLater());

        LinearLayout buttonsLayout = fragmentView.findViewById(R.id.terms_of_use_buttons);
        buttonsLayout.setVisibility(View.VISIBLE);
        TextView previewModeText = fragmentView.findViewById(R.id.preview_mode_text_view);
        previewModeText.setVisibility(View.VISIBLE);

        Context context = getContext();
        Preconditions.checkNotNull(context);

        TextView termOfUseButtonText = fragmentView.findViewById(R.id.terms_of_use_button);
        termOfUseButtonText.setOnClickListener(view -> {
            final Dialog dialog = new Dialog(context);
            dialog.setContentView(R.layout.terms_and_privacy_dialog_layout);

            WebView webView = (WebView) dialog.findViewById(R.id.webView);
            webView.loadUrl(TERMS_OF_USE_URL);

            ImageButton closeButton = (ImageButton) dialog.findViewById(R.id.dialogButtonClose);
            closeButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                }
            });
            dialog.show();
        });

        TextView privacyNoticeButtonText = fragmentView.findViewById(R.id.privacy_notice_button);
        privacyNoticeButtonText.setOnClickListener(view -> {
            final Dialog dialog = new Dialog(context);
            dialog.setContentView(R.layout.terms_and_privacy_dialog_layout);

            WebView webView = (WebView) dialog.findViewById(R.id.webView);
            webView.loadUrl(PRIVACY_NOTICE_URL);

            ImageButton closeButton = (ImageButton) dialog.findViewById(R.id.dialogButtonClose);
            closeButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                }
            });
            dialog.show();
        });

        String extraModules = ModuleProvider.getModules(getContext());
        if (extraModules.contains(ModuleProvider.ModuleName.ALEXA_CUSTOM_ASSISTANT.name())) {
            mApp.getRootComponent().getComponent(AlexaSetupProvider.class).ifPresent(alexaSetupProvider -> {
                loginButtonText.setText(alexaSetupProvider.getSetupResId(LOGIN_ACTION_BUTTON_TEXT));
                previewModeText.setText(alexaSetupProvider.getSetupResId(AUTH_PROVIDER_MESSAGE_TEXT));
            });
        }
    }
}
