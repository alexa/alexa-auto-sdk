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
package com.amazon.alexa.auto.templateruntime.dependencies;

import android.content.Context;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;

import java.lang.ref.WeakReference;

public interface TemplateDirectiveHandler {
    public static final int CLEAR_TEMPLATE_DELAY_MS = 8000;

    public void renderTemplate(AACSMessage message);

    static void clearTemplateAndEndVoiceActivity(AACSMessageSender mMessageSender, WeakReference<Context> mContext){
        AACSMessageBuilder.buildMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "")
                .ifPresent(message -> {
                    mMessageSender.sendMessage(Topic.ALEXA_CLIENT, Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "");
                });
        AlexaApp.from(mContext.get())
                .getRootComponent()
                .getComponent(SessionViewController.class)
                .ifPresent(SessionViewController::clearTemplate);
    }

    static void clearTemplate(WeakReference<Context> mContext){
        AlexaApp.from(mContext.get())
                .getRootComponent()
                .getComponent(SessionViewController.class)
                .ifPresent(SessionViewController::clearTemplate);
    }
}
