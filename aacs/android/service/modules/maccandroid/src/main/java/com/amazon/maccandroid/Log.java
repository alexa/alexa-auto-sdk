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
package com.amazon.maccandroid;

public class Log {
    private static final String PRETAG = "MACCAndroid-";

    public static void i(String tag, String msg) {
        android.util.Log.i(PRETAG + tag, msg);
    }

    public static void d(String tag, String msg) {
        android.util.Log.d(PRETAG + tag, msg);
    }

    public static void e(String tag, String msg) {
        android.util.Log.e(PRETAG + tag, msg);
    }

    public static void e(String tag, String msg, Exception e) {
        android.util.Log.e(PRETAG + tag, msg, e);
    }

    public static void w(String tag, String msg) {
        android.util.Log.w(PRETAG + tag, msg);
    }

    public static void i(String tag, String s, Exception e) {
        android.util.Log.i(tag, s, e);
    }
}
