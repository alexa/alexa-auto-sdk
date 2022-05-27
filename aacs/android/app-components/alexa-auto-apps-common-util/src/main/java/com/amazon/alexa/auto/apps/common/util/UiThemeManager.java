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
package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.apl.APLTheme;
import com.amazon.alexa.auto.apps.common.Constants;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONStringer;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Manager class to handle ambient light sensor and Alexa Auto theme update.
 */
public class UiThemeManager {
    private static final String TAG = UiThemeManager.class.getSimpleName();
    public static final String UI_MODE = "com.amazon.alexa.auto.uiMode";
    public static final String UI_LIGHT_THEME = "com.amazon.alexa.auto.ui.lightTheme";
    public static final String UI_DARK_THEME = "com.amazon.alexa.auto.ui.darkTheme";

    // AACS Intent
    public static final String AACS_INTENT_PAYLOAD_NAME = "name";
    public static final String AACS_INTENT_PAYLOAD_VALUE = "value";

    // UI Modes
    public static final String UI_MODE_KEY = "uiMode";
    public static final String UI_MODE_VALUE_DAY = "day";
    public static final String UI_MODE_VALUE_NIGHT = "night";

    public static final String THEME_NAME = "theme";
    public static final String THEME_ID = "themeId";
    public static final String UI_MODE_VALUE_DARK = "dark";
    public static final String UI_MODE_VALUE_LIGHT = "light";
    public static final String THEME_VALUE_BLACK = "black";
    public static final String THEME_VALUE_GRAY = "gray";
    public static final String THEME_VALUE_GRAY_ONE = "gray1";
    public static final String THEME_VALUE_GRAY_TWO = "gray2";

    APLThemeDirectiveReceiver mAPLThemeDirectiveReceiver;

    /**
     * UI mode type.
     */
    public enum UiModeType {
        LIGHT(1),
        DARK(2);

        private final int value;

        /**
         * Enum constructor.
         *
         * @param newValue new enum value.
         */
        UiModeType(final int newValue) {
            value = newValue;
        }

        /**
         * Get enum value.
         *
         * @return integer value.
         */
        public int getValue() {
            return value;
        }

        /**
         * String value as lower case.
         */
        @Override
        public String toString() {
            return name().toLowerCase();
        }
    }

    private final Context mContext;
    private final BehaviorSubject<UiModeType> mUiModeUpdated;

    private SensorEventListener mSensorEventListener;
    private UiModeType mCurrentAlsBasedMode;

    SensorManager mSensorManager;
    AACSMessageSender mAACSMessageSender;

    private float mCurrentALSValue = -1;

    private static final float ALS_THRESHOLD_VALUE = 10.0f;

    public UiThemeManager(Context context, AACSMessageSender aacsMessageSender) {
        mContext = context;
        mUiModeUpdated = BehaviorSubject.createDefault(UiModeType.DARK);
        mAACSMessageSender = aacsMessageSender;
    }

    public void init() {
        Log.i(TAG, "init");
        mSensorManager = (SensorManager) mContext.getSystemService(Context.SENSOR_SERVICE);
        mAPLThemeDirectiveReceiver = new APLThemeDirectiveReceiver();

        EventBus.getDefault().register(mAPLThemeDirectiveReceiver);

        initAmbientLightSensor();
    }

    @VisibleForTesting
    void init(SensorManager sensorManager) {
        mSensorManager = sensorManager;
        initAmbientLightSensor();
    }

    public void destroy() {
        Log.i(TAG, "destroy");
        mSensorManager.unregisterListener(mSensorEventListener);
        EventBus.getDefault().unregister(mAPLThemeDirectiveReceiver);
    }

    public Observable<UiModeType> getUiModeUpdatedObservable() {
        return mUiModeUpdated;
    }

