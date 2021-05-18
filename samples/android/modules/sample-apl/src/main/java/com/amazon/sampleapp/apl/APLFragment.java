/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.apl;

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;

import com.amazon.apl.android.render.APLPresenter;

/**
 * This fragment represent the UI element where APL will be rendered
 * in the sample app.
 */
public class APLFragment extends Fragment {
    Activity mActivity;

    public APLFragment(Activity activity) {
        super();

        this.mActivity = activity;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        //---------------------------------------------------------------------
        // Initialize the APL Runtime. This must be called during
        // Activity.onCreate() or prior to APLLayout inflation.
        //---------------------------------------------------------------------
        APLPresenter.initialize(mActivity);

        // Inflate the APLLayout view in this fragment
        View view = inflater.inflate(R.layout.apl_view, container, false);

        return view;
    }
}
