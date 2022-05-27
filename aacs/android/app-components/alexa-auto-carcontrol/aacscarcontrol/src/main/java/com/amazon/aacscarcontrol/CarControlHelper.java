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

import android.car.VehiclePropertyIds;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

public class CarControlHelper {
    public static final String TAG = CarControlHelper.class.getCanonicalName();
    private static final String AACS_CC_CAPABILITY_MAPPING_URI =
            "com.amazon.aacscarcontrolmapping.CarControlHelper.capability";
    private static final String AACS_CC_AREA_MAPPING_URI = "com.amazon.aacscarcontrolmapping.CarControlHelper.area";
    private static final String AACS_CC_MODE_MAPPING_URI = "com.amazon.aacscarcontrolmapping.CarControlHelper.mode";
    private static final String AACS_CC_MAPPING = "aacs-cc-mapping";
    private Context mContext;
    private SharedPreferences mCapabilityPref;
    private SharedPreferences mAreaPref;
    private SharedPreferences mModePref;

    public CarControlHelper(Context context) {
        mContext = context;
        mCapabilityPref = mContext.getSharedPreferences(AACS_CC_CAPABILITY_MAPPING_URI, mContext.MODE_PRIVATE);
        mAreaPref = mContext.getSharedPreferences(AACS_CC_AREA_MAPPING_URI, mContext.MODE_PRIVATE);
        mModePref = mContext.getSharedPreferences(AACS_CC_MODE_MAPPING_URI, mContext.MODE_PRIVATE);
        File customConfigFile = context.getFileStreamPath(CarControlConstants.AACS_CC_ConfigMapping);

        if (mCapabilityPref.getBoolean(AACS_CC_MAPPING, false) && mAreaPref.getBoolean(AACS_CC_MAPPING, false)) {
            Log.i(TAG, "Car Control Mapping Previously Exists in SharedPreference");
        } else {
            try {
                JSONObject carControlJSON;
                if (customConfigFile.exists()) {
                    carControlJSON = CarControlUtil.readConfig(new FileInputStream(customConfigFile));
                    Log.i(TAG, "Custom endpoint mapping created");
                } else {
                    carControlJSON = CarControlUtil.readConfig(
                            mContext.getAssets().open(CarControlConstants.AACS_CC_ConfigMapping));
                    Log.i(TAG, "Default endpoint mapping created");
                }

                SharedPreferences.Editor capabilityMapping =
                        mContext.getSharedPreferences(AACS_CC_CAPABILITY_MAPPING_URI, mContext.MODE_PRIVATE).edit();
                SharedPreferences.Editor areaMapping =
                        mContext.getSharedPreferences(AACS_CC_AREA_MAPPING_URI, mContext.MODE_PRIVATE).edit();
                if (carControlJSON.has(CarControlConstants.AACS_CARCONTROL)) {
                    JSONArray ccMap = carControlJSON.getJSONArray(CarControlConstants.AACS_CARCONTROL);
                    for (int i = 0; i < ccMap.length(); i++) {
                        capabilityMapping
                                .putString(ccMap.getJSONObject(i).getString(CarControlConstants.ENDPOINT_ID),
                                        ccMap.getJSONObject(i)
                                                .getJSONArray(CarControlConstants.CAPABILITIES)
                                                .toString())
                                .apply();
                        areaMapping
                                .putString(ccMap.getJSONObject(i).getString(CarControlConstants.ENDPOINT_ID),
                                        ccMap.getJSONObject(i).getString(CarControlConstants.AREA_ID))
                                .apply();
                    }
                    // Flag storing mapping to SharedPreference is finished
                    capabilityMapping.putBoolean(AACS_CC_MAPPING, true).apply();
                    areaMapping.putBoolean(AACS_CC_MAPPING, true).apply();
                }
            } catch (Exception e) {
                Log.e(TAG, "Error open Car Control mapping asset file " + e);
            }
        }
    }

    /**
     * This method retrieves Android PropertyID, AreaID and Property DataType information from SharedPreference
     * and assemble into PropertySetting object then return to {@link CarControlHandler} to be used by Set/Adjust
     * Controller methods for POWER, TOGGLE and RANGE controller.
     * Note that there is no controllerId present in the payload if the controllerType is "POWER".
     *
     * @param endpointId
     * @param capability
     * @param instance
     *
     * Capability - ControllerType
     * Instance - ControllerId
     */
    protected PropertySetting getPropertySetting(String endpointId, String capability, String instance) {
        PropertySetting propertySetting = null;
        if (mCapabilityPref.getBoolean(AACS_CC_MAPPING, false)) {
            try {
                JSONArray capabilitiesArray = new JSONArray(mCapabilityPref.getString(endpointId, ""));
                for (int i = 0; i < capabilitiesArray.length(); i++) {
                    if (capability.equals(capabilitiesArray.getJSONObject(i).getString(CarControlConstants.INTERFACE))
                            && (!capabilitiesArray.getJSONObject(i).has(CarControlConstants.INSTANCE)
                                    || instance.equals(capabilitiesArray.getJSONObject(i).getString(
                                            CarControlConstants.INSTANCE)))) {
                        int areaId = getAreaId(endpointId);
                        if (capabilitiesArray.getJSONObject(i)
                                        .getJSONObject(CarControlConstants.CONFIGURATION)
                                        .has(CarControlConstants.AREA_ID)) {
                            areaId = Integer.decode(capabilitiesArray.getJSONObject(i)
                                                            .getJSONObject(CarControlConstants.CONFIGURATION)
                                                            .getString(CarControlConstants.AREA_ID));
                        }
                        int propertyId = (int) VehiclePropertyIds.class
                                                 .getField(capabilitiesArray.getJSONObject(i)
                                                                   .getJSONObject(CarControlConstants.CONFIGURATION)
                                                                   .getString(CarControlConstants.PROPERTY_ID))
                                                 .get(null);
                        CarControlConstants.DataType dataType =
                                CarControlUtil.getDataType(capabilitiesArray.getJSONObject(i)
                                                                   .getJSONObject(CarControlConstants.CONFIGURATION)
                                                                   .getString(CarControlConstants.DATA_TYPE));
                        propertySetting = new PropertySetting(propertyId, areaId, dataType, "");
                    }
                }
            } catch (JSONException | NoSuchFieldException | IllegalAccessException e) {
                Log.e(TAG, "Could not find Property Setting from AACS CarControl Config " + e);
            }
        }
        return propertySetting;
    }

