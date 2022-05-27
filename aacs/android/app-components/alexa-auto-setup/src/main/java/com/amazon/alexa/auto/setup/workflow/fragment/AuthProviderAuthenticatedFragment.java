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
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.setup.R;

/**
 * Fragment for displaying authorization provider login finish screen. It also provides the option to
 * switch to sign in with CBL.
 */
public class AuthProviderAuthenticatedFragment extends Fragment {
    private static final String TAG = AuthProviderAuthenticatedFragment.class.getSimpleName();

    LoginViewModel mViewModel;
    NavController mController;
    AlexaApp mApp;

    /**
     * Constructs an instance of AuthProviderLoginFinishFragment.
     */
    public AuthProviderAuthenticatedFragment() {}

    /**
     * Constructs an instance of AuthProviderLoginFinishFragment.
     *
     * @param viewModel View Model.
     * @param application Application object from where the fragment will
     *                    fetch dependencies.
     */
    @VisibleForTesting
    AuthProviderAuthenticatedFragment(@NonNull LoginViewModel viewModel, @NonNull Application application) {
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
        return inflater.inflate(R.layout.auth_provider_login_finished, container, false);
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
        loginFinishHeadingText.setText(String.format(format, ""));

        mController = findNavController(fragmentView);

        TextView finishLoginButtonText = fragmentView.findViewById(R.id.auth_provider_login_finished_btn);
        finishLoginButtonText.setOnClickListener(view -> mViewModel.userFinishedLogin());

        TextView signIndButtonText = fragmentView.findViewById(R.id.sign_in_action_button);
        signIndButtonText.setOnClickListener(view -> mViewModel.userSwitchedLogin(AuthMode.CBL_AUTHORIZATION));

        // Setup steps are completed and showing the finish screen.
        mViewModel.setupCompleted();
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
