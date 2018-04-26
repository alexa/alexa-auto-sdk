/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.view;

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderWeatherTemplate extends RecyclerView.ViewHolder
{
    private TextView mainTitle;
    private TextView subTitle;
    private ImageView currentWeatherIcon;
    private TextView currentWeather;
    private TextView highTemp;
    private TextView lowTemp;
    private View[] forecasts = new View[5];

    ViewHolderWeatherTemplate(View v )
    {
        super( v );
        mainTitle = v.findViewById( R.id.mainTitle );
        subTitle = v.findViewById( R.id.subTitle );
        currentWeatherIcon = v.findViewById( R.id.currentWeatherIcon );
        currentWeather = v.findViewById( R.id.currentWeather );
        highTemp = v.findViewById( R.id.highTemp );
        lowTemp = v.findViewById( R.id.lowTemp );
        forecasts[0] = v.findViewById( R.id.forecast0 );
        forecasts[1] = v.findViewById( R.id.forecast1 );
        forecasts[2] = v.findViewById( R.id.forecast2 );
        forecasts[3] = v.findViewById( R.id.forecast3 );
        forecasts[4] = v.findViewById( R.id.forecast4 );
    }

    public TextView getMainTitle() { return mainTitle; }

    public TextView getSubTitle() { return subTitle; }

    public ImageView getCurrentWeatherIcon() { return currentWeatherIcon; }

    public TextView getCurrentWeather() { return currentWeather; }

    public TextView getHighTemp() { return highTemp; }

    public TextView getLowTemp() { return lowTemp; }

    public View getForecast (int number ){
        return forecasts[ number ];
    }

}
