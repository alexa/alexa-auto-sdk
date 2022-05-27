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
package com.amazon.alexa.auto.app.dependencies;

import android.content.Context;

import com.amazon.aacsipc.AACSReceiver;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.SpeechRecognizerMessages;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects to communicate with AACS.
 */
@Module
public class AACSModule {
    /**
     * Provides an instance of {@link AACSMessageSender}.
     *
     * @param context Android Context.
     * @param sender Raw sender object.
     * @return an instance of {@link AACSMessageSender}.
     */
    @Provides
    @AlexaAppScope
    public AACSMessageSender provideMessageSender(WeakReference<Context> context, AACSSender sender) {
        return new AACSMessageSender(context, sender);
    }

    /**
     * Provides an instance of {@link AACSSender}.
     *
     * @return An instance of {@link AACSSender}.
     */
    @Provides
    @AlexaAppScope
    public AACSSender provideAACSSender() {
        return new AACSSender();
    }

    /**
     * Provides an instance of {@link SpeechRecognizerMessages}.
     *
     * @param messageSender AACS Message Sender
     * @return an instance of {@link SpeechRecognizerMessages}.
     */
    @Provides
    @AlexaAppScope
    public SpeechRecognizerMessages provideSpeechRecognizerMessages(AACSMessageSender messageSender) {
        return new SpeechRecognizerMessages(messageSender);
    }

    /**
     * Provides a new instance of {@link AACSReceiver.Builder}.
     * This is resolved by dependency injector to facilitate testing.
     * Always create new instance and do not bind it to any scope.
     *
     * @return New instance of {@link AACSReceiver.Builder}.
     */
    @Provides
    public AACSReceiver.Builder provideReceiverBuilder() {
        return new AACSReceiver.Builder();
    }
}
