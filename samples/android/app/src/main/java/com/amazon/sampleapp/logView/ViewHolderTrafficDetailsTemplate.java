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
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderTrafficDetailsTemplate extends RecyclerView.ViewHolder {
    private final TextView mMainTitle;
    private final TextView mSubTitle;
    private final TextView mLabel;
    private final TextView mAddress;
    private final TextView mTravelDistance;
    private final TextView mTravelTime;
    private final ImageView mCurrentTrafficConditionsIcon;
    private final ImageView mBackgroundImage;

    public ViewHolderTrafficDetailsTemplate(View v) {
        super(v);
        mMainTitle = v.findViewById(R.id.mainTitle);
        mSubTitle = v.findViewById(R.id.subTitle);
        mLabel = v.findViewById(R.id.label);
        mAddress = v.findViewById(R.id.address);
        mTravelDistance = v.findViewById(R.id.travelDistance);
        mTravelTime = v.findViewById(R.id.travelTime);
        mCurrentTrafficConditionsIcon = v.findViewById(R.id.currentTrafficConditionsIcon);
        mBackgroundImage = v.findViewById(R.id.backgroundImage);
    }

    public TextView getMainTitle() {
        return mMainTitle;
    }
    public TextView getSubTitle() {
        return mSubTitle;
    }
    public TextView getLabel() {
        return mLabel;
    }
    public TextView getAddress() {
        return mAddress;
    }
    public TextView getTravelDistance() {
        return mTravelDistance;
    }
    public TextView getTravelTime() {
        return mTravelTime;
    }
    public ImageView getCurrentTrafficConditionsIcon() {
        return mCurrentTrafficConditionsIcon;
    }
    public ImageView getBackgroundImage() {
        return mBackgroundImage;
    }
}
