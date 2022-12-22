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
package com.amazon.alexa.auto.setup.workflow.command

import android.content.Context
import android.util.Log
import com.amazon.aacsconstants.AACSConstants
import com.amazon.aacsipc.AACSPinger
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.Future


class CheckAACSStatusCommand(context: Context) : Command(context) {
        override fun execute() {
                publishEvent(WorkflowMessage(LoginEvent.AACS_NOT_CONNECTED_EVENT))
                checkIfAACSStarted(context)
        }

        private fun checkIfAACSStarted(context: Context) {
                var aacsStarted: Boolean = false;

                val workerPool: ExecutorService = Executors.newSingleThreadExecutor();

                workerPool.submit {
                        var aacsPinger = AACSPinger(context, "com.amazon.alexaautoclientservice.ping");
                        var fut: Future<AACSPinger.AACSPingResponse>;
                        var response: AACSPinger.AACSPingResponse;

                        while (!aacsStarted) {
                                fut = aacsPinger.pingAACS();
                                response = fut.get();

                                if (response.hasResponse) {
                                        Log.d(TAG, "AACS STATE: " + response.AACSState);
                                        if ((AACSConstants.State.ENGINE_INITIALIZED.name==response.AACSState) || (AACSConstants.State.CONNECTED.name==response.AACSState)) {
                                                Log.i(TAG, "Publish Event: " + LoginEvent.AACS_CONNECTED_EVENT);
                                                publishEvent(WorkflowMessage(LoginEvent.AACS_CONNECTED_EVENT));
                                                aacsStarted = true;
                                        }
                                }
                        }

                }
        }
        companion object {
                private val TAG = CheckAACSStatusCommand::class.java.simpleName
        }
}
