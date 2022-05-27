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
import android.car.hardware.property.CarPropertyManager;
import android.content.Context;
import android.util.Log;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * Unit tests for {@link CarControlHandler}
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({Car.class, Log.class})
public class CarControlHandlerTests {
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

    private CarControlHandler mCarControlHandler;
    private List<CarControlHelper.PropertySetting> mModeList;

    @Mock
    private Context mMockedContext;

    @Mock
    private CarPropertyManager mMockedCarManager;

    @Mock
    private CarControlHelper mMockedHelper;

    @Before
    public void setUp() {
        PowerMockito.mockStatic(Log.class);
        mMockedCarManager = PowerMockito.mock(CarPropertyManager.class);
        mMockedHelper = PowerMockito.mock(CarControlHelper.class);
        mCarControlHandler = new CarControlHandler(mMockedContext, mMockedCarManager, mMockedHelper);

        PowerMockito.doNothing()
                .when(mMockedCarManager)
                .setBooleanProperty(Mockito.anyInt(), Mockito.anyInt(), Mockito.anyBoolean());
        PowerMockito.doNothing()
                .when(mMockedCarManager)
                .setIntProperty(Mockito.anyInt(), Mockito.anyInt(), Mockito.anyInt());
        PowerMockito.doNothing()
                .when(mMockedCarManager)
                .setFloatProperty(Mockito.anyInt(), Mockito.anyInt(), Mockito.anyFloat());

        mModeList = new ArrayList<>();
        mModeList.add(new CarControlHelper.PropertySetting(
                PROPERTY_ID_HVAC_MAX_AC_ON, AREA_ID_FULL, CarControlConstants.DataType.BOOLEAN, "true"));
        mModeList.add(new CarControlHelper.PropertySetting(
                PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, CarControlConstants.DataType.INT, "6"));
        mModeList.add(new CarControlHelper.PropertySetting(
                PROPERTY_ID_HVAC_TEMPERATURE_SET, AREA_ID_LEFT, CarControlConstants.DataType.FLOAT, "60.0"));
        mModeList.add(new CarControlHelper.PropertySetting(
                PROPERTY_ID_HVAC_TEMPERATURE_SET, AREA_ID_RIGHT, CarControlConstants.DataType.FLOAT, "60.0"));
    }

