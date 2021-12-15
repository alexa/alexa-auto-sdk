package com.amazon.alexa.auto.apps.common.util.config;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.ExecutorService;

import io.reactivex.rxjava3.core.Single;

/**
 * A helper object to provide Alexa property support.
 */
public class AlexaPropertyManager {
    private static final String TAG = AlexaPropertyManager.class.getSimpleName();

    private final Uri mUri = Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI);

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final ExecutorService mExecutorService;
    @NonNull
    private final Handler mMainThreadHandler;

    public AlexaPropertyManager(@NonNull WeakReference<Context> contextWk, @NonNull ExecutorService executorService) {
        mContextWk = contextWk;
        mExecutorService = executorService;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Query Alexa property with property name.
     * @param propName property name.
     * @return future for property value.
     */
    public Single<Optional<String>> getAlexaProperty(String propName) {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                Optional<String> propValue = getAlexaPropertySync(propName);
                mMainThreadHandler.post(() -> emitter.onSuccess(propValue));
            });
        });
    }

    /**
     * Query Alexa property with property name.
     * @param propName property name.
     * @return future for property value.
     */
    public Single<Optional<Boolean>> getAlexaPropertyBoolean(String propName) {
        return getAlexaProperty(propName).map(propValue -> propValue.map(Boolean::parseBoolean));
    }

    /**
     * Update Alexa property with name and value.
     * @param propName property name.
     * @param value property value.
     * @return future success/failure status.
     */
    public Single<Boolean> updateAlexaProperty(String propName, String value) {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                boolean updateSucceeded = updateAlexaPropertySync(propName, value);
                mMainThreadHandler.post(() -> emitter.onSuccess(updateSucceeded));
            });
        });
    }

    /**
     * Update Alexa property with name and value.
     * @param propName property name.
     * @param value property value.
     * @return future success/failure status.
     */
    public Single<Boolean> updateAlexaPropertyBoolean(String propName, boolean value) {
        return updateAlexaProperty(propName, String.valueOf(value));
    }

    private Optional<String> getAlexaPropertySync(String name) {
        try (Cursor cursor = mContextWk.get().getContentResolver().query(mUri, null, name, null, null)) {
            if (cursor != null) {
                cursor.moveToFirst();
                return Optional.of(cursor.getString(1));
            }
        }

        return Optional.empty();
    }

    private boolean updateAlexaPropertySync(String name, String value) {
        ContentValues values = new ContentValues();
        values.put(name, value);

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        // If one row is updated, we are through with update.
        return context.getContentResolver().update(mUri, values, name, null) == 1;
    }
}
