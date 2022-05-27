package com.amazon.alexa.auto.templateruntime.weather;

import static com.amazon.alexa.auto.templateruntime.weather.WeatherAdapter.COLOR_WHITE;
import static com.amazon.alexa.auto.templateruntime.weather.WeatherAdapter.WEATHER_FORECAST_DAYS_COUNT;
import static com.amazon.alexa.auto.templateruntime.weather.WeatherAdapter.weatherForecastLayouts;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.times;

import android.app.Activity;
import android.content.Context;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.alexa.auto.aacs.common.AssetSource;
import com.amazon.alexa.auto.aacs.common.DisplayAsset;
import com.amazon.alexa.auto.aacs.common.Temperature;
import com.amazon.alexa.auto.aacs.common.WeatherForecast;
import com.amazon.alexa.auto.aacs.common.WeatherTemplate;
import com.amazon.alexa.auto.aacs.common.WeatherTitle;
import com.amazon.alexa.auto.templateruntime.R;
import com.squareup.picasso.Picasso;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;

import java.util.Arrays;
import java.util.Collections;

@RunWith(RobolectricTestRunner.class)
public class WeatherAdapterTest {
    public static final PorterDuffColorFilter COLOR_FILTER_WHITE =
            new PorterDuffColorFilter(COLOR_WHITE, PorterDuff.Mode.SRC_ATOP);
    private WeatherAdapter weatherAdapter;
    private WeatherTemplate weatherTemplate;
    private Activity mActivity;
    private Context mContext;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        mActivity = Robolectric.buildActivity(Activity.class).create().resume().get();
        mContext = mActivity.getApplicationContext();
        weatherTemplate = weatherTemplate();
        weatherAdapter = new WeatherAdapter(weatherTemplate);
    }

    @Test
    public void testWeatherTemplatePagination() {
        assertEquals(weatherAdapter.getItemCount(), WeatherAdapter.TOTAL_PAGES);
        assertEquals(weatherAdapter.getItemViewType(0), R.layout.weather_current);
        assertEquals(weatherAdapter.getItemViewType(1), R.layout.weather_forecast);
    }

    @Test
    public void testOnBindViewHolder_populatesCurrentWeatherDataCorrectly() {
        try (MockedStatic<Picasso> staticMock = Mockito.mockStatic(Picasso.class)) {
            staticMock.when(Picasso::get).thenReturn(new Picasso.Builder(mContext).build());
            LinearLayout parent = new LinearLayout(mContext);
            WeatherAdapter.ViewHolder viewHolder =
                    weatherAdapter.onCreateViewHolder(parent, weatherAdapter.getItemViewType(0));
            weatherAdapter.onBindViewHolder(viewHolder, 0);
            assertEquals(viewHolder.currentTempText.getText(), weatherTemplate.getCurrentWeather());
            assertEquals(viewHolder.weatherIconView.getColorFilter(), COLOR_FILTER_WHITE);
            assertEquals(viewHolder.highTempText.getText(), weatherTemplate.getHighTemperature().getValue());
            assertEquals(viewHolder.lowTempText.getText(), weatherTemplate.getLowTemperature().getValue());
            assertEquals(viewHolder.weatherArrowDownIconView.getColorFilter(), COLOR_FILTER_WHITE);
            assertEquals(viewHolder.weatherArrowUpIconView.getColorFilter(), COLOR_FILTER_WHITE);
            staticMock.verify(times(3), Picasso::get);
        }
    }

    @Test
    public void testOnBindViewHolder_populatesWeatherForecastDataCorrectly() {
        try (MockedStatic<Picasso> staticMock = Mockito.mockStatic(Picasso.class)) {
            staticMock.when(Picasso::get).thenReturn(new Picasso.Builder(mContext).build());
            LinearLayout parent = new LinearLayout(mContext);
            WeatherAdapter.ViewHolder viewHolder =
                    weatherAdapter.onCreateViewHolder(parent, weatherAdapter.getItemViewType(1));
            weatherAdapter.onBindViewHolder(viewHolder, 1);
            for (int i = 0; i < WEATHER_FORECAST_DAYS_COUNT; i++) {
                View view = viewHolder.itemView.findViewById(weatherForecastLayouts.get(i));
                ImageView weatherForecastIconView = view.findViewById(R.id.weather_card_forecast_icon);
                assertEquals(weatherForecastIconView.getColorFilter(), COLOR_FILTER_WHITE);
                TextView weatherForecastDay = view.findViewById(R.id.weather_card_forecast_day);
                assertEquals(weatherForecastDay.getText(), weatherTemplate.getWeatherForecast().get(i).getDay());
                TextView weatherForecastTempHigh = view.findViewById(R.id.weather_card_forecast_temp_high);
                assertEquals(weatherForecastTempHigh.getText(),
                        weatherTemplate.getWeatherForecast().get(i).getHighTemperature());
                TextView weatherForecastTempLow = view.findViewById(R.id.weather_card_forecast_temp_low);
                assertEquals(weatherForecastTempLow.getText(),
                        weatherTemplate.getWeatherForecast().get(i).getLowTemperature());
            }
            staticMock.verify(times(WEATHER_FORECAST_DAYS_COUNT), Picasso::get);
        }
    }

    private WeatherTemplate weatherTemplate() {
        return new WeatherTemplate("WeatherTemplate", "88ade3ee-dc44-42ab-a894-e613a4d5c9ab",
                new Temperature("61°",
                        new DisplayAsset("Down arrow",
                                Collections.singletonList(new AssetSource(88, "lowDarkUrl", "MEDIUM", 80, "lowUrl")))),
                new WeatherTitle("Wednesday, August 11, 2021", "San Francisco, CA"),
                new DisplayAsset("current weather",
                        Collections.singletonList(new AssetSource(90, "currentDarkUrl", "MEDIUM", 70, "currentUrl"))),
                "Clouds giving way to sun",
                new Temperature("71°",
                        new DisplayAsset("Up arrow",
                                Collections.singletonList(
                                        new AssetSource(88, "highDarkUrl", "MEDIUM", 80, "highUrl")))),
                "64°",
                Arrays.asList(new WeatherForecast("69°", "Aug 12", "60°",
                                      new DisplayAsset("forecast",
                                              Collections.singletonList(new AssetSource(88, "forecastWeatherDarkUrl",
                                                      "X-LARGE", 80, "forecastWeatherUrl"))),
                                      "Sun"),
                        new WeatherForecast("71°", "Aug 13", "61°",
                                new DisplayAsset("forecast",
                                        Collections.singletonList(new AssetSource(
                                                88, "forecastWeatherDarkUrl", "X-LARGE", 80, "forecastWeatherUrl"))),
                                "Mon"),
                        new WeatherForecast("67°", "Aug 14", "56°",
                                new DisplayAsset("forecast",
                                        Collections.singletonList(new AssetSource(
                                                88, "forecastWeatherDarkUrl", "X-LARGE", 80, "forecastWeatherUrl"))),
                                "Tue"),
                        new WeatherForecast("69°", "Aug 15", "62°",
                                new DisplayAsset("forecast",
                                        Collections.singletonList(new AssetSource(
                                                88, "forecastWeatherDarkUrl", "X-LARGE", 80, "forecastWeatherUrl"))),
                                "Wed"))

        );
    }
}
