package com.amazon.alexa.auto.apps.common.util;

import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * Preconditions checker.
 */
public class Preconditions {
    /**
     * Ensure that expression evaluates to true.
     *
     * @param expression expression to evaluate.
     */
    public static void checkArgument(boolean expression) {
        if (!expression) {
            throw new IllegalArgumentException();
        }
    }

    /**
     * Ensure that expression evaluates to true.
     *
     * @param expression expression to evaluate.
     * @param errorMessage Error message to accompany.
     */
    public static void checkArgument(boolean expression, @NonNull Object errorMessage) {
        if (!expression) {
            throw new IllegalArgumentException(String.valueOf(errorMessage));
        }
    }

    /**
     * Ensures the object passed to the function is not null.
     *
     * @param object Object to check
     */
    public static void checkNotNull(@Nullable Object object) {
        if (object == null) {
            throw new NullPointerException();
        }
    }

    /**
     * Ensure that function is called from main thread.
     */
    public static void checkMainThread() {
        checkArgument(Looper.myLooper() == Looper.getMainLooper());
    }

    /**
     * Ensure that function is not called from main thread.
     */
    public static void checkNotMainThread() {
        checkArgument(Looper.myLooper() != Looper.getMainLooper());
    }
}