    /**
     * This method retrieves Android PropertyID, AreaID, Property DataType and Setting Value information from
     * SharedPreference and assemble into PropertySetting list then return to {@link CarControlHandler} to be
     * used by Set/Adjust Controller methods for MODE controller.
     *
     * @param endpointId
     * @param capability
     * @param instance
     * @param value
     *
     * Capability - ControllerType
     * Instance - ControllerId
     */
    protected List<PropertySetting> getPropertySettings(
            String endpointId, String capability, String instance, String value) {
        List<PropertySetting> propertySettings = new ArrayList<>();
        if (mCapabilityPref.getBoolean(AACS_CC_MAPPING, false)) {
            try {
                JSONArray supportedModes = getSupportedMode(endpointId, capability, instance);
                for (int i = 0; i < supportedModes.length(); i++) {
                    if (value.equals(supportedModes.getJSONObject(i).getString(CarControlConstants.VALUE))) {
                        JSONArray propertySettingsJson =
                                supportedModes.getJSONObject(i).getJSONArray(CarControlConstants.MODE_SETTINGS);
                        for (int j = 0; j < propertySettingsJson.length(); j++) {
                            int propertyId = (int) VehiclePropertyIds.class
                                                     .getField(propertySettingsJson.getJSONObject(j).getString(
                                                             CarControlConstants.PROPERTY_ID))
                                                     .get(null);
                            int areaId = getAreaId(endpointId);
                            if (propertySettingsJson.getJSONObject(j).has(CarControlConstants.AREA_ID)) {
                                areaId = Integer.decode(
                                        propertySettingsJson.getJSONObject(j).getString(CarControlConstants.AREA_ID));
                            }
                            CarControlConstants.DataType dataType = CarControlUtil.getDataType(
                                    propertySettingsJson.getJSONObject(j).getString(CarControlConstants.DATA_TYPE));
                            String setValue =
                                    propertySettingsJson.getJSONObject(j).getString(CarControlConstants.VALUE);
                            propertySettings.add(new PropertySetting(propertyId, areaId, dataType, setValue));
                        }
                    }
                }
            } catch (JSONException | NoSuchFieldException | IllegalAccessException e) {
                Log.e(TAG, "Could not find Property Settings for Mode Controller from AACS CarControl Config " + e);
            }
        }
        return propertySettings;
    }

    protected JSONArray getSupportedMode(String endpointId, String capability, String instance) {
        if (mCapabilityPref.getBoolean(AACS_CC_MAPPING, false)) {
            try {
                JSONArray capabilitiesArray = new JSONArray(mCapabilityPref.getString(endpointId, ""));
                for (int i = 0; i < capabilitiesArray.length(); i++) {
                    if (capability.equals(capabilitiesArray.getJSONObject(i).getString(CarControlConstants.INTERFACE))
                            && instance.equals(
                                    capabilitiesArray.getJSONObject(i).getString(CarControlConstants.INSTANCE))) {
                        return capabilitiesArray.getJSONObject(i)
                                .getJSONObject(CarControlConstants.CONFIGURATION)
                                .getJSONArray(CarControlConstants.SUPPORTED_MODE);
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "Could not find PropertyID from AACS CarControl Config " + e);
            }
        }
        return new JSONArray();
    }

    protected void saveModeSettings(String endpointId, String instance, String value) {
        String key = endpointId + "|" + instance;
        SharedPreferences.Editor modeSettingMapping =
                mContext.getSharedPreferences(AACS_CC_MODE_MAPPING_URI, mContext.MODE_PRIVATE).edit();
        if (mModePref.contains(key)) {
            modeSettingMapping.remove(key);
        }
        modeSettingMapping.putString(key, value).apply();
    }

    protected String getModeSettings(String endpointId, String instance) {
        String key = endpointId + "|" + instance;
        if (mModePref.contains(key)) {
            return mModePref.getString(key, "");
        }
        return "";
    }

    protected int getAreaId(String endpointId) {
        if (mAreaPref.getBoolean(AACS_CC_MAPPING, false) && mAreaPref.contains(endpointId)) {
            int areaId = Integer.decode(mAreaPref.getString(endpointId, ""));
            return areaId;
        }
        return -1;
    }

    protected static class PropertySetting {
        public int propertyId;
        public int areaId;
        public CarControlConstants.DataType dataType;
        public String value;

        public PropertySetting(int propertyId, int areaId, CarControlConstants.DataType dataType, String value) {
            this.propertyId = propertyId;
            this.areaId = areaId;
            this.dataType = dataType;
            this.value = value;
        }
    }
}