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

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.annotation.VisibleForTesting;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.PagerSnapHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SnapHelper;

import com.amazon.aacsconstants.TemplateRuntimeConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.aacs.common.WeatherTemplate;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.app.common.ui.CirclePageIndicatorDecoration;
import com.amazon.alexa.auto.templateruntime.R;
import com.amazon.alexa.auto.templateruntime.dependencies.TemplateDirectiveHandler;
import com.amazon.alexa.auto.templateruntime.receiver.AlexaVoiceoverCompletedMessage;
import com.squareup.picasso.Picasso;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.jetbrains.annotations.NotNull;

import java.lang.ref.WeakReference;

/**
 * Handles rendering and clearing of Weather templates
 */
public class WeatherDirectiveHandler implements TemplateDirectiveHandler {
    private static final String TAG = WeatherDirectiveHandler.class.getSimpleName();
    private static final Handler mHandler = new Handler();
    private final WeakReference<Context> mContext;
    private long voiceOverEndTime;
    private AACSMessageSender mMessageSender;
    private AACSSender mAACSSender;

    public WeatherDirectiveHandler(Context context) {
        mContext = new WeakReference<Context>(context);
        voiceOverEndTime = Long.MAX_VALUE;
        mAACSSender = new AACSSender();
        mMessageSender = new AACSMessageSender(mContext, mAACSSender);
    }

    /**
     * Unpacks {@link AACSMessage} to get weather data and launches a view.
     *
     * @param message aacs template runtime intent.
     */
    @Override
    public void renderTemplate(AACSMessage message) {
        TemplateDirectiveHandler.clearTemplate(mContext);
        AlexaApp app = AlexaApp.from(mContext.get());
        app.getRootComponent().getComponent(SessionViewController.class).ifPresent(sessionViewController -> {
            sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                TemplateRuntimeMessages.parseWeatherTemplate(message.payload).ifPresent(weatherTemplate -> {
                    try {
                        renderWeatherView(viewGroup, weatherTemplate);
                        sessionViewController.setTemplateDisplayed();
                        EventBus.getDefault().register(this);
                    } catch (Exception e) {
                        Log.e(TAG, "Issue inflating template: " + e);
                    }
                });
            });
        });
    }

    /**
     * Unpacks the weatherTemplate object and renders weather template
     */
    @VisibleForTesting
    void renderWeatherView(ViewGroup viewGroup, WeatherTemplate weatherTemplate) {
        LayoutInflater layoutInflater =
                (LayoutInflater) mContext.get().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        viewGroup.addOnAttachStateChangeListener(getOnAttachStateChangeListener());

        View inflatedView = layoutInflater.inflate(R.layout.weather, null);
        inflatedView.setId(R.id.template_weather_view);

        viewGroup.addView(inflatedView, LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);

        WeatherAdapter adapter = new WeatherAdapter(weatherTemplate);
        LinearLayoutManager layoutManager =
                new LinearLayoutManager(mContext.get(), LinearLayoutManager.HORIZONTAL, false);

        removeClearTemplateCallback();
        RecyclerView recyclerView = inflatedView.findViewById(R.id.weather_recycler_view);
        recyclerView.setLayoutManager(layoutManager);
        SnapHelper snapHelper = new PagerSnapHelper();
        snapHelper.attachToRecyclerView(recyclerView);
        recyclerView.addItemDecoration(new CirclePageIndicatorDecoration());
        recyclerView.setAdapter(adapter);
        recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            public void onScrollStateChanged(@NotNull RecyclerView recyclerView, int newState) {
                long screenScrolledAtTime = System.currentTimeMillis();
                if (newState == RecyclerView.SCROLL_STATE_DRAGGING && screenScrolledAtTime > voiceOverEndTime) {
                    startClearTemplateDelay();
                }
            }
        });

        ImageView closeButton = inflatedView.findViewById(R.id.close_button);
        closeButton.setOnClickListener(v -> {
            TemplateDirectiveHandler.clearTemplateAndEndVoiceActivity(mMessageSender, mContext);
        });

        if (weatherTemplate.getSkillIcon() != null) {
            String skillIconUrl;
            try {
                skillIconUrl = weatherTemplate.getSkillIcon()
                        .getSources()
                        .stream()
                        .filter(source -> source.getSize().equals(TemplateRuntimeConstants.IMAGE_SIZE_SMALL))
                        .findFirst()
                        .get()
                        .getUrl();
            } catch (Exception e) {
                skillIconUrl = weatherTemplate.getSkillIcon().getSources().get(0).getUrl();
            }

            ImageView iconView = inflatedView.findViewById(R.id.weather_template_skill_icon);
            Picasso.get().load(skillIconUrl).into(iconView);
        }

    }

    /**
     * Listener for when the view is cleared from screen
     *
     * @return Returns listener which clears existing callbacks and event subscribers if any
     */
    private View.OnAttachStateChangeListener getOnAttachStateChangeListener() {
        return new View.OnAttachStateChangeListener() {
            @Override
            public void onViewAttachedToWindow(View v) {}

            @Override
            public void onViewDetachedFromWindow(View v) {
                // clear existing callbacks and event subscribers, if any
                removeClearTemplateCallback();
                EventBus.getDefault().unregister(WeatherDirectiveHandler.this);
            }
        };
    }

    /**
     * Subscribe to event published by
     * {@link com.amazon.alexa.auto.templateruntime.receiver.AlexaStateChangeReceiver} when Alexa
     * has completed its voice response to the user's weather utterance
     *
     * @param message - Message published by event publisher
     */
    @Subscribe
    public void OnReceive(AlexaVoiceoverCompletedMessage message) {
        voiceOverEndTime = message.getCompletedAt();
        startClearTemplateDelay();
    }

    /**
     * Runnable to clear weather template from the screen.
     * By Default the delay is set to {@link #CLEAR_TEMPLATE_DELAY_MS} after the voice response is complete - {@link
     * #startClearTemplateDelay()} However if the user interacts (swipes left or right) with the display card, we extend
     * the delay by
     * {@link #CLEAR_TEMPLATE_DELAY_MS} everytime they swipe. This is done so that they have enough time to view
     * the display card after swiping.
     */
    Runnable clearWeatherTemplateRunnable = new Runnable() {
        @Override
        public void run() {
            AlexaApp.from(mContext.get())
                    .getRootComponent()
                    .getComponent(SessionViewController.class)
                    .ifPresent(sessionViewController -> {
                        sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                            if (viewGroup.findViewById(R.id.template_weather_view) != null) {
                                sessionViewController.clearTemplate();
                            }
                        });
                    });
        }
    };

    void startClearTemplateDelay() {
        Log.v(TAG, "Starting clear template delay");
        removeClearTemplateCallback();
        mHandler.postDelayed(clearWeatherTemplateRunnable, CLEAR_TEMPLATE_DELAY_MS);
    }

    public void removeClearTemplateCallback() {
        mHandler.removeCallbacksAndMessages(null);
    }
}
