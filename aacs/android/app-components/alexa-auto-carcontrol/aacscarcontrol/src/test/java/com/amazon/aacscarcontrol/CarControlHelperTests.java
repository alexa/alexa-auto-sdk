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

import android.content.Context;
import android.os.Build;

import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.runner.AndroidJUnit4;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import java.util.List;

@RunWith(AndroidJUnit4.class)
@Config(sdk = {Build.VERSION_CODES.O_MR1})
public class CarControlHelperTests {
    private final String DEFAULT_AC_ENDPOINT = "default.ac";
    private final String DEFAULT_FAN_ENDPOINT = "default.fan";
    private final String INVALID_DEFAULT_FAN_ENDPOINT = "default.fan1";
    private final String CAR_ENDPOINT = "car";
    private final String INVALID_CAR_ENDPOINT = "vehicle";
    private final String RECIR_INSTANCE = "recirculate";
    private final String INTENSITY_INSTANCE = "intensity";
    private final String INVALID_INTENSITY_INSTANCE = "intense";
    private final String SPEED_INSTANCE = "speed";
    private final String INVALID_SPEED_INSTANCE = "speedo";
    private final String HIGH_VALUE = "HIGH";
    private final String SUPERHIGH_VALUE = "SUPERHIGH";
    private final int PROPERTY_ID_HVAC_MAX_AC_ON = 354419974;
    private final int PROPERTY_ID_HVAC_FAN_SPEED = 356517120;
    private final int PROPERTY_ID_HVAC_TEMPERATURE_SET = 358614275;
    private final int PROPERTY_ID_HVAC_POWER_ON = 354419984;
    private final int PROPERTY_ID_HVAC_RECIRC_ON = 354419976;
    private final int AREA_ID_FULL = 117;
    private final int AREA_ID_LEFT = 49;
    private final int AREA_ID_RIGHT = 68;

    Context mContext;
    CarControlHelper mHelper;

    @Before
    public void setup() {
        mContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        mHelper = new CarControlHelper(mContext);
    }

    @Test
    public void testGetPropertySettingWithValidParam() {
        // Power Controller Type
        CarControlHelper.PropertySetting setting1 =
                mHelper.getPropertySetting(DEFAULT_FAN_ENDPOINT, CarControlConstants.POWER_CONTROLLER, "");
        Assert.assertEquals(setting1.propertyId, PROPERTY_ID_HVAC_POWER_ON);
        Assert.assertEquals(setting1.areaId, AREA_ID_FULL);
        Assert.assertEquals(setting1.dataType, CarControlConstants.DataType.BOOLEAN);

        // Range Controller Type
        CarControlHelper.PropertySetting setting2 =
                mHelper.getPropertySetting(DEFAULT_FAN_ENDPOINT, CarControlConstants.RANGE_CONTROLLER, SPEED_INSTANCE);
        Assert.assertEquals(setting2.propertyId, PROPERTY_ID_HVAC_FAN_SPEED);
        Assert.assertEquals(setting2.areaId, AREA_ID_FULL);
        Assert.assertEquals(setting2.dataType, CarControlConstants.DataType.INT);

        // Toggle Controller Type
        CarControlHelper.PropertySetting setting3 =
                mHelper.getPropertySetting(CAR_ENDPOINT, CarControlConstants.TOGGLE_CONTROLLER, RECIR_INSTANCE);
        Assert.assertEquals(setting3.propertyId, PROPERTY_ID_HVAC_RECIRC_ON);
        Assert.assertEquals(setting3.areaId, AREA_ID_FULL);
        Assert.assertEquals(setting3.dataType, CarControlConstants.DataType.BOOLEAN);
    }

    @Test
    public void testGetPropertySettingsWithValidParam() {
        // Mode Controller Type
        List<CarControlHelper.PropertySetting> settingList = mHelper.getPropertySettings(
                DEFAULT_AC_ENDPOINT, CarControlConstants.MODE_CONTROLLER, INTENSITY_INSTANCE, HIGH_VALUE);
        Assert.assertEquals(settingList.get(0).propertyId, PROPERTY_ID_HVAC_MAX_AC_ON);
        Assert.assertEquals(settingList.get(0).areaId, AREA_ID_FULL);
        Assert.assertEquals(settingList.get(0).dataType, CarControlConstants.DataType.BOOLEAN);
        Assert.assertEquals(settingList.get(0).value, "true");

        Assert.assertEquals(settingList.get(1).propertyId, PROPERTY_ID_HVAC_FAN_SPEED);
        Assert.assertEquals(settingList.get(1).areaId, AREA_ID_FULL);
        Assert.assertEquals(settingList.get(1).dataType, CarControlConstants.DataType.INT);
        Assert.assertEquals(settingList.get(1).value, "6");

        Assert.assertEquals(settingList.get(2).propertyId, PROPERTY_ID_HVAC_TEMPERATURE_SET);
        Assert.assertEquals(settingList.get(2).areaId, AREA_ID_LEFT);
        Assert.assertEquals(settingList.get(2).dataType, CarControlConstants.DataType.FLOAT);
        Assert.assertEquals(settingList.get(2).value, "61.0");

        Assert.assertEquals(settingList.get(3).propertyId, PROPERTY_ID_HVAC_TEMPERATURE_SET);
        Assert.assertEquals(settingList.get(3).areaId, AREA_ID_RIGHT);
        Assert.assertEquals(settingList.get(2).dataType, CarControlConstants.DataType.FLOAT);
        Assert.assertEquals(settingList.get(2).value, "61.0");
    }

    @Test
    public void testGetPropertySettingWithInvalidParam() {
        // Power Controller Type
        Assert.assertNull(
                mHelper.getPropertySetting(INVALID_DEFAULT_FAN_ENDPOINT, CarControlConstants.POWER_CONTROLLER, ""));

        // Toggle Controller Type
        Assert.assertNull(mHelper.getPropertySetting(
                DEFAULT_FAN_ENDPOINT, CarControlConstants.TOGGLE_CONTROLLER, INVALID_SPEED_INSTANCE));

        // Range Controller Type
        Assert.assertNull(mHelper.getPropertySetting(
                INVALID_CAR_ENDPOINT, CarControlConstants.RANGE_CONTROLLER, INVALID_INTENSITY_INSTANCE));
    }

    @Test
    public void testGetPropertySettingsWithInvalidParam() {
        // Mode Controller Type
        List<CarControlHelper.PropertySetting> settingList = mHelper.getPropertySettings(
                DEFAULT_AC_ENDPOINT, CarControlConstants.MODE_CONTROLLER, INTENSITY_INSTANCE, SUPERHIGH_VALUE);
        Assert.assertEquals(settingList.size(), 0);
    }
}
