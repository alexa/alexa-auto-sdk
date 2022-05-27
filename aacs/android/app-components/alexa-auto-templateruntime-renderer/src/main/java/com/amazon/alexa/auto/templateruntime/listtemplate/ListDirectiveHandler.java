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
package com.amazon.alexa.auto.templateruntime.listtemplate;

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.PagerSnapHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SnapHelper;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.ListTemplate;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.templateruntime.R;
import com.amazon.alexa.auto.templateruntime.dependencies.TemplateDirectiveHandler;
import com.amazon.alexa.auto.templateruntime.receiver.AlexaVoiceoverCompletedMessage;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.jetbrains.annotations.NotNull;

import java.lang.ref.WeakReference;

public class ListDirectiveHandler implements TemplateDirectiveHandler {
    private static final String TAG = ListDirectiveHandler.class.getSimpleName();
    private WeakReference<Context> mContext;
    private static final Handler mHandler = new Handler();
    private long voiceOverEndTime;

    public ListDirectiveHandler(Context context) {
        mContext = new WeakReference<Context>(context);
        voiceOverEndTime = Long.MAX_VALUE;
    }

    @Override
    public void renderTemplate(AACSMessage message) {
        AlexaApp app = AlexaApp.from(mContext.get());
        app.getRootComponent().getComponent(SessionViewController.class).ifPresent(sessionViewController -> {
            sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                TemplateRuntimeMessages.parseListTemplate(message.payload).ifPresent(listTemplate -> {
                    try {
                        renderListTemplateView(viewGroup, listTemplate);
                        sessionViewController.setTemplateDisplayed();
                        EventBus.getDefault().register(this);
                    } catch (Exception e) {
                        Log.e(TAG, "Issue inflating template: " + e);
                    }
                });
            });
        });
    }

    @Override
    public void clearTemplate() {
        Log.i(TAG, "clearTemplate");
        AlexaApp.from(mContext.get())
                .getRootComponent()
                .getComponent(SessionViewController.class)
                .ifPresent(SessionViewController::clearTemplate);
    }

    void renderListTemplateView(ViewGroup viewGroup, ListTemplate listTemplate) {
        LayoutInflater layoutInflater =
                (LayoutInflater) mContext.get().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        viewGroup.addOnAttachStateChangeListener(getOnAttachStateChangeListener());
        View inflatedView = layoutInflater.inflate(R.layout.list_template, null);
        inflatedView.setId(R.id.list_template_view);

        viewGroup.addView(inflatedView, LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);

        ListTemplateAdapter listTemplateAdapter = new ListTemplateAdapter(listTemplate);

        LinearLayoutManager layoutManager =
                new LinearLayoutManager(mContext.get(), LinearLayoutManager.VERTICAL, false);

        removeClearTemplateCallback();

        RecyclerView recyclerView = inflatedView.findViewById(R.id.list_template_recycler_view);
        recyclerView.setLayoutManager(layoutManager);
        SnapHelper snapHelper = new PagerSnapHelper();
        snapHelper.attachToRecyclerView(recyclerView);

        recyclerView.setAdapter(listTemplateAdapter);
        recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            public void onScrollStateChanged(@NotNull RecyclerView recyclerView, int newState) {
                long screenScrolledAtTime = System.currentTimeMillis();
                if (newState == RecyclerView.SCROLL_STATE_DRAGGING && screenScrolledAtTime > voiceOverEndTime) {
                    startClearTemplateDelay();
                }
            }
        });

        TextView mainTitleText = inflatedView.findViewById(R.id.list_template_card_mainTitle);
        mainTitleText.setText(listTemplate.getTitle().getMainTitle());

        TextView subTitleText = inflatedView.findViewById(R.id.list_template_card_subTitle);
        subTitleText.setText(listTemplate.getTitle().getSubTitle());
    }

    private View.OnAttachStateChangeListener getOnAttachStateChangeListener() {
        return new View.OnAttachStateChangeListener() {
            @Override
            public void onViewAttachedToWindow(View v) {}

            @Override
            public void onViewDetachedFromWindow(View v) {
                // clear existing callbacks and event subscribers, if any
                removeClearTemplateCallback();
                EventBus.getDefault().unregister(ListDirectiveHandler.this);
            }
        };
    }

    public void removeClearTemplateCallback() {
        mHandler.removeCallbacksAndMessages(null);
    }

    /**
     * Subscribe to event published by
     * {@link com.amazon.alexa.auto.templateruntime.receiver.AlexaStateChangeReceiver} when Alexa
     * has completed its voice response to the user's List utterance
     *
     * @param message - Message published by event publisher
     */
    @Subscribe
    public void OnReceive(AlexaVoiceoverCompletedMessage message) {
        voiceOverEndTime = message.getCompletedAt();
        startClearTemplateDelay();
    }

    void startClearTemplateDelay() {
        Log.i(TAG, "Starting clear template delay");
        removeClearTemplateCallback();
        mHandler.postDelayed(clearListTemplateRunnable, CLEAR_TEMPLATE_DELAY_MS);
    }

    Runnable clearListTemplateRunnable = new Runnable() {
        @Override
        public void run() {
            AlexaApp.from(mContext.get())
                    .getRootComponent()
                    .getComponent(SessionViewController.class)
                    .ifPresent(sessionViewController -> {
                        sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                            if (viewGroup.findViewById(R.id.list_template_view) != null) {
                                sessionViewController.clearTemplate();
                            }
                        });
                    });
        }
    };
}
