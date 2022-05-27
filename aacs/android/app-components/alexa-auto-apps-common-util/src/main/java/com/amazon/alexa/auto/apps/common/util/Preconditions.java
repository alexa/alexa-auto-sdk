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
