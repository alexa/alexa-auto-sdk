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

public class ViewHolderWeatherTemplate extends RecyclerView.ViewHolder {
    private final TextView mMainTitle;
    private final TextView mSubTitle;
    private final ImageView mCurrentWeatherIcon;
    private final TextView mCurrentWeather;
    private final TextView mHighTemp;
    private final TextView mLowTemp;
    private final View[] mForecasts = new View[5];

    ViewHolderWeatherTemplate(View v) {
        super(v);
        mMainTitle = v.findViewById(R.id.mainTitle);
        mSubTitle = v.findViewById(R.id.subTitle);
        mCurrentWeatherIcon = v.findViewById(R.id.currentWeatherIcon);
        mCurrentWeather = v.findViewById(R.id.currentWeather);
        mHighTemp = v.findViewById(R.id.highTempCurrent);
        mLowTemp = v.findViewById(R.id.lowTempCurrent);
        mForecasts[0] = v.findViewById(R.id.forecast0);
        mForecasts[1] = v.findViewById(R.id.forecast1);
        mForecasts[2] = v.findViewById(R.id.forecast2);
        mForecasts[3] = v.findViewById(R.id.forecast3);
        mForecasts[4] = v.findViewById(R.id.forecast4);
    }

    public TextView getMainTitle() {
        return mMainTitle;
    }
    public TextView getSubTitle() {
        return mSubTitle;
    }
    public ImageView getCurrentWeatherIcon() {
        return mCurrentWeatherIcon;
    }
    public TextView getCurrentWeather() {
        return mCurrentWeather;
    }
    public TextView getHighTemp() {
        return mHighTemp;
    }
    public TextView getLowTemp() {
        return mLowTemp;
    }
    public View getForecast(int number) {
        return mForecasts[number];
    }
}
