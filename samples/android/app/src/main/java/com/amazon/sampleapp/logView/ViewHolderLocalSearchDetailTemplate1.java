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

public class ViewHolderLocalSearchDetailTemplate1 extends RecyclerView.ViewHolder {
    private final TextView mMainTitle;
    private final TextView mSubTitle;
    private final TextView mAddress;
    private final TextView mPhoneNumber;
    private final TextView mProvider;
    private final ImageView mImage;
    private final TextView mTravelDistance;
    private final TextView mTravelTime;
    private final TextView mOffRouteTime;
    private final TextView mPriceRange;

    private final TextView mRatingValue;
    private final TextView mRatingReviewCount;
    private final ImageView mRatingImage;
    private final TextView mRatingProviderName;
    private final ImageView mRatingProviderImage;

    // not used
    // private final TextView mURL;
    // private final TextView mLatitudeInDegrees;
    // private final TextView mLongitudeInDegrees;

    private final TextView mCurrentStatus;
    // private final TextView mTravelDirection;

    // interpolated
    private final TextView mWeekdayHours;
    private final TextView mSaturdayHours;
    private final TextView mSundayHours;

    public ViewHolderLocalSearchDetailTemplate1(View v) {
        super(v);
        mMainTitle = v.findViewById(R.id.mainTitle);
        mSubTitle = v.findViewById(R.id.subTitle);
        mAddress = v.findViewById(R.id.address);
        mPhoneNumber = v.findViewById(R.id.phoneNumber);
        mProvider = v.findViewById(R.id.provider);
        mImage = v.findViewById(R.id.image);
        mTravelDistance = v.findViewById(R.id.travelDistance);
        mTravelTime = v.findViewById(R.id.travelTime);
        mOffRouteTime = v.findViewById(R.id.offRouteTime);
        mPriceRange = v.findViewById(R.id.priceRange);
        // mURL = v.findViewById( R.id.url );

        mRatingValue = v.findViewById(R.id.ratingValue);
        mRatingReviewCount = v.findViewById(R.id.ratingReviewCount);
        mRatingImage = v.findViewById(R.id.ratingImage);
        mRatingProviderName = v.findViewById(R.id.ratingProviderName);
        mRatingProviderImage = v.findViewById(R.id.ratingProviderImage);

        // mLatitudeInDegrees = v.findViewById( R.id.latitudeInDegrees );
        // mLongitudeInDegrees = v.findViewById( R.id.longitudeInDegrees );

        mCurrentStatus = v.findViewById(R.id.currentStatus);
        // mTravelDirection = v.findViewById( R.id.travelDirection );

        mWeekdayHours = v.findViewById(R.id.weekdayHours);
        mSaturdayHours = v.findViewById(R.id.saturdayHours);
        mSundayHours = v.findViewById(R.id.sundayHours);
    }

    public TextView getMainTitle() {
        return mMainTitle;
    }
    public TextView getSubTitle() {
        return mSubTitle;
    }
    public TextView getAddress() {
        return mAddress;
    }
    public TextView getPhoneNumber() {
        return mPhoneNumber;
    }
    public TextView getProvider() {
        return mProvider;
    }
    public ImageView getImage() {
        return mImage;
    }
    public TextView getTravelDistance() {
        return mTravelDistance;
    }
    public TextView getTravelTime() {
        return mTravelTime;
    }
    public TextView getOffRouteTime() {
        return mOffRouteTime;
    }
    public TextView getPriceRange() {
        return mPriceRange;
    }

    // public TextView getURL() { return mURL; }
    public TextView getRatingValue() {
        return mRatingValue;
    }
    public TextView getRatingReviewCount() {
        return mRatingReviewCount;
    }
    public ImageView getRatingImage() {
        return mRatingImage;
    }
    public TextView getRatingProviderName() {
        return mRatingProviderName;
    }
    public ImageView getRatingProviderImage() {
        return mRatingProviderImage;
    }
    // public TextView getLatitudeInDegrees() { return mLatitudeInDegrees; }
    // public TextView getLongitudeInDegrees() { return mLongitudeInDegrees; }
    public TextView getCurrentStatus() {
        return mCurrentStatus;
    }
    // public TextView getTravelDirection() { return mTravelDirection; }
    public TextView getWeekdayHours() {
        return mWeekdayHours;
    }
    public TextView getSaturdayHours() {
        return mSaturdayHours;
    }
    public TextView getSundayHours() {
        return mSundayHours;
    }
}
