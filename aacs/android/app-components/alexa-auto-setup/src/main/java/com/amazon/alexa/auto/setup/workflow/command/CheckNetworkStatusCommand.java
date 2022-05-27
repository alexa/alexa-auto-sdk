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
package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

/**
 * Command to check network connectivity status and publish the workflow event based on the status.
 */
public class CheckNetworkStatusCommand extends Command {
    public CheckNetworkStatusCommand(Context context) {
        super(context);
    }

    @Override
    public void execute() {
        ConnectivityManager mConnectivityManager =
                (ConnectivityManager) getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo netInfo = mConnectivityManager.getActiveNetworkInfo();
        if (netInfo != null) {
            publishEvent(new WorkflowMessage(LoginEvent.NETWORK_CONNECTED_EVENT));
        } else {
            publishEvent(new WorkflowMessage(LoginEvent.NETWORK_DISCONNECTED_EVENT));
        }
    }
}
