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

#include <vector>
#include "aace/carControl/ClimateControlInterfaceBinder.h"

ClimateControlInterfaceBinder::ClimateControlInterfaceBinder() : ClimateControlInterface() {}

ClimateControlInterfaceBinder::~ClimateControlInterfaceBinder() = default;

void ClimateControlInterfaceBinder::initialize( JNIEnv* env )
{
    // AirConditioningMode
    jclass airConditioningModeEnumClass = env->FindClass( "com/amazon/aace/carControl/ClimateControlInterface$AirConditioningMode" );
    m_enum_AirConditioningMode_MANUAL = NativeLib::FindEnum( env, airConditioningModeEnumClass, "MANUAL", "Lcom/amazon/aace/carControl/ClimateControlInterface$AirConditioningMode;" );
    m_enum_AirConditioningMode_AUTO = NativeLib::FindEnum( env, airConditioningModeEnumClass, "AUTO", "Lcom/amazon/aace/carControl/ClimateControlInterface$AirConditioningMode;" );

    //FanZone
    jclass fanZoneEnumClass = env->FindClass( "com/amazon/aace/carControl/ClimateControlInterface$FanZone" );
    m_enum_FanZone_ALL = NativeLib::FindEnum( env, fanZoneEnumClass, "ALL", "Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;" );

    // TemperatureZone
    jclass temperatureZoneEnumClass = env->FindClass( "com/amazon/aace/carControl/ClimateControlInterface$TemperatureZone" );
    m_enum_TemperatureZone_ALL = NativeLib::FindEnum( env, temperatureZoneEnumClass, "ALL", "Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureZone;" );

    // TemperatureUnit
    jclass temperatureUnitEnumClass = env->FindClass( "com/amazon/aace/carControl/ClimateControlInterface$TemperatureUnit" );
    m_enum_TemperatureUnit_CELSIUS = NativeLib::FindEnum( env, temperatureUnitEnumClass, "CELSIUS", "Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureUnit;" );
    m_enum_TemperatureUnit_FAHRENHEIT = NativeLib::FindEnum( env, temperatureUnitEnumClass, "FAHRENHEIT", "Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureUnit;" );

    m_javaMethod_turnClimateControlOn = env->GetMethodID( getJavaClass(), "turnClimateControlOn", "()Z" );
    m_javaMethod_turnClimateControlOff = env->GetMethodID( getJavaClass(), "turnClimateControlOff", "()Z" );
    m_javaMethod_isClimateControlOn = env->GetMethodID( getJavaClass(), "isClimateControlOn", "()Z" );

    m_javaMethod_turnAirConditioningOn = env->GetMethodID( getJavaClass(), "turnAirConditioningOn", "()Z" );
    m_javaMethod_turnAirConditioningOff = env->GetMethodID( getJavaClass(), "turnAirConditioningOff", "()Z" );
    m_javaMethod_isAirConditioningOn = env->GetMethodID( getJavaClass(), "isAirConditioningOn", "()Z" );


    m_javaMethod_setAirConditioningMode = env->GetMethodID( getJavaClass(), "setAirConditioningMode", "(Lcom/amazon/aace/carControl/ClimateControlInterface$AirConditioningMode;)Z" );
    m_javaMethod_getAirConditioningMode = env->GetMethodID( getJavaClass(), "getAirConditioningMode", "()Lcom/amazon/aace/carControl/ClimateControlInterface$AirConditioningMode;" );

    m_javaMethod_turnFanOn = env->GetMethodID( getJavaClass(), "turnFanOn", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;)Z" );
    m_javaMethod_turnFanOff = env->GetMethodID( getJavaClass(), "turnFanOff", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;)Z" );
    m_javaMethod_isFanOn = env->GetMethodID( getJavaClass(), "isFanOn", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;)Z" );

    m_javaMethod_setFanSpeed = env->GetMethodID( getJavaClass(), "setFanSpeed", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;D)Z" );
    m_javaMethod_adjustFanSpeed = env->GetMethodID( getJavaClass(), "adjustFanSpeed", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;D)Z" );
    m_javaMethod_getFanSpeed = env->GetMethodID( getJavaClass(), "getFanSpeed", "(Lcom/amazon/aace/carControl/ClimateControlInterface$FanZone;)D" );

    m_javaMethod_setTemperature = env->GetMethodID( getJavaClass(), "setTemperature", "(Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureZone;D)Z" );
    m_javaMethod_adjustTemperature = env->GetMethodID( getJavaClass(), "adjustTemperature", "(Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureZone;D)Z" );
    m_javaMethod_getTemperature = env->GetMethodID( getJavaClass(), "getTemperature", "(Lcom/amazon/aace/carControl/ClimateControlInterface$TemperatureZone;)D" );

}

