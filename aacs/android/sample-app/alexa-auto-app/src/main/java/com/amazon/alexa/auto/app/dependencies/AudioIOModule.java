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

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.app.audio.AudioInputHandler;
import com.amazon.alexa.auto.app.audio.AudioInputReader;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects for Audio I/O.
 */
@Module
public class AudioIOModule {
    /**
     * Provides an instance of {@link AudioInputHandler}.
     *
     * @return an instance of {@link AudioInputHandler}.
     */
    @Provides
    @ServiceScope
    public AudioInputHandler provideAudioInputHandler() {
        return new AudioInputHandler(new AudioInputReader());
    }
}
