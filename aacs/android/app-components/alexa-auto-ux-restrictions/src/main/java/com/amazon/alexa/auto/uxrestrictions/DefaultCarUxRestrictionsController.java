package com.amazon.alexa.auto.uxrestrictions;

import android.car.Car;
import android.car.drivingstate.CarUxRestrictions;
import android.car.drivingstate.CarUxRestrictionsManager;
import android.content.Context;
import android.util.Log;

import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestriction;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionsController;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionStatus;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Default implementation for car UX restrictions controller. It initializes Android CarUxRestrictionsManager,
 * registers and unregisters OnUxRestrictionsChangedListener.
 */
public class DefaultCarUxRestrictionsController implements CarUxRestrictionsController {
    private static final String TAG = DefaultCarUxRestrictionsController.class.getSimpleName();

    //Android car UX restrictions code
    private static final int UX_RESTRICTIONS_FULLY_RESTRICTED_CODE = 511;
    private static final int UX_RESTRICTIONS_NO_RESTRICTION_CODE = 0;

    private static final String UX_RESTRICTIONS_FULLY_RESTRICTED_NAME = "UX_RESTRICTIONS_FULLY_RESTRICTED";
    private static final String UX_RESTRICTIONS_NO_RESTRICTION_NAME = "UX_RESTRICTIONS_NO_RESTRICTION";

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
            mCarUxRestrictionsManager = (CarUxRestrictionsManager)
                    car.getCarManager(Car.CAR_UX_RESTRICTION_SERVICE);
            parseActiveUxRestriction(mCarUxRestrictionsManager.getCurrentCarUxRestrictions().getActiveRestrictions());
            uxRestrictionStatusColdStream.onNext(new CarUxRestrictionStatus(mIsRequiresDistractionOptimization, mCarUxRestriction));
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
                        uxRestrictionStatusColdStream.onNext(new CarUxRestrictionStatus(mIsRequiresDistractionOptimization, mCarUxRestriction));
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
     * @param activeRestriction active restriction code.
     */
    private void parseActiveUxRestriction(int activeRestriction) {
        switch (activeRestriction) {
            case UX_RESTRICTIONS_FULLY_RESTRICTED_CODE:
                mCarUxRestriction = new CarUxRestriction(UX_RESTRICTIONS_FULLY_RESTRICTED_NAME);
                mIsRequiresDistractionOptimization = true;
                break;
            case UX_RESTRICTIONS_NO_RESTRICTION_CODE:
                mCarUxRestriction = new CarUxRestriction(UX_RESTRICTIONS_NO_RESTRICTION_NAME);
                mIsRequiresDistractionOptimization = false;
                break;
            default:
                Log.w(TAG,"Fail to recognize active ux restriction code.");
        }
    }
}
