package com.amazon.alexa.auto.setup.workflow.fragment;

import android.graphics.Color;
import android.os.Bundle;
import android.os.LocaleList;
import android.text.Annotation;
import android.text.SpannedString;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import javax.inject.Inject;

import org.greenrobot.eventbus.EventBus;

import static com.amazon.alexa.auto.app.common.util.PopupDialogUtil.embedUrlInPopupDialog;
import static com.amazon.alexa.auto.app.common.util.ViewUtils.toggleViewVisibility;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;
import static com.amazon.alexa.auto.apps.common.util.LocaleUtil.getLocalizedDomain;
import static com.amazon.aacsconstants.AACSPropertyConstants.WAKEWORD_ENABLED;

/**
 * Fragment for enabling Preview Mode
 */
public class EnablePreviewModeFragment extends Fragment {
    private static final String TAG = EnablePreviewModeFragment.class.getSimpleName();

    private static final String CONDITIONS_OF_USE_URL =
            "https://www.%s/gp/help/customer/display.html?nodeId=201909000&pop-up=1";
    private static final String TERMS_OF_USE_URL =
            "https://www.%s/gp/help/customer/display.html?nodeId=201566380&pop-up=1";
    private static final String PRIVACY_INFO_URL
            = "https://www.%s/gp/help/customer/display.html?nodeId=468496&pop-up=1";

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
        mViewModel = mViewModel == null
                ? new ViewModelProvider(this).get(EnablePreviewModeViewModel.class)
                : mViewModel;
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
            embedUrlInPopupDialog(getContext(),
                    enablePreviewModeDisclaimer,
                    spannedString.getSpanStart(spans[0]),
                    spannedString.getSpanEnd(spans[0]),
                    String.format(CONDITIONS_OF_USE_URL, getLocalizedDomain(LocaleList.getDefault().get(0))),
                    Color.CYAN);

            embedUrlInPopupDialog(getContext(),
                    enablePreviewModeDisclaimer,
                    spannedString.getSpanStart(spans[1]),
                    spannedString.getSpanEnd(spans[1]),
                    String.format(TERMS_OF_USE_URL, getLocalizedDomain(LocaleList.getDefault().get(0))),
                    Color.CYAN);

            embedUrlInPopupDialog(getContext(),
                    enablePreviewModeDisclaimer,
                    spannedString.getSpanStart(spans[2]),
                    spannedString.getSpanEnd(spans[2]),
                    String.format(PRIVACY_INFO_URL, getLocalizedDomain(LocaleList.getDefault().get(0))),
                    Color.CYAN);
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

