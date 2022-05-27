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
package com.amazon.alexa.auto.templateruntime.bodytemplate;

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.BodyTemplate;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;
import com.amazon.alexa.auto.templateruntime.R;
import com.amazon.alexa.auto.templateruntime.dependencies.TemplateDirectiveHandler;
import com.amazon.alexa.auto.templateruntime.receiver.AlexaVoiceoverCompletedMessage;
import com.squareup.picasso.Picasso;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;
import java.util.NoSuchElementException;

public class BodyTemplateDirectiveHandler implements TemplateDirectiveHandler {
    private static final String TAG = BodyTemplateDirectiveHandler.class.getSimpleName();
    private static final int MAX_DISPLAY_CHARS = 80;
    private static final String IMAGE_SIZE_LARGE = "LARGE";

    private WeakReference<Context> mContext;
    private static final Handler mHandler = new Handler();

    public BodyTemplateDirectiveHandler(Context context) {
        mContext = new WeakReference<Context>(context);
    }

    @Override
    public void renderTemplate(AACSMessage message) {
        AlexaApp app = AlexaApp.from(mContext.get());
        app.getRootComponent().getComponent(SessionViewController.class).ifPresent(sessionViewController -> {
            sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                TemplateRuntimeMessages.parseBodyTemplate(message.payload).ifPresent(bodyTemplate -> {
                    try {
                        renderBodyTemplateView(viewGroup, bodyTemplate, mContext);
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

    private void renderBodyTemplateView(
            ViewGroup viewGroup, BodyTemplate bodyTemplate, WeakReference<Context> mContext) {
        LayoutInflater layoutInflater =
                (LayoutInflater) mContext.get().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        viewGroup.addOnAttachStateChangeListener(getOnAttachStateChangeListener());

        View inflatedView = layoutInflater.inflate(R.layout.body_template, null);
        inflatedView.setId(R.id.body_template_view);

        viewGroup.addView(inflatedView, LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);

        removeClearTemplateCallback();

        TextView mainTitleText = inflatedView.findViewById(R.id.body_template_card_mainTitle);
        mainTitleText.setText(bodyTemplate.getTitle().getMainTitle());

        TextView subTitleText = inflatedView.findViewById(R.id.body_template_card_subTitle);
        subTitleText.setText(bodyTemplate.getTitle().getSubTitle());

        TextView textField;

        if (bodyTemplate.getImage() != null) {
            String imageUrl;
            try {
                imageUrl = bodyTemplate.getImage()
                                   .getSources()
                                   .stream()
                                   .filter(source -> source.getSize().equals(IMAGE_SIZE_LARGE))
                                   .findFirst()
                                   .get()
                                   .getUrl();
            } catch (NoSuchElementException e) {
                imageUrl = bodyTemplate.getImage().getSources().get(0).getUrl();
            }

            ImageView imageView = inflatedView.findViewById(R.id.body_template_card_image);
            Picasso.get().load(imageUrl).into(imageView);
            inflatedView.findViewById(R.id.body_template_card_text_field).setVisibility(View.GONE);
            textField = inflatedView.findViewById(R.id.body_template_card_text_field_image);
        } else {
            inflatedView.findViewById(R.id.body_template_card_image).setVisibility(View.GONE);
            inflatedView.findViewById(R.id.body_template_card_text_field_image).setVisibility(View.GONE);
            textField = inflatedView.findViewById(R.id.body_template_card_text_field);
        }

        textField.setText(getDisplayText(mContext, bodyTemplate.getTextField()));
    }

    private String getDisplayText(WeakReference<Context> mContext, String textFieldString) {
        if (textFieldString.length() <= MAX_DISPLAY_CHARS) {
            return textFieldString;
        } else {
            textFieldString = textFieldString.substring(0, MAX_DISPLAY_CHARS);
            if (textFieldString.charAt(MAX_DISPLAY_CHARS - 1) != ' ') {
                textFieldString = textFieldString.substring(0, textFieldString.lastIndexOf(" "));
            }
            return textFieldString + mContext.get().getResources().getString(R.string.ellipsis);
        }
    }

    private View.OnAttachStateChangeListener getOnAttachStateChangeListener() {
        return new View.OnAttachStateChangeListener() {
            @Override
            public void onViewAttachedToWindow(View v) {}

            @Override
            public void onViewDetachedFromWindow(View v) {
                // clear existing callbacks and event subscribers, if any
                removeClearTemplateCallback();
                EventBus.getDefault().unregister(BodyTemplateDirectiveHandler.this);
            }
        };
    }

    public void removeClearTemplateCallback() {
        mHandler.removeCallbacksAndMessages(null);
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
        startClearTemplateDelay();
    }

    private void startClearTemplateDelay() {
        Log.i(TAG, "Starting clear template delay");
        removeClearTemplateCallback();
        mHandler.postDelayed(clearBodyTemplateRunnable, CLEAR_TEMPLATE_DELAY_MS);
    }

    Runnable clearBodyTemplateRunnable = new Runnable() {
        @Override
        public void run() {
            AlexaApp.from(mContext.get())
                    .getRootComponent()
                    .getComponent(SessionViewController.class)
                    .ifPresent(sessionViewController -> {
                        sessionViewController.getTemplateRuntimeViewContainer().ifPresent(viewGroup -> {
                            if (viewGroup.findViewById(R.id.body_template_view) != null) {
                                sessionViewController.clearTemplate();
                            }
                        });
                    });
        }
    };
}
