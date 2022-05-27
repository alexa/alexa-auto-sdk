/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aacscarcontrol;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.Optional;

public class AACSCarControlReceiver extends BroadcastReceiver {
    private static final String TAG = AACSCarControlReceiver.class.getCanonicalName();
    private static AACSMessageSender mAACSMessageSender;
    private static Context mContext;
    private static CarControlHandler mCarControlHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.hasFileDescriptors()) {
            Uri fileUri = intent.getData();
            CarControlUtil.copyExternalFileToCarControl(context, fileUri);
            return;
        }

        Optional<AACSMessage> messageOptional = AACSMessageBuilder.parseEmbeddedIntent(intent);
        if (messageOptional.isPresent()) {
            mContext = context;
            mAACSMessageSender = new AACSMessageSender(new WeakReference<>(context), new AACSSender());
            final PendingResult result = goAsync();
            Task asyncTask = new Task(result, intent);
            asyncTask.execute();
        } else {
            Log.e(TAG, "AACS Car Control Intent Message cannot be parsed");
        }
    }

    /**
     * This method parses the CarControl.SetControllerValue message payload, extracts the endpointId, the
     * controllerType (as known as capability), the controllerId (as known as instance), the new value and notifies
     * the {@link CarControlHandler} to set the setting of the controller accordingly. Note that there is no
     * controllerId present in the payload if the controllerType is "POWER"
     *
     * @param context
     * @param payload
     * @param messageId
     *
     * Capability - ControllerType
     * Instance - ControllerId
     */
    private static void setControllerValue(Context context, String payload, String messageId) {
        if (mCarControlHandler == null) {
            mCarControlHandler = new CarControlHandler(context);
        }
        JSONObject reply = new JSONObject();
        boolean isSuccessful = false;
        String endpointId = "";
        String capability = "";
        String instance = "";
        String value = "";
        boolean turnOn = false;
        try {
            JSONObject payloadJSON = new JSONObject(payload);
            endpointId = payloadJSON.getString(CarControlConstants.ENDPOINT_ID);
            capability = payloadJSON.getString(CarControlConstants.CONTROLLER_TYPE);

            if (payloadJSON.has(CarControlConstants.CONTROLLER_ID)) {
                instance = payloadJSON.getString(CarControlConstants.CONTROLLER_ID);
            }
            if (payloadJSON.has(CarControlConstants.VALUE)) {
                value = payloadJSON.getString(CarControlConstants.VALUE);
            }
            if (payloadJSON.has(CarControlConstants.TURNON)) {
                turnOn = payloadJSON.getBoolean(CarControlConstants.TURNON);
            }

            switch (capability) {
                case CarControlConstants.POWER:
                    isSuccessful = mCarControlHandler.changePowerController(endpointId, turnOn);
                    break;
                case CarControlConstants.MODE:
                    isSuccessful = mCarControlHandler.setModeControllerValue(endpointId, instance, value);
                    break;
                case CarControlConstants.RANGE:
                    isSuccessful =
                            mCarControlHandler.setRangeControllerValue(endpointId, instance, Double.parseDouble(value));
                    break;
                case CarControlConstants.TOGGLE:
                    isSuccessful = mCarControlHandler.changeToggleController(endpointId, instance, turnOn);
                    break;
                default:
                    Log.e(TAG, "Unsupported ControllerType/Capability caught in setControllerValue: " + capability);
                    break;
            }
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error occurred when parsing Car Control message payload JSON: %s", payload));
        } catch (RuntimeException e) {
            Log.e(TAG, String.format("Unable to perform set value for %s, %s due to %s", endpointId, instance, e));
        }

        try {
            reply.put("success", isSuccessful);
        } catch (JSONException e) {
            Log.e(TAG, "Error creating Car Control reply message payload JSON " + e);
            return;
        }

        mAACSMessageSender.sendReplyMessage(
                messageId, Topic.CAR_CONTROL, Action.CarControl.SET_CONTROLLER_VALUE, reply.toString());
    }

    /**
     * This method parses the CarControl.AdjustControllerValue message payload, extracts the endpointId, the
     * controllerType (as known as capability), the controllerId (as known as instance), the new value and
     * notifies the {@link CarControlHandler} to adjust the setting of the controller accordingly. Note that
     * there is no controllerId present in the payload if the controllerType is "POWER".
     *
     * @param context
     * @param payload
     * @param messageId
     *
     * Capability - ControllerType
     * Instance - ControllerId
     */
    private static void adjustControllerValue(Context context, String payload, String messageId) {
        // Initialize Car Control Handler for making call to AAOS Car API
        if (mCarControlHandler == null) {
            mCarControlHandler = new CarControlHandler(context);
        }
        boolean isSuccessful = false;
        String endpointId = "";
        String capability = "";
        String instance = "";
        String delta = "";
        try {
            JSONObject payloadJSON = new JSONObject(payload);
            endpointId = payloadJSON.getString(CarControlConstants.ENDPOINT_ID);
            capability = payloadJSON.getString(CarControlConstants.CONTROLLER_TYPE);
            if (payloadJSON.has(CarControlConstants.CONTROLLER_ID)) {
                instance = payloadJSON.getString(CarControlConstants.CONTROLLER_ID);
            }
            if (payloadJSON.has(CarControlConstants.DELTA)) {
                delta = payloadJSON.getString(CarControlConstants.DELTA);
            }
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error occurred when parsing Car Control message payload JSON: %s", payload));
        }

        try {
            switch (capability) {
                case CarControlConstants.MODE:
                    isSuccessful =
                            mCarControlHandler.adjustModeControllerValue(endpointId, instance, Integer.parseInt(delta));
                    break;
                case CarControlConstants.RANGE:
                    isSuccessful = mCarControlHandler.adjustRangeControllerValue(
                            endpointId, instance, Double.parseDouble(delta));
                    break;
                default:
                    Log.e(TAG, "Unsupported ControllerType/Capability caught in adjustControllerValue: " + capability);
                    break;
            }
        } catch (RuntimeException e) {
            Log.e(TAG, String.format("Unable to perform adjust value for %s, %s due to %s", endpointId, instance, e));
        }

        JSONObject reply = new JSONObject();
        try {
            reply.put("success", isSuccessful);
        } catch (JSONException e) {
            Log.e(TAG, "Error creating Car Control reply message payload JSON: " + e);
            return;
        }

        mAACSMessageSender.sendReplyMessage(
                messageId, Topic.CAR_CONTROL, Action.CarControl.ADJUST_CONTROLLER_VALUE, reply.toString());
    }

    private static class Task extends AsyncTask<String, Integer, String> {
        private final PendingResult pendingResult;
        private final Intent intent;

        private Task(PendingResult pendingResult, Intent intent) {
            this.pendingResult = pendingResult;
            this.intent = intent;
        }

        @Override
        protected String doInBackground(String... strings) {
            Optional<AACSMessage> messageOptional = AACSMessageBuilder.parseEmbeddedIntent(intent);
            AACSMessage message = messageOptional.get();
            if (Topic.CAR_CONTROL.equals(message.topic)) {
                switch (message.action) {
                    case Action.CarControl.SET_CONTROLLER_VALUE:
                        setControllerValue(mContext, message.payload, message.messageId);
                        break;
                    case Action.CarControl.ADJUST_CONTROLLER_VALUE:
                        adjustControllerValue(mContext, message.payload, message.messageId);
                        break;
                }
            }
            StringBuilder sb = new StringBuilder();
            sb.append("Action: " + intent.getAction() + "\n");
            String log = sb.toString();
            Log.d(TAG, log);
            return log;
        }

        @Override
        protected void onPostExecute(String s) {
            super.onPostExecute(s);
            pendingResult.finish();
        }
    }
}