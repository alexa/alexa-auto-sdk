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
package com.amazon.alexa.auto.aacs_annotation_api;

/**
 * Implement this interface to observe the Service Lifecycle.
 */
@SuppressWarnings("unused")
public interface ILifecycleObserver {
    /**
     * Lifecycle owner's onCreate() method is called
     */
    int ON_CREATE = 0;

    /**
     * @hide This is not applicable for the Service, still adding it here for the future
     * Lifecycle owner's onStart() method is called
     */
    int ON_START = 1;

    /**
     * @hide This is not applicable for the Service, still adding it here for the future
     * Lifecycle owner's onResume() method is called
     */
    int ON_RESUME = 2;

    /**
     * @hide This is not applicable for the Service, still adding it here for the future
     * Lifecycle owner's onPause() method is called
     */
    int ON_PAUSE = 3;

    /**
     * @hide This is not applicable for the Service, still adding it here for the future
     * Lifecycle owner's onStop() method is called
     */
    int ON_STOP = 4;

    /**
     * Lifecycle owner's onDestroy() method is called
     */
    int ON_DESTROY = 5;

    /**
     * This is a callback method when life cycle events (like {@value ON_CREATE}, {@value ON_DESTROY}) occurs
     * Do not perform any blocking operations in the implementation. Use it for registration or un-registration like
     * activities
     * @param event {@value ON_CREATE}, {@value ON_DESTROY}
     */
    void onLifecycleEvent(int event);
}
