/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.PhoneCallController;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.aace.phonecontrol.PhoneCallController;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class PhoneCallControllerHandler extends PhoneCallController {
    private static final String sTag = "PhoneCallController";

    private static final long sDialingToRingingDelay = 2;

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private TextView mDeviceConfigurationButton;
    private TextView mCurrentCallNumberView, mCallStateView, mLastCalledNumberView;
    private EditText mCallingNumberText;
    private LinearLayout mLocalAnswerDecline, mRemoteAnswerDecline, mControlsLayout;
    private TextView mLocalInitiateButton, mLocalEndButton, mLocalAnswerButton, mLocalDeclineButton;
    private TextView mRemoteInitiateButton, mRemoteEndButton, mRemoteAnswerButton, mRemoteDeclineButton;
    private String mCallId, mCurrentCallNumber, mLastCalledNumber;
    private boolean mCallActivated = false;
    private boolean mLocalCallStarted = false;
    private boolean mRemoteCallStarted = false;
    private CallState mCallState;
    private ConnectionState mConnectionState;
    private HashMap<CallingDeviceConfigurationProperty, Boolean> mDeviceConfiguration;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private Timer mTimer = new Timer();

    public PhoneCallControllerHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mCallId = "";
        mCallState = CallState.IDLE;
        mConnectionState = ConnectionState.DISCONNECTED;
        setupGUI();
        updateGUI();

        mDeviceConfiguration = new HashMap<>();
        mDeviceConfiguration.put(CallingDeviceConfigurationProperty.DTMF_SUPPORTED, false);
    }

    @Override
    public boolean dial(String payload) {
        // Handling should not block the caller
        String callId = "";
        String calleeNumber = "";

        try {
            JSONObject obj = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, obj.toString(4));
            callId = obj.getString("callId");
            calleeNumber = getCalleeDefaultAddressValue(obj);
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing dial directive payload: " + e.getMessage());
            return false;
        }

        mCallId = callId;
        mCurrentCallNumber = calleeNumber;
        mCallState = CallState.DIALING;
        logCallInfo("dial()");
        callStateChanged(CallState.DIALING, callId);

        startDialingCallTimer(sDialingToRingingDelay);
        mLocalCallStarted = true;
        updateGUI();
        return true;
    }

    @Override
    public boolean redial(String payload) {
        // Handling should not block the caller
        String callId = "";
        try {
            JSONObject obj = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, obj.toString(4));
            callId = obj.getString("callId");
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing redial directive payload: " + e.getMessage());
            return false;
        }

        if (mLastCalledNumber == null || mLastCalledNumber.equals("")) {
            callFailed(callId, CallError.NO_NUMBER_FOR_REDIAL);
            return true;
        }

        mCallId = callId;
        mCurrentCallNumber = mLastCalledNumber;
        mCallState = CallState.DIALING;
        logCallInfo("redial()");
        callStateChanged(CallState.DIALING, callId);

        startDialingCallTimer(sDialingToRingingDelay);
        mLocalCallStarted = true;
        updateGUI();
        return true;
    }

    @Override
    public void answer(String payload) {
        // Handling should not block the caller
        String callId = "";
        try {
            JSONObject obj = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, obj.toString(4));
            callId = obj.getString("callId");
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing answer directive payload: " + e.getMessage());
            return;
        }

        logCallInfo("answer()");

        if (!mCallId.equals(callId)) {
            callFailed(callId, CallError.OTHER, "Call ID does not match");
        }
        handleAnswerCall();
    }

    @Override
    public void stop(String payload) {
        // Handling should not block the caller
        String callId = "";
        try {
            JSONObject obj = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, obj.toString(4));
            callId = obj.getString("callId");
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing stop directive payload: " + e.getMessage());
            return;
        }

        logCallInfo("stop()");

        if (!mCallId.equals(callId)) {
            callFailed(callId, CallError.OTHER, "Call ID does not match");
        }
        if (mCallState == CallState.INBOUND_RINGING || mCallState == CallState.CALL_RECEIVED) {
            handleDeclineCall(false);
        } else {
            handleEndCall();
        }
    }

    @Override
    public void sendDTMF(String payload) {
        // Handling should not block the caller
        String callId = "";
        try {
            JSONObject obj = new JSONObject(payload);
            mLogger.postJSONTemplate(sTag, obj.toString(4));
            callId = obj.getString("callId");
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing stop directive payload: " + e.getMessage());
            return;
        }

        logCallInfo("sendDTMF()");

        if (!mCallId.equals(callId)) {
            callFailed(callId, CallError.OTHER, "Call ID does not match");
        }
        sendDTMFSucceeded(callId);
    }

    private void startDialingCallTimer(Long delaySeconds) {
        mTimer.schedule(new TimerTask() {
            public void run() {
                handleInitiateCall();
            }
        }, delaySeconds * 1000);
    }

    private class InitiateCallTask implements Runnable {
        @Override
        public void run() {
            if (mCallId.equals("")) {
                mCallId = createCallId();
            }

            mCallState = CallState.OUTBOUND_RINGING;
            callStateChanged(CallState.OUTBOUND_RINGING, mCallId);
            logCallInfo("handleInitiateCall()");
            updateGUI();
        }
    };

    private void handleInitiateCall() {
        mLocalCallStarted = true;
        mExecutor.submit(new InitiateCallTask());
    }

    private void startCallReceivedTimer(Long delaySeconds) {
        mTimer.schedule(new TimerTask() {
            public void run() {
                handleCallReceived();
            }
        }, delaySeconds * 1000);
    }

    private void handleCallReceived() {
        mExecutor.submit(new CallReceivedTask());
    }

    private class CallReceivedTask implements Runnable {
        @Override
        public void run() {
            // Notify of call received and then set state to ringing
            callStateChanged(CallState.CALL_RECEIVED, mCallId);
            mCallState = CallState.INBOUND_RINGING;
            callStateChanged(mCallState, mCallId);
            logCallInfo("handleCallReceived()");
            updateGUI();
        }
    };

    private void handleRemoteInitiateCall() {
        mRemoteCallStarted = true;
        String callId = createCallId();
        mCallId = callId;
        mCallState = CallState.INBOUND_RINGING;
        callStateChanged(mCallState, callId);
        logCallInfo("handleRemoteInitiateCall()");
        updateGUI();
    }

    private void handleAnswerCall() {
        mCallActivated = true;
        mCallState = CallState.ACTIVE;
        callStateChanged(CallState.ACTIVE, mCallId);
        logCallInfo("handleAnswerCall()");
        updateGUI();
    }

    private void handleDeclineCall(boolean remoteDeclined) {
        if (remoteDeclined) {
            mLastCalledNumber = mCurrentCallNumber;
        }
        mRemoteCallStarted = false;
        mLocalCallStarted = false;
        mCallState = CallState.IDLE;
        logCallInfo("handleDeclineCall()");
        callStateChanged(CallState.IDLE, mCallId);
        mCurrentCallNumber = "";
        mCallId = "";
        updateGUI();
    }

    private void handleEndCall() {
        if (mLocalCallStarted) {
            mLastCalledNumber = mCurrentCallNumber;
        }
        mRemoteCallStarted = false;
        mLocalCallStarted = false;
        mCallActivated = false;
        mCallState = CallState.IDLE;
        logCallInfo("handleEndCall()");
        callStateChanged(CallState.IDLE, mCallId);
        mCurrentCallNumber = "";
        mCallId = "";
        updateGUI();
    }

    private void togglePhoneConnectionState(boolean enable) {
        if (enable) {
            mControlsLayout.setVisibility(View.VISIBLE);
            mConnectionState = ConnectionState.CONNECTED;
            mLogger.postInfo(sTag, "ConnectionState: CONNECTED");
            connectionStateChanged(mConnectionState);
        } else {
            mControlsLayout.setVisibility(View.GONE);
            mConnectionState = ConnectionState.DISCONNECTED;
            mLogger.postInfo(sTag, "ConnectionState: DISCONNECTED");
            connectionStateChanged(mConnectionState);

            // Notify the Engine if phone disconnects during call
            if (mLocalCallStarted || mRemoteCallStarted) {
                mLogger.postInfo(sTag, "Call Failed, ConnectionState: DISCONNECTED");
                callFailed(mCallId, CallError.OTHER, "Connection lost");
                handleEndCall();
            }
        }
    }

    private String getCalleeDefaultAddressValue(JSONObject payload) {
        String address = "";
        try {
            JSONObject callee = payload.getJSONObject("callee");
            if (callee != null) {
                JSONObject defaultAddress = callee.getJSONObject("defaultContactAddress");
                if (defaultAddress != null) {
                    return defaultAddress.getString("value");
                }
            }
        } catch (JSONException e) {
            mLogger.postError(sTag, "Error parsing callee default address value: " + e.getMessage());
        }
        return address;
    }

    private void onDeviceConfigurationUpdated() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
        builder.setTitle("Device Configurations");

        CallingDeviceConfigurationProperty[] configurations = CallingDeviceConfigurationProperty.values();
        final String[] configurationNames = new String[configurations.length];
        boolean[] configurationValues = new boolean[configurations.length];

        for (int i = 0; i < configurations.length; i++) {
            configurationNames[i] = configurations[i].name();
        }
        Arrays.sort(configurationNames);

        for (int i = 0; i < configurations.length; i++) {
            configurationValues[i] =
                    mDeviceConfiguration.get(CallingDeviceConfigurationProperty.valueOf(configurationNames[i]));
        }

        builder.setMultiChoiceItems(
                configurationNames, configurationValues, new DialogInterface.OnMultiChoiceClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i, boolean b) {
                        mDeviceConfiguration.put(CallingDeviceConfigurationProperty.valueOf(configurationNames[i]), b);
                    }
                });

        builder.setPositiveButton("Update", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                deviceConfigurationUpdated(mDeviceConfiguration);
            }
        });
        builder.setNegativeButton("Cancel", null);
        AlertDialog dialog = builder.create();
        dialog.show();
        mLogger.postInfo(sTag, "onDeviceConfigurationUpdated() finished");
    }

    private void onLocalInitiate() {
        mCurrentCallNumber = mCallingNumberText.getText().toString();
        handleInitiateCall();
    }

    private void onLocalEnd() {
        handleEndCall();
    }

    private void onLocalAnswer() {
        handleAnswerCall();
    }
    private void onLocalDecline() {
        handleDeclineCall(false);
    }

    private void onRemoteInitiate() {
        handleRemoteInitiateCall();
    }

    private void onRemoteEnd() {
        handleEndCall();
    }

    private void onRemoteAnswer() {
        handleAnswerCall();
    }

    private void onRemoteDecline() {
        handleDeclineCall(true);
    }

    /* For Updating GUI */

    private void setupGUI() {
        // Switch to toggle phone call connection state
        View switchItem = mActivity.findViewById(R.id.togglePhoneCallConnection);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.phone_call_connection_switch);
        SwitchCompat phoneCallConnectionSwitch = switchItem.findViewById(R.id.drawerSwitch);
        phoneCallConnectionSwitch.setChecked(false);
        phoneCallConnectionSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                togglePhoneConnectionState(isChecked);
            }
        });

        mControlsLayout = mActivity.findViewById(R.id.phone_call_controller_config);
        mControlsLayout.setVisibility(View.GONE);
        mDeviceConfigurationButton = mActivity.findViewById(R.id.deviceConfiguration);
        mCurrentCallNumberView = mActivity.findViewById(R.id.currentCallNumber);
        mCallStateView = mActivity.findViewById(R.id.currentCallState);
        mLastCalledNumberView = mActivity.findViewById(R.id.lastCalledNumber);
        mCallingNumberText = mActivity.findViewById(R.id.localCallingNumber);
        mLocalInitiateButton = mActivity.findViewById(R.id.localInitiate);
        mLocalEndButton = mActivity.findViewById(R.id.localEnd);
        mLocalAnswerDecline = mActivity.findViewById(R.id.localAnswerDecline);
        mLocalAnswerButton = mActivity.findViewById(R.id.localAnswer);
        mLocalDeclineButton = mActivity.findViewById(R.id.localDecline);
        mRemoteInitiateButton = mActivity.findViewById(R.id.remoteInitiate);
        mRemoteEndButton = mActivity.findViewById(R.id.remoteEnd);
        mRemoteAnswerDecline = mActivity.findViewById(R.id.remoteAnswerDecline);
        mRemoteAnswerButton = mActivity.findViewById(R.id.remoteAnswer);
        mRemoteDeclineButton = mActivity.findViewById(R.id.remoteDecline);

        mDeviceConfigurationButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onDeviceConfigurationUpdated();
            }
        });
        mLocalInitiateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLocalInitiate();
            }
        });
        mLocalEndButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLocalEnd();
            }
        });
        mLocalAnswerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLocalAnswer();
            }
        });
        mLocalDeclineButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLocalDecline();
            }
        });
        mRemoteInitiateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onRemoteInitiate();
            }
        });
        mRemoteEndButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onRemoteEnd();
            }
        });
        mRemoteAnswerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onRemoteAnswer();
            }
        });
        mRemoteDeclineButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onRemoteDecline();
            }
        });
    }

    private void updateGUI() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mCallState == CallState.IDLE) {
                    mCallStateView.setText("");
                } else {
                    mCallStateView.setText(mCallState.toString());
                }
                mCurrentCallNumberView.setText(mCurrentCallNumber);
                mLastCalledNumberView.setText(mLastCalledNumber);

                if (mCallActivated) {
                    mLocalInitiateButton.setVisibility(View.GONE);
                    mLocalEndButton.setVisibility(View.VISIBLE);
                    mRemoteInitiateButton.setVisibility(View.GONE);
                    mRemoteEndButton.setVisibility(View.VISIBLE);
                    mLocalAnswerDecline.setVisibility(View.GONE);
                    mRemoteAnswerDecline.setVisibility(View.GONE);
                } else if (!mLocalCallStarted && !mRemoteCallStarted) {
                    mLocalInitiateButton.setVisibility(View.VISIBLE);
                    mLocalEndButton.setVisibility(View.GONE);
                    mRemoteInitiateButton.setVisibility(View.VISIBLE);
                    mRemoteEndButton.setVisibility(View.GONE);
                    mLocalAnswerDecline.setVisibility(View.GONE);
                    mRemoteAnswerDecline.setVisibility(View.GONE);
                } else if (mLocalCallStarted && !mRemoteCallStarted) {
                    mLocalInitiateButton.setVisibility(View.GONE);
                    mLocalEndButton.setVisibility(View.VISIBLE);
                    mRemoteInitiateButton.setVisibility(View.GONE);
                    mRemoteEndButton.setVisibility(View.GONE);
                    mLocalAnswerDecline.setVisibility(View.GONE);
                    mRemoteAnswerDecline.setVisibility(View.VISIBLE);
                } else if (!mLocalCallStarted && mRemoteCallStarted) {
                    mLocalInitiateButton.setVisibility(View.GONE);
                    mLocalEndButton.setVisibility(View.GONE);
                    mRemoteInitiateButton.setVisibility(View.GONE);
                    mRemoteEndButton.setVisibility(View.VISIBLE);
                    mLocalAnswerDecline.setVisibility(View.VISIBLE);
                    mRemoteAnswerDecline.setVisibility(View.GONE);
                }
            }
        });
    }

    private void logCallInfo(String msg) {
        mLogger.postInfo(sTag,
                msg + " - Phone number: " + mCurrentCallNumber + " Call id: " + mCallId
                        + " CallState: " + mCallState.toString() + " ConnectionState: " + mConnectionState);
    }
}
