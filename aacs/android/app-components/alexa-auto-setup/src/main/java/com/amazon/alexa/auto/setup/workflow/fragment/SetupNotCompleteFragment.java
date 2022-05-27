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

import static com.amazon.alexa.auto.apps.common.Constants.ALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.ALEXA_AND_PTT;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA_AND_PTT;
import static com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent.ALEXA_ONLY;
import static com.amazon.alexa.auto.setup.workflow.event.VoiceAssistanceEvent.NON_ALEXA_ONLY;

import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.NavGraph;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupController;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.SetupProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.R;

/**
 * Fragment to support errors during the setup process
 */
public class SetupNotCompleteFragment extends Fragment {
    private static final String TAG = SetupNotCompleteFragment.class.getSimpleName();

    private NavController mNavController;
    private AlexaApp mApp;
    SetupController mSetupController;

    /**
     * Constructs an instance of SetupNotCompleteFragment.
     */
    public SetupNotCompleteFragment() {}

    @VisibleForTesting
    SetupNotCompleteFragment(@NonNull NavController navController, @NonNull Application application) {
        mNavController = navController;
        mApp = AlexaApp.from(application);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.setup_not_complete, container, false);
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

        if (ModuleProvider.isAlexaCustomAssistantEnabled(fragmentView.getContext())) {
            ImageView alexaImage = fragmentView.findViewById(R.id.alexa_img_view);
            alexaImage.setVisibility(View.GONE);

            TextView setupNotCompleteText = fragmentView.findViewById(R.id.setup_not_complete_text);
            setupNotCompleteText.setText(R.string.setup_not_complete_text_with_alexa_custom_assistant);
        }

        mNavController = findNavController(fragmentView);

        TextView tryAlexaButtonView = fragmentView.findViewById(R.id.setup_not_complete_retry_button);
        tryAlexaButtonView.setOnClickListener(view -> {
            if (ModuleProvider.isAlexaCustomAssistantEnabled(fragmentView.getContext())) {
                SetupProvider setupProvider = mApp.getRootComponent().getComponent(SetupProvider.class).get();
                NavGraph graph = mNavController.getNavInflater().inflate(setupProvider.getCustomSetupNavigationGraph());

                // get current setup flow here
                if (mSetupController == null) {
                    mSetupController = fetchVoiceAssistanceSetupController(fragmentView.getContext());
                }

                graph.setStartDestination(
                        setupProvider.getSetupWorkflowStartDestinationByKey(mSetupController.getCurrentSetupFlow()));
                mNavController.setGraph(graph);

                mApp.getRootComponent()
                        .getComponent(AlexaSetupWorkflowController.class)
                        .ifPresent(alexaSetupWorkflowController -> {
                            alexaSetupWorkflowController.stopSetupWorkflow();

                            switch (mSetupController.getCurrentSetupFlow()) {
                                case ALEXA:
                                case ALEXA_AND_PTT:
                                    alexaSetupWorkflowController.startSetupWorkflow(
                                            getContext(), mNavController, ALEXA_ONLY);
                                    break;
                                case NONALEXA:
                                case NONALEXA_AND_PTT:
                                    alexaSetupWorkflowController.startSetupWorkflow(
                                            getContext(), mNavController, NON_ALEXA_ONLY);
                                    break;
                                default:
                                    alexaSetupWorkflowController.startSetupWorkflow(getContext(), mNavController, null);
                            }
                        });
            } else {
                mApp.getRootComponent()
                        .getComponent(AlexaSetupWorkflowController.class)
                        .ifPresent(alexaSetupWorkflowController -> {
                            alexaSetupWorkflowController.stopSetupWorkflow();
                            alexaSetupWorkflowController.startSetupWorkflow(getContext(), mNavController, null);
                        });
            }
        });
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }

    @VisibleForTesting
    SetupController fetchVoiceAssistanceSetupController(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        return app.getRootComponent().getComponent(SetupController.class).orElse(null);
    }
}