    /**
     * Init ambient light sensor.
     */
    private void initAmbientLightSensor() {
        Log.i(TAG, "initAmbientLightSensor");
        initSensorListener();

        if (mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT) != null) {
            Log.i(TAG, "Device has ALS");
            Sensor ambientLightSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
            mSensorManager.registerListener(mSensorEventListener, ambientLightSensor, SensorManager.SENSOR_DELAY_UI);
        } else {
            Log.e(TAG, "Device has no ALS");
        }
    }

    /**
     * Init sensor listener.
     */
    private void initSensorListener() {
        mSensorEventListener = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent sensorEvent) {
                float value = sensorEvent.values[0];
                if (sensorEvent.sensor.getType() == Sensor.TYPE_LIGHT) {
                    if (value > ALS_THRESHOLD_VALUE) {
                        if (mCurrentALSValue <= ALS_THRESHOLD_VALUE) {
                            Log.i(TAG, "setting day theme");
                            mCurrentALSValue = value;
                            handleAlsUpdate(UiModeType.LIGHT);
                        }
                    } else {
                        if (mCurrentALSValue > ALS_THRESHOLD_VALUE || mCurrentALSValue == -1) {
                            Log.i(TAG, "setting night theme");
                            mCurrentALSValue = value;
                            handleAlsUpdate(UiModeType.DARK);
                        }
                    }
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int i) {}
        };
    }

    /**
     * Handle ambient light sensor update.
     *
     * @param uiModeType @c ThemeType instance.
     */
    void handleAlsUpdate(UiModeType uiModeType) {
        Log.i(TAG, "handleAlsUpdate");
        if (uiModeType != getCurrentUIMode()) {
            mCurrentAlsBasedMode = uiModeType;
            mUiModeUpdated.onNext(uiModeType);

            saveCurrentUIMode(uiModeType);
            saveAPLThemeProperties(uiModeType);
            sendUiModeUpdate(uiModeType);

            sendThemeUpdate(uiModeType);
        }
    }

    /**
     * Get current UI mode.
     *
     * @return UiModeType UI mode type.
     */
    UiModeType getCurrentUIMode() {
        if (mCurrentAlsBasedMode != null) {
            return mCurrentAlsBasedMode;
        }

        return UiModeType.DARK;
    }

    private void saveCurrentUIMode(UiModeType uiMode) {
        SharedPreferences.Editor editor = mContext.getSharedPreferences(UI_MODE, 0).edit();
        editor.putString(UI_MODE, uiMode.toString());
        editor.apply();
    }

    /**
     * Update APL runtime property.
     */
    private void sendUiModeUpdate(UiModeType uiMode) {
        String payload;
        try {
            if (uiMode.equals(UiModeType.LIGHT)) {
                payload = new JSONStringer()
                                  .object()
                                  .key(AACS_INTENT_PAYLOAD_NAME)
                                  .value(UI_MODE_KEY)
                                  .key(AACS_INTENT_PAYLOAD_VALUE)
                                  .value(UI_MODE_VALUE_DAY)
                                  .endObject()
                                  .toString();
            } else if (uiMode.equals(UiModeType.DARK)) {
                payload = new JSONStringer()
                                  .object()
                                  .key(AACS_INTENT_PAYLOAD_NAME)
                                  .value(UI_MODE_KEY)
                                  .key(AACS_INTENT_PAYLOAD_VALUE)
                                  .value(UI_MODE_VALUE_NIGHT)
                                  .endObject()
                                  .toString();
            } else {
                payload = null;
                Log.e(TAG, "UI mode is invalid.");
            }

            if (payload != null) {
                mAACSMessageSender.sendMessage(Topic.APL, Action.APL.SET_PLATFORM_PROPERTY, payload);
            }
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse UI mode payload.");
        }
    }

    private void sendThemeUpdate(UiModeType uiMode) {
        String themeId = getSavedThemeId(uiMode);
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(AACS_INTENT_PAYLOAD_NAME)
                                     .value(THEME_ID)
                                     .key(AACS_INTENT_PAYLOAD_VALUE)
                                     .value(themeId)
                                     .endObject()
                                     .toString();
            mAACSMessageSender.sendMessage(Topic.APL, Action.APL.SET_PLATFORM_PROPERTY, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse UI theme payload.");
        }
    }

    String getSavedThemeId(UiModeType uiMode) {
        SharedPreferences sharedPreferences = null;
        if (uiMode.equals(UiModeType.LIGHT)) {
            sharedPreferences = mContext.getSharedPreferences(UI_LIGHT_THEME, 0);
        } else if (uiMode.equals(UiModeType.DARK)) {
            sharedPreferences = mContext.getSharedPreferences(UI_DARK_THEME, 0);
        }

        if (sharedPreferences != null) {
            return sharedPreferences.getString(THEME_ID, "");
        } else {
            return "";
        }
    }

    /**
     * Saving APL theme properties for rendering APL template with the updated APL theme.
     * @param uiMode day/night mode
     */
    private void saveAPLThemeProperties(UiModeType uiMode) {
        String themeId = getSavedThemeId(uiMode);
        SharedPreferences.Editor editor = mContext.getSharedPreferences(Constants.APL_RUNTIME_PROPERTIES, 0).edit();
        if (themeId.isEmpty()) {
            editor.putString(THEME_NAME, uiMode.toString());
        } else {
            editor.putString(THEME_NAME, uiMode + "-" + getSavedThemeId(uiMode));
        }
        editor.apply();
    }

    /**
     * Subscribe APL theme change and update APL runtime property.
     */
    @VisibleForTesting
    class APLThemeDirectiveReceiver {
        @Subscribe
        public void OnReceive(APLTheme theme) {
            mAACSMessageSender.sendMessage(Topic.APL, Action.APL.SET_PLATFORM_PROPERTY, theme.getThemePayload());
        }
    }
}
