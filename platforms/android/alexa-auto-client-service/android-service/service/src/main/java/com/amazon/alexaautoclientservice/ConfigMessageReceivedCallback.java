package com.amazon.alexaautoclientservice;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSReceiver;
import com.amazon.alexaautoclientservice.AlexaAutoClientService.AACSStateMachine;
import com.amazon.alexaautoclientservice.AlexaAutoClientService.State;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONException;
import org.json.JSONObject;

public class ConfigMessageReceivedCallback implements AACSReceiver.MessageReceivedCallback {
    private static final String TAG = AACSConstants.AACS + "-" + ConfigMessageReceivedCallback.class.getSimpleName();
    private Context mContext;
    private AACSStateMachine mStateMachine;

    public ConfigMessageReceivedCallback(@NonNull Context context, @NonNull AACSStateMachine aacsStateMachine) {
        mContext = context;
        mStateMachine = aacsStateMachine;
    }

    @Override
    public void onMessageReceived(String s) {
        Log.i(TAG, "Configuration message received.");
        if (mStateMachine.getState() == State.STARTED) {
            try {
                JSONObject configMessage = new JSONObject(s);
                boolean configSaved = FileUtil.saveConfiguration(mContext,
                        configMessage.getJSONArray("configFilepaths"), configMessage.getJSONArray("configStrings"));

                if (configSaved) {
                    if (FileUtil.lvcEnabled()) {
                        mStateMachine.setState(State.WAIT_FOR_LVC_CONFIG);
                    } else {
                        mStateMachine.setState(State.CONFIGURED);
                    }
                } else {
                    Log.e(TAG, "Unable to save configuration after it was received.");
                }

            } catch (JSONException e) {
                Log.d(TAG,
                        String.format(
                                "Error constructing configuration message JSON object. Error: %s", e.getMessage()));
            }
        } else {
            Log.e(TAG, "Configuration message should only be sent alongside the startService intent.");
        }
    }
}
