package com.amazon.alexa.auto.apps.common.aacs;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;

/**
 * Service Controller for AACS.
 */
public class AACSServiceController {
    private static final String TAG = AACSServiceController.class.getSimpleName();

    /**
     * Start Alexa Auto Client Service.
     *
     * @param context Android Context to start the service.
     * @param waitForNewConfig Signal AACS to use new configuration, which will
     *                         be sent after service start.
     */
    public static void startAACS(@NonNull Context context, boolean waitForNewConfig) {
        Log.i(TAG, "Sending start signal to Alexa Client Service.");
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(AACSConstants.AACS_PACKAGE_NAME, AACSConstants.AACS_CLASS_NAME));
        intent.setAction(Action.LAUNCH_SERVICE);
        intent.putExtra(AACSConstants.NEW_CONFIG, waitForNewConfig);

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }

    /**
     * Stop the Alexa Auto Client Service.
     *
     * @param context Android Context to stop the service.
     */
    public static void stopAACS(@NonNull Context context) {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(AACSConstants.AACS_PACKAGE_NAME, AACSConstants.AACS_CLASS_NAME));

        context.stopService(intent);
    }
}
