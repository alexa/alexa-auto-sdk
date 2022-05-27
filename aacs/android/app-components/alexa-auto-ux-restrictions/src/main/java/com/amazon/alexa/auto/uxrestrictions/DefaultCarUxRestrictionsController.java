/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.uxrestrictions;

import android.car.Car;
import android.car.drivingstate.CarUxRestrictions;
import android.car.drivingstate.CarUxRestrictionsManager;
import android.content.Context;
import android.util.Log;

import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestriction;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionStatus;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionsController;
import com.amazon.alexa.auto.apps.common.Constants;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Default implementation for car UX restrictions controller. It initializes Android CarUxRestrictionsManager,
 * registers and unregisters OnUxRestrictionsChangedListener.
 */
public class DefaultCarUxRestrictionsController implements CarUxRestrictionsController {
    private static final String TAG = DefaultCarUxRestrictionsController.class.getSimpleName();

    // Android car UX restrictions code
    private static final int UX_RESTRICTIONS_DRIVING_CODE = CarUxRestrictions.UX_RESTRICTIONS_FULLY_RESTRICTED
            - CarUxRestrictions.UX_RESTRICTIONS_NO_VOICE_TRANSCRIPTION;
    private static final int UX_RESTRICTIONS_NO_RESTRICTION_CODE = CarUxRestrictions.UX_RESTRICTIONS_BASELINE;

    private final WeakReference<Context> mContext;
    private BehaviorSubject<CarUxRestrictionStatus> uxRestrictionStatusColdStream;

    private CarUxRestrictionsManager mCarUxRestrictionsManager;
    private CarUxRestriction mCarUxRestriction;
    private boolean mIsRequiresDistractionOptimization;

    public DefaultCarUxRestrictionsController(WeakReference<Context> context) {
        mContext = context;
        uxRestrictionStatusColdStream = BehaviorSubject.create();

        initializeAndroidCarRestrictionsManager();
    }

    private void initializeAndroidCarRestrictionsManager() {
        Car car = Car.createCar(mContext.get());
        if (car == null) {
            Log.e(TAG, "Car Service is not available, Car object is null");
        } else {
            mCarUxRestrictionsManager = (CarUxRestrictionsManager) car.getCarManager(Car.CAR_UX_RESTRICTION_SERVICE);
            parseActiveUxRestriction(mCarUxRestrictionsManager.getCurrentCarUxRestrictions().getActiveRestrictions());
            uxRestrictionStatusColdStream.onNext(
                    new CarUxRestrictionStatus(mIsRequiresDistractionOptimization, mCarUxRestriction));
        }
    }

    @Override
    public CarUxRestriction getActiveCarUxRestriction() {
        return mCarUxRestriction;
    }

    @Override
    public Observable<CarUxRestrictionStatus> observeCarUxRestrictionChanged() {
        return uxRestrictionStatusColdStream;
    }

    @Override
    public void registerListener() {
        Log.d(TAG, "Register UX restrictions listener.");
        CarUxRestrictionsManager.OnUxRestrictionsChangedListener uxrChangeListener =
                new CarUxRestrictionsManager.OnUxRestrictionsChangedListener() {
                    @Override
                    public void onUxRestrictionsChanged(CarUxRestrictions carUxRestrictions) {
                        Log.d(TAG, "onUxRestrictionsChanged");
                        mIsRequiresDistractionOptimization = carUxRestrictions.isRequiresDistractionOptimization();
                        parseActiveUxRestriction(carUxRestrictions.getActiveRestrictions());
                        uxRestrictionStatusColdStream.onNext(
                                new CarUxRestrictionStatus(mIsRequiresDistractionOptimization, mCarUxRestriction));
                    }
                };
        mCarUxRestrictionsManager.registerListener(uxrChangeListener);
        uxrChangeListener.onUxRestrictionsChanged(mCarUxRestrictionsManager.getCurrentCarUxRestrictions());
    }

    @Override
    public void unregisterListener() {
        Log.d(TAG, "Unregister UX restrictions listener.");
        mCarUxRestrictionsManager.unregisterListener();
    }

    /**
     * Parse active restriction code to a CarUxRestriction object.
     * @param restrictionCode active restriction code.
     */
    private void parseActiveUxRestriction(int restrictionCode) {
        if (restrictionCode >= UX_RESTRICTIONS_DRIVING_CODE) {
            mCarUxRestriction = new CarUxRestriction(Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_MOVING);
            mIsRequiresDistractionOptimization = true;
        } else if (restrictionCode == UX_RESTRICTIONS_NO_RESTRICTION_CODE) {
            mCarUxRestriction = new CarUxRestriction(Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_PARKED);
            mIsRequiresDistractionOptimization = false;
        } else {
            Log.w(TAG, "Fail to recognize active ux restriction code: " + restrictionCode);
        }
    }
}
