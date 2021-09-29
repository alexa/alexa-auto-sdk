package com.amazon.alexa.auto.setup.workflow.fragment;

import android.app.Application;
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

import static com.amazon.alexa.auto.app.common.util.ViewUtils.toggleViewVisibility;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

/**
 * Fragment for displaying Login screen and different options to login.
 */
public class LoginFragment extends Fragment {
    private static final String TAG = LoginFragment.class.getSimpleName();

    LoginViewModel mViewModel;
    QRCodeGenerator mQRCodeGenerator;
    AlexaApp mApp;
    NavController mNavController;
    private Handler mHandler;

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
        mNavController = findNavController(fragmentView);

        TextView signInButtonView = fragmentView.findViewById(R.id.sign_in_action_button);
        signInButtonView.setOnClickListener(view -> mViewModel.startLogin());

        ProgressBar spinner = fragmentView.findViewById(R.id.login_progress_spinner);
        spinner.getIndeterminateDrawable().setColorFilter(
                ResourcesCompat.getColor(getResources(), R.color.Cyan, null), PorterDuff.Mode.MULTIPLY);

        if (isPreviewModeEnabled(getContext())) {
            TextView tryAlexaButtonView = fragmentView.findViewById(R.id.try_alexa_action_button);
            tryAlexaButtonView.setVisibility(View.VISIBLE);
            tryAlexaButtonView.setOnClickListener(view -> {
                updateSpinnerVisibility(View.VISIBLE);
                Handler handler = new Handler();
                handler.postDelayed(() ->
                        mNavController.navigate(R.id.navigation_fragment_enablePreviewMode),
                        2000);
            });
        }
    }

    private void authWorkflowStateChanged(AuthWorkflowData loginData) {
        switch (loginData.getAuthState()) {
            case CBL_Auth_Started:
                updateQRCodeContainerVisibility(View.GONE);
                updateSpinnerVisibility(View.VISIBLE);
                updateLoginInContainerVisibility(View.VISIBLE);
                break;
            case CBL_Auth_CodePair_Received:
                Preconditions.checkNotNull(loginData.getCodePair());

                updateQRCodeContainerVisibility(View.VISIBLE);
                updateSpinnerVisibility(View.GONE);
                updateLoginInContainerVisibility(View.GONE);
                updateCBLCodePair(loginData.getCodePair());
                break;
            case CBL_Auth_Finished:
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
                break;
            case CBL_Auth_Start_Failed:
                EventBus.getDefault().post(new WorkflowMessage(SETUP_ERROR));
                break;
        }
    }

    private void updateCBLCodePair(CodePair codePair) {
        String URL_PARAM = "?cbl-code=";
        Bitmap qrCodeBitmap =
                mQRCodeGenerator.generateQRCode(codePair.getValidationUrl() + URL_PARAM + codePair.getValidationCode());
        showQRCodeScreen(codePair.getValidationCode(), qrCodeBitmap);
    }

    private void updateSpinnerVisibility(int visible) {
        View view = requireView();
        ProgressBar spinner = view.findViewById(R.id.login_progress_spinner);
        spinner.setVisibility(visible);
        TextView signInButton = view.findViewById(R.id.sign_in_action_button);
        toggleViewVisibility(signInButton, visible);
        if (isPreviewModeEnabled(getContext())) {
            TextView tryAlexaButton = view.findViewById(R.id.try_alexa_action_button);
            toggleViewVisibility(tryAlexaButton, visible);
        }
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

    private void setContentForCBLViewTitle() {
        View view = requireView();

        // Make multicolor text for title.
        TextView titleTextView = view.findViewById(R.id.qr_code_title_textview);
        String titleText = getResources().getString(R.string.login_qr_code_message);
        Spanned spanned = Html.fromHtml(titleText, Html.FROM_HTML_MODE_COMPACT);
        titleTextView.setText(spanned);
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }

    @VisibleForTesting
    boolean isPreviewModeEnabled(@NonNull Context context) {
        return ModuleProvider.isPreviewModeEnabled(context);
    }

}