jobject ClimateControlInterfaceBinder::convert( AirConditioningMode mode )
{
    switch( mode ) {
    case AirConditioningMode::MANUAL:
        return m_enum_AirConditioningMode_MANUAL.get();
    case AirConditioningMode::AUTO:
        return m_enum_AirConditioningMode_AUTO.get();
    }
}

jobject ClimateControlInterfaceBinder::convert( FanZone zone )
{
    switch( zone ) {
    case FanZone::ALL:
        return m_enum_FanZone_ALL.get();
    }
}

jobject ClimateControlInterfaceBinder::convert( TemperatureZone zone )
{
    switch( zone ) {
    case TemperatureZone::ALL:
        return m_enum_TemperatureZone_ALL.get();
    }
}

jobject ClimateControlInterfaceBinder::convert( TemperatureUnit unit )
{
    switch( unit ) {
    case TemperatureUnit::CELSIUS:
        return m_enum_TemperatureUnit_CELSIUS.get();
    case TemperatureUnit::FAHRENHEIT:
        return m_enum_TemperatureUnit_FAHRENHEIT.get();
    }
}

aace::carControl::ClimateControlInterface::AirConditioningMode ClimateControlInterfaceBinder::convertAirConditioningMode( JNIEnv* env, jobject obj )
{
    if( m_enum_AirConditioningMode_AUTO.isSameObject( env, obj ) ) {
        return AirConditioningMode::AUTO;
    }
    else {
        return AirConditioningMode::MANUAL;
    }
}

aace::carControl::ClimateControlInterface::FanZone ClimateControlInterfaceBinder::convertFanZone( JNIEnv* env, jobject obj )
{
    if( m_enum_FanZone_ALL.isSameObject( env, obj ) ) {
        return FanZone::ALL;
    }
    else {
        return FanZone::ALL;
    }
}

aace::carControl::ClimateControlInterface::TemperatureZone ClimateControlInterfaceBinder::convertTemperatureZone( JNIEnv* env, jobject obj )
{
    if( m_enum_TemperatureZone_ALL.isSameObject( env, obj ) ) {
        return TemperatureZone::ALL;
    }
    else {
        return TemperatureZone::ALL;
    }
}

aace::carControl::ClimateControlInterface::TemperatureUnit ClimateControlInterfaceBinder::convertTemperatureUnit( JNIEnv* env, jobject obj )
{
    if( m_enum_TemperatureUnit_CELSIUS.isSameObject( env, obj ) ) {
        return TemperatureUnit::CELSIUS;
    }
    else {
        return TemperatureUnit::FAHRENHEIT;
    }
}

// Climate Control

bool ClimateControlInterfaceBinder::turnClimateControlOn() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnClimateControlOn != nullptr ) {
        ThreadContext context;

        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnClimateControlOn );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::turnClimateControlOff() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnClimateControlOff != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnClimateControlOff );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::isClimateControlOn() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_isClimateControlOn != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_isClimateControlOn );
        }
    }
    return result;
}

// Air Conditioning

bool ClimateControlInterfaceBinder::turnAirConditioningOn() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnAirConditioningOn != nullptr ) {
        ThreadContext context;

        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnAirConditioningOn );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::turnAirConditioningOff() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnAirConditioningOff != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnAirConditioningOff );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::isAirConditioningOn() {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_isAirConditioningOn != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_isAirConditioningOn );
        }
    }
    return result;
}


bool ClimateControlInterfaceBinder::setAirConditioningMode( AirConditioningMode mode ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_setAirConditioningMode != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setAirConditioningMode, convert( mode ) );
        }
    }
    return result;
}

aace::carControl::ClimateControlInterface::AirConditioningMode ClimateControlInterfaceBinder::getAirConditioningMode() {
    AirConditioningMode result = AirConditioningMode::MANUAL;
    if ( getJavaObject() != nullptr && m_javaMethod_getAirConditioningMode != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            jobject mode = context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getAirConditioningMode );
            result = convertAirConditioningMode( context.getEnv(), mode );
        }
    }
    return result;
}

