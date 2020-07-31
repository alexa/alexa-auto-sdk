/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.logView;

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import com.amazon.sampleapp.R;

import java.util.ArrayList;

public class ViewHolderPreviousWaypointsTemplate extends RecyclerView.ViewHolder {
    private final TextView[] mNames = new TextView[5];
    private final TextView[] mAddresses = new TextView[5];

    public ViewHolderPreviousWaypointsTemplate(View v) {
        super(v);

        mNames[0] = v.findViewById(R.id.waypointName1);
        mNames[1] = v.findViewById(R.id.waypointName2);
        mNames[2] = v.findViewById(R.id.waypointName3);
        mNames[3] = v.findViewById(R.id.waypointName4);
        mNames[4] = v.findViewById(R.id.waypointName5);

        mAddresses[0] = v.findViewById(R.id.waypointAddress1);
        mAddresses[1] = v.findViewById(R.id.waypointAddress2);
        mAddresses[2] = v.findViewById(R.id.waypointAddress3);
        mAddresses[3] = v.findViewById(R.id.waypointAddress4);
        mAddresses[4] = v.findViewById(R.id.waypointAddress5);
    }

    public void setWaypoints(String[] names, String[] addresses) {
        for (int i = 0; i < 5; i++) {
            mNames[i].setText(names[i]);
            mAddresses[i].setText(addresses[i]);
        }
    }
}
