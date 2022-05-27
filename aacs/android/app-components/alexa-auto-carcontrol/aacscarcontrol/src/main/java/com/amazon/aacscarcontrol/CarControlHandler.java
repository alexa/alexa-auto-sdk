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

import android.car.Car;
import android.car.VehiclePropertyIds;
import android.car.hardware.property.CarPropertyManager;
import android.content.Context;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.List;

public class CarControlHandler {
    private static final String TAG = AACSConstants.AACS + "-" + CarControlHandler.class.getCanonicalName();
    private Context mContext;
    private CarPropertyManager mCarManager;
    private CarControlHelper mHelper;

    public CarControlHandler(Context context) {
        mContext = context;
        Car car = Car.createCar(context);
        if (car == null) {
            Log.e(TAG, "Car Service is not available, Car object is null");
            mCarManager = null;
        } else {
            mCarManager = (CarPropertyManager) car.getCarManager(Car.PROPERTY_SERVICE);
            if (mHelper == null) {
                mHelper = new CarControlHelper(context);
            }
        }
    }

    CarControlHandler(Context context, CarPropertyManager carPropertyManager, CarControlHelper helper) {
        mContext = context;
        mCarManager = carPropertyManager;
        mHelper = helper;
    }

    public boolean changePowerController(String endpointId, boolean turnOn) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.POWER_CONTROLLER, "");
        if (setting == null) {
            Log.e(TAG, String.format("Property setting %s cannot be found from Car Control configuration", endpointId));
            return false;
        }
        try {
            mCarManager.setBooleanProperty(setting.propertyId, setting.areaId, turnOn);
            Log.d(TAG,
                    String.format("Turn Power Controller for PropertyID: %s at AreaID: %s to %s", setting.propertyId,
                            setting.areaId, turnOn));
            return true;
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG, String.format("Error when change Power controller %s with exception %s", endpointId, e));
            return false;
        }
    }

    public boolean isPowerControllerOn(String endpointId) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.POWER_CONTROLLER, "");
        if (setting == null) {
            Log.e(TAG, String.format("Property setting %s cannot be found from Car Control configuration", endpointId));
            return false;
        }
        try {
            return mCarManager.getBooleanProperty(setting.propertyId, setting.areaId);
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG, String.format("Error when get Power controller value for %s with exception %s", endpointId, e));
            return false;
        }
    }

    public boolean changeToggleController(String endpointId, String instance, boolean turnOn) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.TOGGLE_CONTROLLER, instance);
        if (setting == null) {
            Log.e(TAG,
                    String.format("Property setting combination %s, %s cannot be found from Car Control configuration",
                            endpointId, instance));
            return false;
        }
        try {
            switch (setting.dataType) {
                case INT:
                    if (turnOn) {
                        mCarManager.setIntProperty(setting.propertyId, setting.areaId, 1);
                    } else {
                        mCarManager.setIntProperty(setting.propertyId, setting.areaId, 0);
                    }
                    Log.d(TAG,
                            String.format("Turn Toggle Controller for PropertyID: %s at AreaID: %s to %s",
                                    setting.propertyId, setting.areaId, turnOn));
                    return true;
                case BOOLEAN:
                    mCarManager.setBooleanProperty(setting.propertyId, setting.areaId, turnOn);
                    Log.d(TAG,
                            String.format("Turn Toggle Controller for PropertyID: %s at AreaID: %s to %s",
                                    setting.propertyId, setting.areaId, turnOn));
                    return true;
                default:
                    Log.e(TAG, "Configuration Setting Type not correct when changeToggleController");
                    return false;
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG, String.format("Error when change Toggle controller %s with exception %s", endpointId, e));
            return false;
        }
    }

    public boolean isToggleControllerOn(String endpointId, String instance) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.TOGGLE_CONTROLLER, instance);
        if (setting == null) {
            Log.e(TAG,
                    String.format("Property setting combination %s, %s cannot be found from Car Control configuration",
                            endpointId, instance));
            return false;
        }
        try {
            return mCarManager.getBooleanProperty(setting.propertyId, setting.areaId);
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG,
                    String.format("Error when change Toggle controller %s for instance %s with exception %s",
                            endpointId, instance, e));
            return false;
        }
    }

    public boolean setRangeControllerValue(String endpointId, String instance, double value) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.RANGE_CONTROLLER, instance);
        if (setting == null) {
            Log.e(TAG,
                    String.format("Property setting combination %s, %s cannot be found from Car Control configuration",
                            endpointId, instance));
            return false;
        }
        try {
            switch (setting.dataType) {
                case INT:
                    mCarManager.setIntProperty(setting.propertyId, setting.areaId, (int) value);
                    Log.d(TAG,
                            String.format("Set Range Controller for PropertyID: %s at AreaID: %s to %s",
                                    setting.propertyId, setting.areaId, value));
                    return true;
                case FLOAT:
                    if (mCarManager.getIntProperty(
                                VehiclePropertyIds.HVAC_TEMPERATURE_DISPLAY_UNITS, CarControlConstants.AREA_GLOBAL)
                            == CarControlConstants.CELSIUS_UNIT) {
                        mCarManager.setFloatProperty(setting.propertyId, setting.areaId, (float) value);
                    } else if (mCarManager.getIntProperty(VehiclePropertyIds.HVAC_TEMPERATURE_DISPLAY_UNITS,
                                       CarControlConstants.AREA_GLOBAL)
                            == CarControlConstants.FAHRENHEIT_UNIT) {
                        // Need to do conversion here since Android internally uses Celsius
                        mCarManager.setFloatProperty(
                                setting.propertyId, setting.areaId, CarControlUtil.celcius((float) value));
                    }
                    Log.d(TAG,
                            String.format("Set Range Controller for PropertyID: %s at AreaID: %s to %f",
                                    setting.propertyId, setting.areaId, value));
                    return true;
                default:
                    Log.e(TAG, "Configuration Setting Type not correct when setRangeControllerValue");
                    return false;
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG, String.format("Error set Range controller %s value %s with exception %s", endpointId, value, e));
            return false;
        }
    }

    public boolean adjustRangeControllerValue(String endpointId, String instance, double delta) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        double currentRangeControllerValue = getRangeControllerValue(endpointId, instance);
        if (currentRangeControllerValue < 0.0) {
            Log.e(TAG, "Invalid Current Range Value");
            return false;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.RANGE_CONTROLLER, instance);
        if (setting == null) {
            Log.e(TAG,
                    String.format("Property setting combination %s, %s cannot be found from Car Control configuration",
                            endpointId, instance));
            return false;
        }
        try {
            switch (setting.dataType) {
                case INT:
                    mCarManager.setIntProperty(
                            setting.propertyId, setting.areaId, (int) (currentRangeControllerValue + delta));
                    Log.d(TAG,
                            String.format("Adjust Range Controller for PropertyID: %s at AreaID: %s by %s",
                                    setting.propertyId, setting.areaId, delta));
                    return true;
                case FLOAT:
                    mCarManager.setFloatProperty(
                            setting.propertyId, setting.areaId, ((float) currentRangeControllerValue + (float) delta));
                    Log.d(TAG,
                            String.format("Adjust Range Controller for PropertyID: %s at AreaID: %s by %s",
                                    setting.propertyId, setting.areaId, delta));
                    return true;
                default:
                    Log.e(TAG, "Configuration Setting Type not correct when adjustRangeControllerValue");
                    return false;
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG,
                    String.format(
                            "Error when adjust Range controller %s by %f with exception %s", endpointId, delta, e));
            return false;
        }
    }

    public double getRangeControllerValue(String endpointId, String instance) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return -1.0;
        }
        CarControlHelper.PropertySetting setting =
                mHelper.getPropertySetting(endpointId, CarControlConstants.RANGE_CONTROLLER, instance);
        if (setting == null) {
            Log.e(TAG,
                    String.format("Property setting combination %s, %s cannot be found from Car Control configuration",
                            endpointId, instance));
            return -1.0;
        }
        try {
            switch (setting.dataType) {
                case INT:
                    return (double) mCarManager.getIntProperty(setting.propertyId, setting.areaId);
                case FLOAT:
                    return (double) mCarManager.getFloatProperty(setting.propertyId, setting.areaId);
                default:
                    Log.e(TAG, "Configuration Setting Type not correct when getRangeControllerValue");
                    return -1.0;
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG,
                    String.format("Error when getting value for Range controller %s with exception %s", endpointId, e));
            return -1.0;
        }
    }

    public boolean setModeControllerValue(String endpointId, String instance, String value) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        List<CarControlHelper.PropertySetting> settings =
                mHelper.getPropertySettings(endpointId, CarControlConstants.MODE_CONTROLLER, instance, value);
        if (settings == null || settings.size() == 0) {
            Log.e(TAG,
                    String.format(
                            "Property setting combination %s, %s, %s cannot be found from Car Control configuration",
                            endpointId, instance, value));
            return false;
        }
        try {
            for (CarControlHelper.PropertySetting setting : settings) {
                switch (setting.dataType) {
                    case INT:
                        mCarManager.setIntProperty(setting.propertyId, setting.areaId, Integer.parseInt(setting.value));
                        Log.d(TAG,
                                String.format("Set Mode Controller for PropertyID: %s at AreaID: %s to %s",
                                        setting.propertyId, setting.areaId, setting.value));
                        continue;
                    case BOOLEAN:
                        mCarManager.setBooleanProperty(
                                setting.propertyId, setting.areaId, Boolean.parseBoolean(setting.value));
                        Log.d(TAG,
                                String.format("Set Mode Controller for PropertyID: %s at AreaID: %s to %s",
                                        setting.propertyId, setting.areaId, setting.value));
                        continue;
                    case FLOAT:
                        if (mCarManager.getIntProperty(
                                    VehiclePropertyIds.HVAC_TEMPERATURE_DISPLAY_UNITS, CarControlConstants.AREA_GLOBAL)
                                == CarControlConstants.CELSIUS_UNIT) {
                            mCarManager.setFloatProperty(
                                    setting.propertyId, setting.areaId, Float.parseFloat(setting.value));
                        } else if (mCarManager.getIntProperty(VehiclePropertyIds.HVAC_TEMPERATURE_DISPLAY_UNITS,
                                           CarControlConstants.AREA_GLOBAL)
                                == CarControlConstants.FAHRENHEIT_UNIT) {
                            // Need to do conversion here since Android internally uses Celsius
                            mCarManager.setFloatProperty(setting.propertyId, setting.areaId,
                                    CarControlUtil.celcius(Float.parseFloat(setting.value)));
                        }
                        Log.d(TAG,
                                String.format("Set Mode Controller for PropertyID: %s at AreaID: %s to %s",
                                        setting.propertyId, setting.areaId, setting.value));
                        continue;
                    default:
                        Log.e(TAG, "Configuration Setting Type not correct when setModeControllerValue");
                        return false;
                }
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(TAG,
                    String.format(
                            "Error when set mode controller %s value %s with exception %s", endpointId, value, e));
            return false;
        }
        mHelper.saveModeSettings(endpointId, instance, value);
        return true;
    }

    public boolean adjustModeControllerValue(String endpointId, String instance, int delta) {
        if (mCarManager == null) {
            Log.e(TAG, "Car Property Manager is null");
            return false;
        }
        String curMode = getModeControllerValue(endpointId, instance);
        if (curMode.isEmpty()) {
            Log.e(TAG, "current ModeController mode value is empty");
            return false;
        }
        JSONArray supportedModes = mHelper.getSupportedMode(endpointId, CarControlConstants.MODE_CONTROLLER, instance);
        try {
            for (int i = 0; i < supportedModes.length(); i++) {
                if (supportedModes.getJSONObject(i).getString(CarControlConstants.VALUE).equals(curMode)) {
                    if (i + delta > supportedModes.length() - 1) {
                        setModeControllerValue(endpointId, instance,
                                supportedModes.getJSONObject(supportedModes.length() - 1)
                                        .getString(CarControlConstants.VALUE));
                        break;
                    } else if (i + delta < 0) {
                        setModeControllerValue(endpointId, instance,
                                supportedModes.getJSONObject(0).getString(CarControlConstants.VALUE));
                        break;
                    } else {
                        setModeControllerValue(endpointId, instance,
                                supportedModes.getJSONObject(i + delta).getString(CarControlConstants.VALUE));
                        break;
                    }
                }
            }
        } catch (SecurityException | JSONException e) {
            Log.e(TAG,
                    String.format(
                            "Error when adjust mode controller %s by %d with exception %s", endpointId, delta, e));
            return false;
        }
        return true;
    }

    public String getModeControllerValue(String endpointId, String instance) {
        String value = mHelper.getModeSettings(endpointId, instance);
        if (value.isEmpty()) {
            Log.e(TAG, endpointId + " has not set to any value for " + instance);
        }
        return value;
    }
}