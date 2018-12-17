package com.amazon.sampleapp.impl.CarControl;

import android.content.Context;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.aace.carControl.ClimateControlInterface;
import com.amazon.sampleapp.R;

public class ClimateControlHandler extends ClimateControlInterface
{
    private static final String sTag = "ClimateControl";

    private LoggerHandler mLogger;
    private boolean mPower;
    private boolean mAC;
    private ClimateControlInterface.AirConditioningMode mACMode;
    private boolean mAuto;
    private boolean mFan;
    private double mFanSpeed;
    private double mTemperature;
    
    public ClimateControlHandler( Context context, LoggerHandler logger ) {
        mLogger = logger;
        mPower = false;
        mAC = false;
        mACMode = ClimateControlInterface.AirConditioningMode.MANUAL;
        mAuto = false;
        mFan = false;
        mFanSpeed = 0;
        mTemperature = 60;
    }

    // Climate Control

    @Override
    public boolean turnClimateControlOn() {
        mPower = true;
        mLogger.postInfo(sTag, "Power On");
        return true;
    }

    @Override
    public boolean turnClimateControlOff() {
        mPower = false;
        mLogger.postInfo(sTag, "Power Off");
        return true;
    }

    @Override
    public boolean isClimateControlOn() {
        return mPower;
    }

    // Air Conditioning

    @Override
    public boolean turnAirConditioningOn() {
        mAC = true;
        return true;
    }
    @Override
    public boolean turnAirConditioningOff() {
        mAC = false;
        return true;
    }
    @Override
    public boolean isAirConditioningOn() {
        return mAC;
    }

    @Override
    public boolean setAirConditioningMode( ClimateControlInterface.AirConditioningMode mode ) {
        mACMode = mode;
        if ( mode == ClimateControlInterface.AirConditioningMode.AUTO ) {
            mLogger.postInfo(sTag, "Auto On");
        }
        else {
            mLogger.postInfo(sTag, "Auto Off");
        }
        return true;
    }

    @Override
    public ClimateControlInterface.AirConditioningMode getAirConditioningMode() {
        return mACMode;
    }

    // Fan

    @Override
    public boolean turnFanOn( FanZone zone ) {
        mFan = true;
        mLogger.postInfo(sTag, "Fan On");
        return true;
    }
    @Override
    public boolean turnFanOff( FanZone zone ) {
        mFan = false;
        mLogger.postInfo(sTag, "Fan Off");
        return true;
    }
    @Override
    public boolean isFanOn( FanZone zone ) {
        return mFan;
    }

    @Override
    public boolean setFanSpeed( FanZone zone, double value ) {
        mFanSpeed = value;
        mLogger.postInfo(sTag, "Fan Speed = " + mFanSpeed);
        return true;
    }

    @Override
    public boolean adjustFanSpeed( FanZone zone, double delta ) {
        mFanSpeed += delta;
        mLogger.postInfo(sTag, "Fan Speed = " + mFanSpeed);
        return true;
    }

    @Override
    public double getFanSpeed( FanZone zone ) {
        return mFanSpeed;
    }

    // Temperature

    @Override
    public boolean setTemperature( ClimateControlInterface.TemperatureZone zone, double value ) {
        mTemperature = value;
        mLogger.postInfo(sTag, "Temperature = " + mTemperature);
        return true;
    }

    @Override
    public boolean adjustTemperature( ClimateControlInterface.TemperatureZone zone, double delta ) {
        mTemperature += delta;
        mLogger.postInfo(sTag, "Temperature = " + mTemperature);
        return true;
    }

    @Override
    public double getTemperature( ClimateControlInterface.TemperatureZone zone ) {
        return mTemperature;
    }
}
