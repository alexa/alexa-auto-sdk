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

package com.amazon.aacsipc;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import androidx.annotation.NonNull;

import java.lang.reflect.Method;

public class IPCUtils {
    private static final String TAG = IPCUtils.class.getSimpleName();

    private boolean isAppInSystemPartition;
    private boolean isPrivilegedApp;
    private boolean isSignedBySystem;
    private String packageName;

    private static IPCUtils INSTANCE;

    private IPCUtils(Context context) {
        packageName = context.getPackageName();
        isAppInSystemPartition = checkIsAppInSystemPartition(context.getPackageManager());
        isPrivilegedApp = checkIsPrivilegedApp(context.getPackageManager());
        isSignedBySystem = checkIsSignedBySystem(context.getPackageManager());
    }

    /**
     * Returns an instance of IPCUtils.
     * Checks if the INSTANCE is null and constructs an instance of IPCUtils.
     *
     * @param context Android Context
     */
    public static IPCUtils getInstance(@NonNull Context context) {
        if (INSTANCE == null) {
            INSTANCE = new IPCUtils(context);
        }
        return INSTANCE;
    }

    /**
     * Returns true if the app is installed as a system app.
     * Returns false if the app is installed as a user app.
     *
     * The app is a system app if it satisfies any one of the following conditions:
     * 1. checkIsAppInSystemPartition - Checks if the installation path is under /system.
     * 2. checkIsPrivilegedApp - Checks if the app is a privileged app.
     * 3. checkIsSignedBySystem - Checks if the app is signed by the same signature as that of android.
     *
     * @return boolean value of if its a system app or not.
     */
    public boolean isSystemApp() {
        boolean sysApp = isAppInSystemPartition || isPrivilegedApp || isSignedBySystem;
        Log.i(TAG, "isSystemApp: " + sysApp);
        return sysApp;
    }

    private boolean checkIsAppInSystemPartition(PackageManager packageManager) {
        try {
            ApplicationInfo applicationInfo = packageManager.getApplicationInfo(packageName, 0);

            return ((applicationInfo.flags & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP))
                    != 0);
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, e.getMessage());
            return false;
        }
    }

    private boolean checkIsPrivilegedApp(PackageManager packageManager) {
        try {
            ApplicationInfo applicationInfo = packageManager.getApplicationInfo(packageName, 0);
            @SuppressWarnings("JavaReflectionMemberAccess")
            @SuppressLint("BlockedPrivateApi")
            Method method = ApplicationInfo.class.getDeclaredMethod("isPrivilegedApp");

            return (Boolean) method.invoke(applicationInfo);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            return false;
        }
    }

    private boolean checkIsSignedBySystem(PackageManager packageManager) {
        try {
            @SuppressLint("PackageManagerGetSignatures")
            PackageInfo packageInfoApp = packageManager.getPackageInfo(packageName, PackageManager.GET_SIGNATURES);
            @SuppressLint("PackageManagerGetSignatures")
            PackageInfo packageInfoSys = packageManager.getPackageInfo("android", PackageManager.GET_SIGNATURES);

            return ((packageInfoApp != null) && (packageInfoSys != null)
                    && (packageInfoSys.signatures[0].equals(packageInfoApp.signatures[0])));
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, e.getMessage());
            return false;
        }
    }
}
