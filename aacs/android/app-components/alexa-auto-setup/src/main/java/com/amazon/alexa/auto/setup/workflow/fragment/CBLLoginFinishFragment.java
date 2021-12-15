package com.amazon.alexa.auto.setup.workflow.fragment;

import android.app.Application;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.setup.R;

import java.lang.ref.WeakReference;

/**
 * Fragment for displaying CBL login finish screen.
 */
public class CBLLoginFinishFragment extends Fragment {
    private static final String TAG = CBLLoginFinishFragment.class.getSimpleName();

    private LoginViewModel mViewModel;
    private AlexaApp mApp;

    /**
     * Constructs an instance of CBLLoginFinishFragment.
     */
    public CBLLoginFinishFragment() {}

    /**
     * Constructs an instance of CBLLoginFinishFragment.
     *
     * @param viewModel View Model.
     * @param application Application object from where the fragment will
     *                    fetch dependencies.
     */
    @VisibleForTesting
    CBLLoginFinishFragment(@NonNull LoginViewModel viewModel, @NonNull Application application) {
        this.mViewModel = viewModel;
        mApp = AlexaApp.from(application);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(LoginViewModel.class);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.cbl_login_finished, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        if (mApp == null) { // It would be non-null for test injected dependencies.
            mApp = AlexaApp.from(getContext());
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();

        TextView loginFinishHeadingText = fragmentView.findViewById(R.id.login_finish_heading);
        String format = getResources().getString(R.string.login_finish_heading_text);

        String headingString = "";
        String userFirstNameName = mViewModel.getUserFirstName();
        if (userFirstNameName != null) {
            // Make sure user first name's first letter is always capital.
            userFirstNameName =
                    userFirstNameName.substring(0, 1).toUpperCase() + userFirstNameName.substring(1).toLowerCase();
            headingString = String.format(format, ", " + userFirstNameName);
        } else {
            headingString = String.format(format, "");
        }
        loginFinishHeadingText.setText(headingString);

        TextView loginCompletedButton = fragmentView.findViewById(R.id.cbl_login_finished_btn);
        loginCompletedButton.setOnClickListener(view -> { mViewModel.userFinishedLogin(); });

        // Setup steps are completed and sending setup complete event.
        mViewModel.setupCompleted();
        sendSetupCompleteEvent();
    }

    /**
     * Send setup complete event to AACS, it will trigger Alexa feature introduction VUI.
     */
    private void sendSetupCompleteEvent() {
        new AACSMessageSender(new WeakReference<>(getContext()), new AACSSender())
                .sendMessage(Topic.DEVICE_SETUP, Action.DeviceSetup.SETUP_COMPLETED, "");
    }
}
