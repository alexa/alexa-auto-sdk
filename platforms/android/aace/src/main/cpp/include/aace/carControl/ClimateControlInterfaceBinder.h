/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/***************************************************************************************
 * CAUTION: THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT MODIFY THIS FILE!          *
 *          ANY NECESSARY MODIFICATIONS SHOULD BE INCORPORATED INTO THE GENERATOR.     *
 ***************************************************************************************/

#ifndef AACE_CAR_CONTROL_CLIMATE_CONTROL_INTERFACE_BINDER_H
#define AACE_CAR_CONTROL_CLIMATE_CONTROL_INTERFACE_BINDER_H

#include "AACE/CarControl/ClimateControlInterface.h"
#include "aace/core/PlatformInterfaceBinder.h"

class ClimateControlInterfaceBinder : public PlatformInterfaceBinder, public aace::carControl::ClimateControlInterface {
public:
    ClimateControlInterfaceBinder();
    virtual ~ClimateControlInterfaceBinder();

protected:
    void initialize( JNIEnv* env ) override;

public:
    // Climate Control

    bool turnClimateControlOn() override;
    bool turnClimateControlOff() override;
    bool isClimateControlOn() override;

    // Air Conditioning

    bool turnAirConditioningOn() override;
    bool turnAirConditioningOff() override;
    bool isAirConditioningOn() override;

    bool setAirConditioningMode( AirConditioningMode mode ) override;
    AirConditioningMode getAirConditioningMode() override;

    // Fan

    bool turnFanOn( FanZone zone ) override;
    bool turnFanOff( FanZone zone ) override;
    bool isFanOn( FanZone zone ) override;

    bool setFanSpeed( FanZone zone, double value ) override;
    bool adjustFanSpeed( FanZone zone, double delta ) override;
    double getFanSpeed( FanZone zone ) override;

    // Temperature

    bool setTemperature( TemperatureZone zone, double value ) override;
    bool adjustTemperature( TemperatureZone zone, double delta )  override;
    double getTemperature( TemperatureZone zone ) override;

    AirConditioningMode convertAirConditioningMode( JNIEnv* env, jobject obj );
    FanZone convertFanZone( JNIEnv* env, jobject obj );
    TemperatureZone convertTemperatureZone( JNIEnv* env, jobject obj );
    TemperatureUnit convertTemperatureUnit( JNIEnv* env, jobject obj );

private:
    jobject convert( AirConditioningMode mode );
    jobject convert( FanZone zone );
    jobject convert( TemperatureZone zone );
    jobject convert( TemperatureUnit unit );

    ObjectRef m_enum_AirConditioningMode_MANUAL;
    ObjectRef m_enum_AirConditioningMode_AUTO;

    ObjectRef m_enum_FanZone_ALL;

    ObjectRef m_enum_TemperatureZone_ALL;

    ObjectRef m_enum_TemperatureUnit_CELSIUS;
    ObjectRef m_enum_TemperatureUnit_FAHRENHEIT;

    jmethodID m_javaMethod_turnClimateControlOn = nullptr;
    jmethodID m_javaMethod_turnClimateControlOff = nullptr;
    jmethodID m_javaMethod_isClimateControlOn = nullptr;

    jmethodID m_javaMethod_turnAirConditioningOn = nullptr;
    jmethodID m_javaMethod_turnAirConditioningOff = nullptr;
    jmethodID m_javaMethod_isAirConditioningOn = nullptr;

    jmethodID m_javaMethod_setAirConditioningMode = nullptr;
    jmethodID m_javaMethod_getAirConditioningMode = nullptr;

    jmethodID m_javaMethod_turnFanOn = nullptr;
    jmethodID m_javaMethod_turnFanOff = nullptr;
    jmethodID m_javaMethod_isFanOn = nullptr;

    jmethodID m_javaMethod_setFanSpeed = nullptr;
    jmethodID m_javaMethod_adjustFanSpeed = nullptr;
    jmethodID m_javaMethod_getFanSpeed = nullptr;

    jmethodID m_javaMethod_setTemperature = nullptr;
    jmethodID m_javaMethod_adjustTemperature = nullptr;
    jmethodID m_javaMethod_getTemperature = nullptr;
};

#endif //AACE_CAR_CONTROL_CLIMATE_CONTROL_INTERFACE_BINDER_H
