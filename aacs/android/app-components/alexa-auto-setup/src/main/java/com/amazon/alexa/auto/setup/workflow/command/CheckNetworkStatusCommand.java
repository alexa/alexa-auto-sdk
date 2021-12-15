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