// Fan

bool ClimateControlInterfaceBinder::turnFanOn( FanZone zone ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnFanOn != nullptr ) {
        ThreadContext context;

        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnFanOn, convert( zone ) );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::turnFanOff( FanZone zone ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_turnFanOff != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_turnFanOff, convert( zone ) );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::isFanOn( FanZone zone ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_isFanOn != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_isFanOn, convert( zone ) );
        }
    }
    return result;
}


bool ClimateControlInterfaceBinder::setFanSpeed( FanZone zone, double value ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_setFanSpeed != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setFanSpeed, convert( zone ), value );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::adjustFanSpeed( FanZone zone, double delta ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_adjustFanSpeed != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_adjustFanSpeed, convert( zone ), delta );
        }
    }
    return result;
}

double ClimateControlInterfaceBinder::getFanSpeed( FanZone zone ) {
    double result = 0;
    if ( getJavaObject() != nullptr && m_javaMethod_getFanSpeed != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallDoubleMethod( getJavaObject(), m_javaMethod_getFanSpeed, convert( zone ) );
        }
    }
    return result;
}

// Temperature

bool ClimateControlInterfaceBinder::setTemperature( TemperatureZone zone, double value ) {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_setTemperature != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setTemperature, convert( zone ), value );
        }
    }
    return result;
}

bool ClimateControlInterfaceBinder::adjustTemperature( TemperatureZone zone, double delta )  {
    bool result = false;
    if ( getJavaObject() != nullptr && m_javaMethod_adjustTemperature != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_adjustTemperature, convert( zone ), delta );
        }
    }
    return result;
}

double ClimateControlInterfaceBinder::getTemperature( TemperatureZone zone ) {
    double result = 0;
    if ( getJavaObject() != nullptr && m_javaMethod_getTemperature != nullptr ) {
        ThreadContext context;
        if ( context.isValid() ) {
            result = context.getEnv()->CallDoubleMethod( getJavaObject(), m_javaMethod_getTemperature, convert( zone ) );
        }
    }
    return result;
}

// JNI
#define CLIMATECONTROLINTERFACE(cptr) ((ClimateControlInterfaceBinder *) cptr)

extern "C" {

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addClimateControlSwitch( JNIEnv * env , jobject /* this */, jlong cptr ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        return climateControl->addClimateControlSwitch();
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addAirConditioningSwitch( JNIEnv * env , jobject /* this */, jlong cptr ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        return climateControl->addAirConditioningSwitch();
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addAirConditioningModeSelector( JNIEnv * env , jobject /* this */, jlong cptr, jobjectArray jAirConditioningModes ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        std::vector<aace::carControl::ClimateControlInterface::AirConditioningMode> modes;

        int count = env->GetArrayLength(jAirConditioningModes);

        for ( int i = 0; i < count; i++ ) {
            auto mode = climateControl->convertAirConditioningMode( env, env->GetObjectArrayElement( jAirConditioningModes, i ) );
            modes.push_back( mode );
        }
        return climateControl->addAirConditioningModeSelector( modes );
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addFanSwitch( JNIEnv * env , jobject /* this */, jlong cptr, jobject jFanZone ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        auto zone = climateControl->convertFanZone( env, jFanZone );
        return climateControl->addFanSwitch( zone );
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addFanSpeedControl( JNIEnv * env , jobject /* this */, jlong cptr, jobject jFanZone, jdouble minimum, jdouble maximum, jdouble precision ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        auto zone = climateControl->convertFanZone( env, jFanZone );
        return climateControl->addFanSpeedControl( zone, minimum, maximum, precision );
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_carControl_ClimateControlInterface_addTemperatureControl( JNIEnv * env , jobject /* this */, jlong cptr, jobject jTemperatureZone, jdouble minimum, jdouble maximum, jdouble precision, jobject jTemperatureUnit ) {
        auto climateControl = CLIMATECONTROLINTERFACE(cptr);
        auto zone = climateControl->convertTemperatureZone( env, jTemperatureZone );
        auto unit = climateControl->convertTemperatureUnit( env, jTemperatureUnit );
        return climateControl->addTemperatureControl( zone, minimum, maximum, precision, unit );
    }

}

// END OF FILE
