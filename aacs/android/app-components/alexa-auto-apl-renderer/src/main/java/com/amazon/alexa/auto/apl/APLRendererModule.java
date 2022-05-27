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
package com.amazon.alexa.auto.apl;

import android.content.Context;
import android.util.Log;

import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.apl.APLVisualController;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.apl.handler.APLHandler;

import java.lang.ref.WeakReference;

public class APLRendererModule implements ModuleInterface {
    private static final String TAG = APLRendererModule.class.getSimpleName();

    @Override
    public void initialize(Context context) {
        Log.d(TAG, "initialize");
        WeakReference<Context> contextWk = new WeakReference<>(context);
        AlexaApp.from(context).getRootComponent().activateScope(
                new APLHandler(contextWk, new AACSMessageSender(contextWk, new AACSSender())));
    }

    @Override
    public void uninitialize(Context context) {
        Log.d(TAG, "uninitialize");
        AlexaApp.from(context).getRootComponent().deactivateScope(APLVisualController.class);
    }
}