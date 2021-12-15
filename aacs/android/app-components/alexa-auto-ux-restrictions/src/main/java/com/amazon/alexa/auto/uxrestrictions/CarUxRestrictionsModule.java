package com.amazon.alexa.auto.uxrestrictions;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionsController;
import com.amazon.alexa.auto.apps.common.Constants;

import org.json.JSONStringer;

import java.lang.ref.WeakReference;

import androidx.annotation.NonNull;
import io.reactivex.rxjava3.disposables.Disposable;

public class CarUxRestrictionsModule implements ModuleInterface {
    private static final String TAG = CarUxRestrictionsModule.class.getSimpleName();
    private static final String ANDROID_CAR_PACKAGE = "com.android.car";

    private Context mContext;
    private Disposable mCarUxRestrictionDisposable;

    @NonNull
    private AACSMessageSender mAACSSender;

    @Override
    public void initialize(Context context) {
        Log.d(TAG, "initialize");
        mContext = context;
        mAACSSender = new AACSMessageSender(new WeakReference<Context>(mContext), new AACSSender());
        initializeCarUxRestrictionsManager();
    }

    @Override
    public void uninitialize(Context context) {
        Log.d(TAG, "uninitialize");
        AlexaApp mApp = AlexaApp.from(context);
        mApp.getRootComponent()
                .getComponent(CarUxRestrictionsController.class)
                .ifPresent(carUxRestrictionsController -> {
                    carUxRestrictionsController.unregisterListener();
                    mApp.getRootComponent().deactivateScope(CarUxRestrictionsController.class);
                });

        if (mCarUxRestrictionDisposable != null) {
            mCarUxRestrictionDisposable.dispose();
            mCarUxRestrictionDisposable = null;
        }
    }


    /**
     * Initialize car UX restrictions manager. If Android car package exists on the device,
     * we can use Android car UX restrictions APIs to get current UX restriction value and subscribe to the restrictions update.
     * Otherwise, OEM needs to provide their own implementation for car UX restrictions updates.
     */
    private void initializeCarUxRestrictionsManager() {
        AlexaApp mApp = AlexaApp.from(mContext);
        if (isAndroidCardPackageExisted()) {
            Log.d(TAG, "Android car library exists, initialize default CarUxRestrictionsController.");
            mApp.getRootComponent().activateScope(new DefaultCarUxRestrictionsController(new WeakReference<>(mContext)));
            mApp.getRootComponent()
                    .getComponent(CarUxRestrictionsController.class)
                    .ifPresent(CarUxRestrictionsController::registerListener);
        } else {
            Log.d(TAG, "Android Car UX Restrictions does not supported, please provide your own car UX restrictions implementation, and activate it as a scoped component");
        }

        observeCarUxRestrictionsChanges();
    }

    /**
     * Check if Android car package exists on the device.
     *
     * @return true if exists, otherwise false.
     */
    public boolean isAndroidCardPackageExisted() {
        PackageManager pm = mContext.getPackageManager();
        try {
            pm.getPackageInfo(ANDROID_CAR_PACKAGE, PackageManager.GET_META_DATA);
        } catch (PackageManager.NameNotFoundException e) {
            Log.d(TAG, "Android card package does not exist.");
            return false;
        }
        return true;
    }

    /**
     * Observe Car UX restrictions changes on the device.
     * Currently, we have one requirement regarding to sending AASB message with APL runtime property based on the UX restrictions updates.
     */
    private void observeCarUxRestrictionsChanges() {
        AlexaApp mApp = AlexaApp.from(mContext);
        mApp.getRootComponent().getComponent(CarUxRestrictionsController.class).ifPresent(carUxRestrictionController -> {
            mCarUxRestrictionDisposable = carUxRestrictionController.observeCarUxRestrictionChanged().subscribe(onUxRestrictionStatus -> {
                String payload;
                String drivingStateValue;
                if (onUxRestrictionStatus.isRequiredUXRestriction()) {
                    drivingStateValue = Constants.APL_RUNTIME_PROPERTY_DRIVING_STATE_VALUE_MOVING;
                } else {
                    drivingStateValue = Constants.APL_RUNTIME_PROPERTY_DRIVING_STATE_VALUE_PARKED;
                }

                payload = new JSONStringer()
                        .object()
                        .key(Constants.APL_RUNTIME_PROPERTY_NAME_KEY)
                        .value(Constants.APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME)
                        .key(Constants.APL_RUNTIME_PROPERTY_VALUE_KEY)
                        .value(drivingStateValue)
                        .endObject()
                        .toString();
                mAACSSender.sendMessage(Topic.APL, Action.APL.SET_PLATFORM_PROPERTY, payload);

                saveDrivingState(drivingStateValue);
            });
        });
    }

    /**
     * Saving APL driving state properties for rendering APL template with the updated APL driving state.
     * @param drivingStateValue parked/driving
     */
    private void saveDrivingState(String drivingStateValue) {
        SharedPreferences.Editor editor = mContext.getSharedPreferences(Constants.APL_RUNTIME_PROPERTIES, 0).edit();
        editor.putString(Constants.APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME, drivingStateValue);
        editor.apply();
    }
}