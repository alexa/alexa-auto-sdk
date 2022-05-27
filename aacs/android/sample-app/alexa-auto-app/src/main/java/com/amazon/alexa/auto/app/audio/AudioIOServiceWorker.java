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
package com.amazon.alexa.auto.app.audio;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;

import com.amazon.alexa.auto.aacs.common.AACSMessage;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import io.reactivex.rxjava3.core.Observable;

/**
 * An interface to represent a work performed by Audio IO service.
 * The service determines its own lifecycle by combining the state of all the
 * work items.
 */
public interface AudioIOServiceWorker {
    @Documented
    @Retention(RetentionPolicy.SOURCE)
    @IntDef({IDLE, WORKING})
    @interface WorkerState {}

    int IDLE = 0;
    int WORKING = 1;

    /**
     * Obtain a stream to listen to worker state.
     *
     * @return Stream of worker state.
     */
    @NonNull
    Observable<Integer> getWorkerState();
}