    @Test
    public void setPowerControllerWithValidParam() {
        PowerMockito
                .when(mMockedHelper.getPropertySetting(Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
                .thenReturn(new CarControlHelper.PropertySetting(
                        PROPERTY_ID_HVAC_POWER_ON, AREA_ID_FULL, CarControlConstants.DataType.BOOLEAN, ""));

        // Test Set Power Controller
        Assert.assertTrue(mCarControlHandler.changePowerController(DEFAULT_FAN_ENDPOINT, true));
        Mockito.verify(mMockedCarManager, Mockito.times(1))
                .setBooleanProperty(PROPERTY_ID_HVAC_POWER_ON, AREA_ID_FULL, true);
        Assert.assertTrue(mCarControlHandler.changePowerController(DEFAULT_FAN_ENDPOINT, false));
        Mockito.verify(mMockedCarManager, Mockito.times(1))
                .setBooleanProperty(PROPERTY_ID_HVAC_POWER_ON, AREA_ID_FULL, false);
    }
    @Test
    public void setToggleControllerWithValidParam() {
        PowerMockito
                .when(mMockedHelper.getPropertySetting(Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
                .thenReturn(new CarControlHelper.PropertySetting(
                        PROPERTY_ID_HVAC_RECIRC_ON, AREA_ID_FULL, CarControlConstants.DataType.BOOLEAN, ""));
        // Test Set Toggle Controller
        Assert.assertTrue(mCarControlHandler.changeToggleController(CAR_ENDPOINT, RECIR_INSTANCE, true));
        Mockito.verify(mMockedCarManager, Mockito.times(1))
                .setBooleanProperty(PROPERTY_ID_HVAC_RECIRC_ON, AREA_ID_FULL, true);
        Assert.assertTrue(mCarControlHandler.changeToggleController(CAR_ENDPOINT, RECIR_INSTANCE, false));
        Mockito.verify(mMockedCarManager, Mockito.times(1))
                .setBooleanProperty(PROPERTY_ID_HVAC_RECIRC_ON, AREA_ID_FULL, false);
    }

    @Test
    public void setModeControllerWithValidParam() {
        PowerMockito
                .when(mMockedHelper.getPropertySettings(
                        Mockito.anyString(), Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
                .thenReturn(mModeList);
        // Test Set Mode Controller
        Assert.assertTrue(
                mCarControlHandler.setModeControllerValue(DEFAULT_AC_ENDPOINT, INTENSITY_INSTANCE, HIGH_VALUE));
        Mockito.verify(mMockedCarManager, Mockito.times(1))
                .setBooleanProperty(PROPERTY_ID_HVAC_MAX_AC_ON, AREA_ID_FULL, true);
        Mockito.verify(mMockedCarManager, Mockito.times(1)).setIntProperty(PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, 6);
    }

    @Test
    public void setRangeControllerWithValidParam() {
        PowerMockito
                .when(mMockedHelper.getPropertySetting(Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
                .thenReturn(new CarControlHelper.PropertySetting(
                        PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, CarControlConstants.DataType.INT, ""));
        // Test Set Range Controller
        Assert.assertTrue(mCarControlHandler.setRangeControllerValue(DEFAULT_FAN_ENDPOINT, SPEED_INSTANCE, 3.0));
        Mockito.verify(mMockedCarManager, Mockito.times(1)).setIntProperty(PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, 3);
    }

    public void adjustRangeControllerWithValidParam() {
        // Test Adjust Range Controller
        PowerMockito
                .when(mMockedHelper.getPropertySetting(Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
                .thenReturn(new CarControlHelper.PropertySetting(
                        PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, CarControlConstants.DataType.INT, ""));
        Assert.assertTrue(mCarControlHandler.adjustRangeControllerValue(DEFAULT_FAN_ENDPOINT, SPEED_INSTANCE, 1.0));
        Mockito.verify(mMockedCarManager, Mockito.times(1)).setIntProperty(PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, 4);
        Assert.assertTrue(mCarControlHandler.adjustRangeControllerValue(DEFAULT_FAN_ENDPOINT, SPEED_INSTANCE, -1.0));
        Mockito.verify(mMockedCarManager, Mockito.times(1)).setIntProperty(PROPERTY_ID_HVAC_FAN_SPEED, AREA_ID_FULL, 3);
    }

    @Test
    public void setControllerWithInvalidParam() {
        // Test Set Power Controller
        Assert.assertFalse(mCarControlHandler.changePowerController(INVALID_DEFAULT_FAN_ENDPOINT, true));
        Assert.assertFalse(mCarControlHandler.changePowerController(INVALID_DEFAULT_FAN_ENDPOINT, false));

        // Test Set Toggle Controller
        Assert.assertFalse(mCarControlHandler.changeToggleController(INVALID_CAR_ENDPOINT, RECIR_INSTANCE, true));
        Assert.assertFalse(mCarControlHandler.changeToggleController(INVALID_CAR_ENDPOINT, RECIR_INSTANCE, false));

        // Test Set Range Controller
        Assert.assertFalse(
                mCarControlHandler.setRangeControllerValue(DEFAULT_FAN_ENDPOINT, INVALID_SPEED_INSTANCE, 3.0));
        Assert.assertFalse(
                mCarControlHandler.setRangeControllerValue(DEFAULT_FAN_ENDPOINT, INVALID_SPEED_INSTANCE, 6.0));

        // Test Set Mode Controller
        Assert.assertFalse(
                mCarControlHandler.setModeControllerValue(DEFAULT_AC_ENDPOINT, INTENSITY_INSTANCE, SUPERHIGH_VALUE));
    }

    @Test
    public void adjustControllerWithInvalidParam() {
        // Test Adjust Range Controller
        Assert.assertFalse(
                mCarControlHandler.adjustRangeControllerValue(DEFAULT_FAN_ENDPOINT, INVALID_SPEED_INSTANCE, 3.0));
        Assert.assertFalse(
                mCarControlHandler.adjustRangeControllerValue(DEFAULT_FAN_ENDPOINT, INVALID_SPEED_INSTANCE, 6.0));
    }
}
