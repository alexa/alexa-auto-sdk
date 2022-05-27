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
package com.amazon.alexa.auto.navigation.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TemplateRuntimeConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.navigation.dependencies.AndroidModule;
import com.amazon.alexa.auto.navigation.dependencies.DaggerNaviComponent;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;

import javax.inject.Inject;

/**
 * Broadcast receiver which gets POI directives.
 */
public class LocalSearchTemplateRuntimeReceiver extends BroadcastReceiver {
    private static final String TAG = LocalSearchTemplateRuntimeReceiver.class.getSimpleName();

    @Inject
    LocalSearchDirectiveHandler mLocalSearchDirectiveHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, this + " | onReceive: intent: " + intent);
        if (mLocalSearchDirectiveHandler == null) {
            Log.i(TAG, this + " | first onReceive so doing injection");
            DaggerNaviComponent.builder().androidModule(new AndroidModule(context)).build().injectPOIReceiver(this);
        }

        if (intent != null && intent.getExtras() != null) {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                if (message.action.equals(Action.TemplateRuntime.RENDER_TEMPLATE)) {
                    TemplateRuntimeMessages.getTemplateType(message.payload).ifPresent(type -> {
                        if (type.equals(TemplateRuntimeConstants.TEMPLATE_TYPE_LOCAL_SEARCH_LIST)) {
                            // clearing template in case there is one currently rendered
                            mLocalSearchDirectiveHandler.clearTemplate();
                            mLocalSearchDirectiveHandler.renderLocalSearchListTemplate(message);
                        } else if (type.equals(TemplateRuntimeConstants.TEMPLATE_TYPE_LOCAL_SEARCH_DETAIL)) {
                            // clearing template in case there is one currently rendered
                            mLocalSearchDirectiveHandler.clearTemplate();
                            mLocalSearchDirectiveHandler.renderLocalSearchDetailTemplate(message);
                        }
                    });
                } else if (message.action.equals(Action.TemplateRuntime.CLEAR_TEMPLATE)) {
                    mLocalSearchDirectiveHandler.clearLocalSearchTemplate();
                }
            });
        }
    }
}
