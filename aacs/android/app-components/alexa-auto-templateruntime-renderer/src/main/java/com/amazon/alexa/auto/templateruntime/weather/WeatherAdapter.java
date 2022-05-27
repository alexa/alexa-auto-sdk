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
package com.amazon.alexa.auto.templateruntime.weather;

import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.alexa.auto.aacs.common.WeatherForecast;
import com.amazon.alexa.auto.aacs.common.WeatherTemplate;
import com.amazon.alexa.auto.templateruntime.R;
import com.squareup.picasso.Picasso;

import org.jetbrains.annotations.NotNull;

import java.util.Arrays;
import java.util.List;

public class WeatherAdapter extends RecyclerView.Adapter<WeatherAdapter.ViewHolder> {
    private static final String TAG = WeatherAdapter.class.getSimpleName();
    static final List<Integer> weatherForecastLayouts =
            Arrays.asList(R.id.weather_card_forecast_day1_layout, R.id.weather_card_forecast_day2_layout,
                    R.id.weather_card_forecast_day3_layout, R.id.weather_card_forecast_day4_layout);
    public static final int TOTAL_PAGES = 2;
    public static final int WEATHER_CURRENT_INDEX = 0;
    public static final int WEATHER_FORECAST_INDEX = 1;
    public static final int WEATHER_FORECAST_DAYS_COUNT = 4;
    public static final String IMAGE_SIZE_MEDIUM = "MEDIUM";
    public static final String IMAGE_SIZE_XL = "X-LARGE";
    public static final int COLOR_WHITE = Color.argb(255, 255, 255, 255);

    private final WeatherTemplate weatherTemplate;

    public WeatherAdapter(WeatherTemplate weatherTemplate) {
        this.weatherTemplate = weatherTemplate;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull @NotNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(viewType, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull @NotNull ViewHolder holder, int position) {
        switch (position) {
            case WEATHER_CURRENT_INDEX:
                populateCurrentWeatherData(holder);
                Log.d(TAG, "Populated current weather data on view");
                break;
            case WEATHER_FORECAST_INDEX:
                populateWeatherForecastData(holder);
                Log.d(TAG, "Populated weather forecast data on view");
                break;
            default:
                Log.d(TAG, "Invalid position: " + position);
        }
    }

    @Override
    public int getItemCount() {
        return TOTAL_PAGES;
    }

    @Override
    public int getItemViewType(int position) {
        switch (position) {
            case WEATHER_CURRENT_INDEX:
                return R.layout.weather_current;
            case WEATHER_FORECAST_INDEX:
                return R.layout.weather_forecast;
        }
        return -1;
    }

    private void populateCurrentWeatherData(@NotNull ViewHolder holder) {
        String weatherIconUrl = weatherTemplate.getCurrentWeatherIcon()
                                        .getSources()
                                        .stream()
                                        .filter(source -> source.getSize().equals(IMAGE_SIZE_MEDIUM))
                                        .findFirst()
                                        .get()
                                        .getDarkBackgroundUrl();
        Picasso.get().load(weatherIconUrl).into(holder.weatherIconView);
        holder.weatherIconView.setColorFilter(COLOR_WHITE);

        holder.currentTempText.setText(weatherTemplate.getCurrentWeather());

        String arrowUpUrl = weatherTemplate.getHighTemperature()
                                    .getArrow()
                                    .getSources()
                                    .stream()
                                    .filter(source -> source.getSize().equals(IMAGE_SIZE_MEDIUM))
                                    .findFirst()
                                    .get()
                                    .getDarkBackgroundUrl();
        Picasso.get().load(arrowUpUrl).into(holder.weatherArrowUpIconView);
        holder.weatherArrowUpIconView.setColorFilter(COLOR_WHITE);

        holder.highTempText.setText(weatherTemplate.getHighTemperature().getValue());

        String arrowDownUrl = weatherTemplate.getLowTemperature()
                                      .getArrow()
                                      .getSources()
                                      .stream()
                                      .filter(source -> source.getSize().equals(IMAGE_SIZE_MEDIUM))
                                      .findFirst()
                                      .get()
                                      .getDarkBackgroundUrl();
        Picasso.get().load(arrowDownUrl).into(holder.weatherArrowDownIconView);
        holder.weatherArrowDownIconView.setColorFilter(COLOR_WHITE);

        holder.lowTempText.setText(weatherTemplate.getLowTemperature().getValue());
    }

    private void populateWeatherForecastData(@NotNull ViewHolder holder) {
        for (int i = 0; i < WEATHER_FORECAST_DAYS_COUNT; i++) {
            View view = holder.itemView.findViewById(weatherForecastLayouts.get(i));
            List<WeatherForecast> weatherForecastList = weatherTemplate.getWeatherForecast();
            ImageView weatherForecastIconView = view.findViewById(R.id.weather_card_forecast_icon);
            String weatherForecastIconUrl = weatherForecastList.get(i)
                                                    .getImage()
                                                    .getSources()
                                                    .stream()
                                                    .filter(source -> source.getSize().equals(IMAGE_SIZE_XL))
                                                    .findFirst()
                                                    .get()
                                                    .getDarkBackgroundUrl();
            Picasso.get().load(weatherForecastIconUrl).into(weatherForecastIconView);
            weatherForecastIconView.setColorFilter(COLOR_WHITE);

            TextView weatherForecastDay = view.findViewById(R.id.weather_card_forecast_day);
            weatherForecastDay.setText(weatherForecastList.get(i).getDay());

            TextView weatherForecastTempHigh = view.findViewById(R.id.weather_card_forecast_temp_high);
            weatherForecastTempHigh.setText(weatherForecastList.get(i).getHighTemperature());

            TextView weatherForecastLowHigh = view.findViewById(R.id.weather_card_forecast_temp_low);
            weatherForecastLowHigh.setText(weatherForecastList.get(i).getLowTemperature());
        }
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        public ImageView weatherIconView;
        public TextView currentTempText;
        public ImageView weatherArrowUpIconView;
        public TextView highTempText;
        public ImageView weatherArrowDownIconView;
        public TextView lowTempText;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            weatherIconView = itemView.findViewById(R.id.weather_card_weather_icon);
            currentTempText = itemView.findViewById(R.id.weather_card_temp_current);
            weatherArrowUpIconView = itemView.findViewById(R.id.weather_card_arrow_up);
            highTempText = itemView.findViewById(R.id.weather_card_temp_high);
            weatherArrowDownIconView = itemView.findViewById(R.id.weather_card_arrow_down);
            lowTempText = itemView.findViewById(R.id.weather_card_temp_low);
        }
    }
}
