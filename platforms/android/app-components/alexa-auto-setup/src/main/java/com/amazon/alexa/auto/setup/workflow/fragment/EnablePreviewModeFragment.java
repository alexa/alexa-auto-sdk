package com.amazon.alexa.auto.setup.workflow.fragment;

import android.graphics.Color;
import android.os.Bundle;
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
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import static com.amazon.alexa.auto.app.common.util.PopupDialogUtil.embedUrlInPopupDialog;
import static com.amazon.alexa.auto.app.common.util.ViewUtils.toggleViewVisibility;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;

/**
 * Fragment for enabling Preview Mode
 */
public class EnablePreviewModeFragment extends Fragment {
    private static final String CONDITIONS_OF_USE_URL =
            "https://www.amazon.com/gp/help/customer/display.html?nodeId=201909000&pop-up=1";
    private static final String TERMS_OF_USE_URL =
            "https://www.amazon.com/gp/help/customer/display.html?nodeId=201566380&pop-up=1";
    private static final String PRIVACY_INFO_URL
            = "https://www.amazon.com/gp/help/customer/display.html?nodeId=468496&pop-up=1";
    public static final int CONDITIONS_OF_USE_LINK_START_INDEX = 169;
    public static final int CONDITIONS_OF_USE_LINK_END_INDEX = 186;
    public static final int TERMS_OF_USE_LINK_START_INDEX = 191;
    public static final int TERMS_OF_USE_END_INDEX = 215;
    public static final int PRIVACY_DISCLAIMER_LINK_START_INDEX = 220;
    public static final int PRIVACY_DISCLAIMER_LINK_END_INDEX = 271;

    private EnablePreviewModeViewModel mViewModel;

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
        embedUrlInPopupDialog(getContext(),
                enablePreviewModeDisclaimer,
                CONDITIONS_OF_USE_LINK_START_INDEX,
                CONDITIONS_OF_USE_LINK_END_INDEX,
                CONDITIONS_OF_USE_URL,
                Color.CYAN);

        embedUrlInPopupDialog(getContext(),
                enablePreviewModeDisclaimer,
                TERMS_OF_USE_LINK_START_INDEX,
                TERMS_OF_USE_END_INDEX,
                TERMS_OF_USE_URL,
                Color.CYAN);

        embedUrlInPopupDialog(getContext(),
                enablePreviewModeDisclaimer,
                PRIVACY_DISCLAIMER_LINK_START_INDEX,
                PRIVACY_DISCLAIMER_LINK_END_INDEX,
                PRIVACY_INFO_URL,
                Color.CYAN);
    }

    private void authWorkflowStateChanged(AuthWorkflowData loginData) {
        switch (loginData.getAuthState()) {
            case Auth_Provider_Auth_Started:
                updateSpinnerVisibility(View.VISIBLE);
                break;
            case Auth_Provider_Authorized:
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

