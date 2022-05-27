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
package com.amazon.alexa.auto.comms.ui.dependencies;

import android.content.Context;
import android.telecom.TelecomManager;

import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects for Alexa communication.
 */
@Module
public class CommunicationModule {
    @Provides
    @Singleton
    public BluetoothDirectiveHandler provideCommunicationDirectiveHandler(WeakReference<Context> context) {
        return new BluetoothDirectiveHandler(context);
    }

    @Provides
    @Singleton
    public TelecomManager provideTelecomManager(WeakReference<Context> context) {
        return (TelecomManager) context.get().getSystemService(Context.TELECOM_SERVICE);
    }
}
