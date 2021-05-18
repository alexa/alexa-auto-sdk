package com.amazon.alexa.auto.setup.workflow.fragment;

import static com.amazon.alexa.auto.apps.common.Constants.CBL_LOGIN_ALEXA_HINT_TEXT;
import static com.amazon.alexa.auto.apps.common.Constants.CBL_LOGIN_NONALEXA_HINT_TEXT;
import static com.amazon.alexa.auto.apps.common.Constants.LOGIN_FINISH_HEADING_TEXT;
import static com.amazon.alexa.auto.apps.common.Constants.LOGIN_TRY_TEXT;

import android.app.Application;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AlexaSetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.R;

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

        if (getArguments() != null) {
            String supportedFeature = getArguments().getString(ModuleProvider.MODULES);
            if (supportedFeature != null) {
                if (supportedFeature.contains(ModuleProvider.ModuleName.ALEXA_CUSTOM_ASSISTANT.name())) {
                    updateUIForAlexaCustomAssistant();
                }
            }
        }

        TextView getStartedButtonText = fragmentView.findViewById(R.id.cbl_login_finished_btn);
        getStartedButtonText.setOnClickListener(view -> { mViewModel.userFinishedLogin(); });

        // Setup steps are completed and showing the finish screen.
        mViewModel.setupCompleted();
    }

    @VisibleForTesting
    private void updateUIForAlexaCustomAssistant() {
        Log.d(TAG, "Update UI components for alexa custom assistant");
        View fragmentView = requireView();

        TextView loginFinishHeadingText = fragmentView.findViewById(R.id.login_finish_heading);
        TextView loginHintTryText = fragmentView.findViewById(R.id.login_hint_try);
        TextView assistantHint1Text = fragmentView.findViewById(R.id.alexa_hint1);
        TextView assistantHint2Text = fragmentView.findViewById(R.id.alexa_hint2);
        TextView assistantHint3Text = fragmentView.findViewById(R.id.alexa_hint3);

        mApp.getRootComponent().getComponent(AlexaSetupProvider.class).ifPresent(alexaSetupProvider -> {
            loginFinishHeadingText.setText(alexaSetupProvider.getSetupResId(LOGIN_FINISH_HEADING_TEXT));
            loginHintTryText.setText(alexaSetupProvider.getSetupResId(LOGIN_TRY_TEXT));
            assistantHint1Text.setText(alexaSetupProvider.getSetupResId(CBL_LOGIN_ALEXA_HINT_TEXT));
            assistantHint2Text.setText(alexaSetupProvider.getSetupResId(CBL_LOGIN_NONALEXA_HINT_TEXT));
            assistantHint3Text.setVisibility(View.GONE);
        });
    }
}
