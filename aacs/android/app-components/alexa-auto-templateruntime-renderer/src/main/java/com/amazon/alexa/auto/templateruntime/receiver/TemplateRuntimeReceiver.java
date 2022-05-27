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
package com.amazon.alexa.auto.templateruntime.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.templateruntime.dependencies.AndroidModule;
import com.amazon.alexa.auto.templateruntime.dependencies.DaggerTemplateRuntimeComponent;
import com.amazon.alexa.auto.templateruntime.dependencies.HandlerFactory;
import com.amazon.alexa.auto.templateruntime.dependencies.TemplateDirectiveHandler;

import javax.inject.Inject;

/**
 * Receives Template Runtime messages, unpacks it and renders view
 */
public class TemplateRuntimeReceiver extends BroadcastReceiver {
    private static final String TAG = TemplateRuntimeReceiver.class.getSimpleName();

    @Inject
    HandlerFactory handlerFactory;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "onReceive: intent: " + intent.getAction());

        if (handlerFactory == null) {
            DaggerTemplateRuntimeComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectTemplateRuntimeReceiver(this);
        }

        if (intent != null && intent.getExtras() != null) {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                if (message.topic.equals(Topic.TEMPLATE_RUNTIME)
                        && message.action.equals(Action.TemplateRuntime.RENDER_TEMPLATE)) {
                    TemplateRuntimeMessages.getTemplateType(message.payload).ifPresent(type -> {
                        try {
                            TemplateDirectiveHandler templateDirectiveHandler =
                                    handlerFactory.getHandler(context, type);
                            templateDirectiveHandler.clearTemplate();
                            templateDirectiveHandler.renderTemplate(message);
                        } catch (IllegalArgumentException e) {
                            Log.i(TAG, "Unknown template type: " + type);
                        }
                    });
                }
            });
        }
    }
}
