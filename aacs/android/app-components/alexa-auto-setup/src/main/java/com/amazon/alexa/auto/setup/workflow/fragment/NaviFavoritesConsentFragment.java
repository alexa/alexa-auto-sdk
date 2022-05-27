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

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.NAVI_FAVORITES_CONSENT_COMPLETED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.DISABLED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.ENABLED;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.model.UserConsent;

import org.greenrobot.eventbus.EventBus;

public class NaviFavoritesConsentFragment extends Fragment {
    private static final String TAG = NaviFavoritesConsentFragment.class.getSimpleName();

    /**
     * Constructs an instance of LocationConsentFragment.
     */
    public NaviFavoritesConsentFragment() {}

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.navi_favorites_consent, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();

        TextView skipButtonView = fragmentView.findViewById(R.id.setup_skip_button_view);
        skipButtonView.setOnClickListener(view -> { updatePropertyAndNavigate(DISABLED); });
        TextView useNaviFavoritesButtonView = fragmentView.findViewById(R.id.use_nav_favorites_button_view);
        useNaviFavoritesButtonView.setOnClickListener(view -> { updatePropertyAndNavigate(ENABLED); });

        if (ModuleProvider.isAlexaCustomAssistantEnabled(fragmentView.getContext())) {
            ImageView alexaImage = fragmentView.findViewById(R.id.alexa_img_view);
            alexaImage.setVisibility(View.GONE);

            TextView naviFavoritesPermissionText = fragmentView.findViewById(R.id.nav_favorites_permission_text_view);
            naviFavoritesPermissionText.setText(R.string.nav_favorites_permission_body_text);

            TextView hintText = fragmentView.findViewById(R.id.alexa_hint2);
            hintText.setVisibility(View.GONE);
        }
    }

    private void updatePropertyAndNavigate(UserConsent value) {
        Log.d(TAG, "Changing Navi Favorites setting to:" + value.name());
        NaviFavoritesSettingsProvider.updateNavFavoritesSetting(getContext(), value == ENABLED);
        EventBus.getDefault().post(new WorkflowMessage(NAVI_FAVORITES_CONSENT_COMPLETED));
    }
}
