/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexalve;

import com.amazon.alexalve.ILVCClient;

/**
 * The {@link ILVCService} is the interface for the Alexa Auto SDK client application to communicate
 * with the {@link LocalVoiceControlService}. Registering the Auto SDK client as the
 * {@link ILVCClient} to the {@link ILVCService} is the means to link the application to the
 * {@link LocalVoiceControlService} after the application has successfully bound to the service.
 */
interface ILVCService {

    /**
     * Registers the Auto SDK client implementation with the {@link LocalVoiceControlService}.
     * The {@link LocalVoiceControlService} requires a client to be registered for successful
     * operation. Client registration is expected after a binding to the
     * {@link LocalVoiceControlService} is established and is required before the LVC service is
     * able to configure and start. See {@link ILVCClient} for details on the responsibility of
     * the registered client.
     *
     * Note: {@link ILVCService} supports only one registered {@link ILVCClient} at a time.
     *
     * @param client The Auto SDK client implementation to register with the
     *               {@link LocalVoiceControlService}
     */
    void registerClient(ILVCClient client);

    /**
     * Unregisters a previously registered client
     *
     * @param client The Auto SDK client implementation to unregister from the
     *               {@link LocalVoiceControlService}
     */
    void unregisterClient(ILVCClient client);

    /**
     * Notifies the {@link LocalVoiceControlService} to start when ready. This is an asynchronous
     * start that will be followed by the {@link ILVCClient#onStart} callback when the startup
     * sequence is complete.
     */
    void start();

    /**
     * Notifies the {@link LocalVoiceControlService} to stop. This is an asynchronous
     * stop that will be followed by the {@link ILVCClient#onStop} callback when the shutdown
     * sequence is complete.
     */
    void stop();
}
