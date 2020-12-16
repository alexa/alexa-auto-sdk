package com.amazon.alexa.auto.voiceinteraction.config;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;

import java.lang.ref.WeakReference;

/**
 * A helper object to provide Alexa property support.
 */
public class AlexaPropertyManager {
    private static final String TAG = AlexaPropertyManager.class.getSimpleName();

    private final Uri mUri = Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI);

    @NonNull
    private final WeakReference<Context> mContextWk;

    public AlexaPropertyManager(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
    }

    /**
     * Query Alexa property with property name.
     * @param name property name.
     * @return property value.
     */
    public String getAlexaProperty(String name) {
        Cursor cursor = mContextWk.get().getContentResolver().query(mUri, null, name, null, null);
        if (cursor != null) {
            cursor.moveToFirst();
            return cursor.getString(1);
        }

        return null;
    }

    /**
     * Update Alexa property with name and value.
     * @param name property name.
     * @param value property value.
     */
    public void updateAlexaProperty(String name, String value) {
        ContentValues values = new ContentValues();
        values.put(name, value);
        mContextWk.get().getContentResolver().update(mUri, values, name, null);
    }
}
