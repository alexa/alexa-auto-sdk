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

package com.amazon.sampleapp.impl.CarControl;

import android.content.Context;

import com.amazon.aace.carControl.CarControl;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * This class demonstrates a how to handle Car Control platform interfaces.
 * It uses the CarControlDataProvider class to retrieve and store values.
 */
public class CarControlHandler extends CarControl {
    private static final String TAG = "CarControl";
    private static final String MODE_CONTROLLER = "ModeController";
    private static final String POWER_CONTROLLER = "PowerController";
    private static final String RANGE_CONTROLLER = "RangeController";
    private static final String TOGGLE_CONTROLLER = "ToggleController";

    private LoggerHandler mLogger;

    public CarControlHandler(Context cCarControl, LoggerHandler logger) {
        mLogger = logger;
    }

    @Override
    public void turnPowerControllerOn(String endpointId) throws Exception {
        CarControlDataProvider.getBoolController(endpointId).setValue(true);

        String log = POWER_CONTROLLER + ",endpoint=" + endpointId + ",name=TurnOn";
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity   : Power Controller")
                .append("\n")
                .append("Endpoint : ")
                .append(endpointId)
                .append("\n")
                .append("Action   : Turn On");
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public void turnPowerControllerOff(String endpointId) throws Exception {
        CarControlDataProvider.getBoolController(endpointId).setValue(false);

        String log = POWER_CONTROLLER + ",endpoint=" + endpointId + ",name=TurnOff";
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity   : Power Controller")
                .append("\n")
                .append("Endpoint : ")
                .append(endpointId)
                .append("\n")
                .append("Action   : Turn Off");
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public boolean isPowerControllerOn(String endpointId) throws Exception {
        boolean isOn = CarControlDataProvider.getBoolController(endpointId).getValue();

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity   : Power Controller")
                .append("\n")
                .append("Endpoint : ")
                .append(endpointId)
                .append("\n")
                .append("Action   : Query State")
                .append("\n")
                .append("State    : ")
                .append(isOn ? "On" : "Off");
        mLogger.postInfo(TAG, prettyPrint.toString());

        return isOn;
    }

    @Override
    public void turnToggleControllerOn(String endpointId, String controllerId) throws Exception {
        CarControlDataProvider.getBoolController(endpointId, controllerId).setValue(true);

        String log = TOGGLE_CONTROLLER + ",endpoint=" + endpointId + ",name=TurnOn"
                + ",instance=" + controllerId;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Toggle Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Turn On");
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public void turnToggleControllerOff(String endpointId, String controllerId) throws Exception {
        CarControlDataProvider.getBoolController(endpointId, controllerId).setValue(false);

        String log = TOGGLE_CONTROLLER + ",endpoint=" + endpointId + ",name=TurnOff"
                + ",instance=" + controllerId;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Toggle Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Turn Off");
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public boolean isToggleControllerOn(String endpointId, String controllerId) throws Exception {
        boolean isOn = CarControlDataProvider.getBoolController(endpointId, controllerId).getValue();

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Toggle Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Query State")
                .append("\n")
                .append("State      : ")
                .append(isOn ? "On" : "Off");
        mLogger.postInfo(TAG, prettyPrint.toString());

        return isOn;
    }

    @Override
    public void setRangeControllerValue(String endpointId, String controllerId, double value) throws Exception {
        CarControlDataProvider.getRangeController(endpointId, controllerId).setValue(value);

        String log = RANGE_CONTROLLER + ",endpoint=" + endpointId + ",name=SetRangeValue"
                + ",instance=" + controllerId + ",rangeValue=" + value;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Range Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Set Value")
                .append("\n")
                .append("Value      : ")
                .append(value);
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public void adjustRangeControllerValue(String endpointId, String controllerId, double delta) throws Exception {
        double value = CarControlDataProvider.getRangeController(endpointId, controllerId).getValue();
        CarControlDataProvider.getRangeController(endpointId, controllerId).adjustValue(delta);
        double valueNew = CarControlDataProvider.getRangeController(endpointId, controllerId).getValue();

        String log = RANGE_CONTROLLER + ",endpoint=" + endpointId + ",name=AdjustRangeValue"
                + ",instance=" + controllerId + ",rangeValueDelta=" + delta;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Range Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Adjust Value")
                .append("\n")
                .append("Value      : ")
                .append(value)
                .append("\n")
                .append("Delta      : ")
                .append(delta)
                .append("\n")
                .append("New Value  : ")
                .append(valueNew);
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public double getRangeControllerValue(String endpointId, String controllerId) throws Exception {
        double value = CarControlDataProvider.getRangeController(endpointId, controllerId).getValue();

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Range Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Get Value")
                .append("\n")
                .append("Value      : ")
                .append(value);
        mLogger.postInfo(TAG, prettyPrint.toString());

        return value;
    }

    @Override
    public void setModeControllerValue(String endpointId, String controllerId, String value) throws Exception {
        CarControlDataProvider.getModeController(endpointId, controllerId).setMode(value);

        String log = MODE_CONTROLLER + ",endpoint=" + endpointId + ",name=SetMode"
                + ",instance=" + controllerId + ",mode=" + value;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Mode Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Set Mode")
                .append("\n")
                .append("Value      : ")
                .append(value);
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public void adjustModeControllerValue(String endpointId, String controllerId, int delta) throws Exception {
        String value = CarControlDataProvider.getModeController(endpointId, controllerId).getMode();
        CarControlDataProvider.getModeController(endpointId, controllerId).adjustMode(delta);
        String valueNew = CarControlDataProvider.getModeController(endpointId, controllerId).getMode();

        String log = MODE_CONTROLLER + ",endpoint=" + endpointId + ",name=AdjustMode"
                + ",instance=" + controllerId + ",modeDelta=" + delta;
        mLogger.postInfo(TAG, log);

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Mode Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Adjust Value")
                .append("\n")
                .append("Value      : ")
                .append(value)
                .append("\n")
                .append("Delta      : ")
                .append(delta)
                .append("\n")
                .append("New Value  : ")
                .append(valueNew);
        mLogger.postInfo(TAG, prettyPrint.toString());
    }

    @Override
    public String getModeControllerValue(String endpointId, String controllerId) throws Exception {
        String value = CarControlDataProvider.getModeController(endpointId, controllerId).getMode();

        StringBuilder prettyPrint = new StringBuilder("\n");
        prettyPrint.append("Entity     : Mode Controller")
                .append("\n")
                .append("Endpoint   : ")
                .append(endpointId)
                .append("\n")
                .append("Controller : ")
                .append(controllerId)
                .append("\n")
                .append("Action     : Get Value")
                .append("\n")
                .append("Value      : ")
                .append(value);
        mLogger.postInfo(TAG, prettyPrint.toString());

        return value;
    }
}
