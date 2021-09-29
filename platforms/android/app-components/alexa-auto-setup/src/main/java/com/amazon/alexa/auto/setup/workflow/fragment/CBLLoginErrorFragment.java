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
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.setup.R;

/**
 * Fragment to display error screen when CBL login in fails
 */
public class CBLLoginErrorFragment extends Fragment {
    private static final String TAG = CBLLoginErrorFragment.class.getSimpleName();

    private AlexaApp mApp;
    private NavController mNavController;

    /**
     * Constructs an instance of CBLLoginErrorFragment.
     */
    public CBLLoginErrorFragment() {
    }

    @VisibleForTesting
    CBLLoginErrorFragment(@NonNull NavController navController, @NonNull Application application) {
        mNavController = navController;
        mApp = AlexaApp.from(application);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.cbl_login_error, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        if (mApp == null) {
            mApp = AlexaApp.from(getContext());
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();
        mNavController = findNavController(fragmentView);

        TextView tryAlexaButtonView = fragmentView.findViewById(R.id.cbl_retry_button);
        tryAlexaButtonView.setOnClickListener(view -> {
            mApp.getRootComponent()
                    .getComponent(AlexaSetupWorkflowController.class)
                    .ifPresent(alexaSetupWorkflowController -> {
                        alexaSetupWorkflowController.stopSetupWorkflow();
                        alexaSetupWorkflowController.startSetupWorkflow(getContext(),
                                mNavController, null);
                    });
        });
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }

}
