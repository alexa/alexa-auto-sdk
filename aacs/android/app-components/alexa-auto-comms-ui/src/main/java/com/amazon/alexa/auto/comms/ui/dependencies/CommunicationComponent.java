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

import com.amazon.alexa.auto.comms.ui.receiver.BluetoothReceiver;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Communication Dependencies.
 */
@Component(modules = {CommunicationModule.class, AndroidModule.class})
@Singleton
public interface CommunicationComponent {
    /**
     * Inject dependencies for @c BluetoothReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectBluetoothReceiver(BluetoothReceiver receiver);
}
