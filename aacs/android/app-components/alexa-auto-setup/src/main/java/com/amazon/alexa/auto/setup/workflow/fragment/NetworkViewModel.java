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
import android.content.Intent;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;

/**
 * ViewModel for @{link NetworkFragment}
 */
public class NetworkViewModel extends AndroidViewModel {
    private static final String TAG = NetworkViewModel.class.getSimpleName();

    /**
     * Constructor for NetworkViewModel.
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public NetworkViewModel(@NonNull Application application) {
        super(application);
    }

    public void goToNetworkSettings() {
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getApplication().getApplicationContext().startActivity(intent);
    }
}
