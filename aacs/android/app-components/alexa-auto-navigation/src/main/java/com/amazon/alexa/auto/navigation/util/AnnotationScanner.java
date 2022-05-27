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
package com.amazon.alexa.auto.navigation.util;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

import com.amazon.alexa.auto.navigation.providers.NaviProvider;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.Set;

public class AnnotationScanner {
    private static final String TAG = "AnnotationScanner";
    private static final String PACKAGE = "package";
    private static final String NAVI_PROVIDER_MODULE = "NaviProviderModules";
    private static final String GET_NAVI_PROVIDER_MODULES = "getNaviProviderModules";
    private static final String GET_INSTANCE = "getInstance";

    private NaviProvider mNaviProvider;

    private Set<String> scan(Context context) {
        try {
            ApplicationInfo info = context.getPackageManager().getApplicationInfo(
                    context.getPackageName(), PackageManager.GET_META_DATA);
            if (info.metaData != null) {
                Bundle bundle = info.metaData;
                Set<String> result = new HashSet<>();
                for (String key : bundle.keySet()) {
                    if (PACKAGE.equals(bundle.get(key)))
                        result.add(key);
                }
                return result;
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, e.getMessage());
        }
        return new HashSet<>();
    }

    private boolean containsMethod(Method[] methods, String methodName) {
        for (Method m : methods) {
            if (m != null && methodName != null && methodName.equals(m.getName())) {
                return true;
            }
        }
        return false;
    }

    public NaviProvider getNaviProvider(Context context) {
        if (mNaviProvider == null) {
            Set<String> generatedClasses = scan(context);
            for (String generatedClass : generatedClasses) {
                try {
                    Class<?> NaviProviderModules = Class.forName(generatedClass + "." + NAVI_PROVIDER_MODULE);
                    if (!containsMethod(NaviProviderModules.getMethods(), GET_NAVI_PROVIDER_MODULES)) {
                        continue;
                    }
                    Method method = NaviProviderModules.getMethod(GET_NAVI_PROVIDER_MODULES);
                    Set<String> naviProviderModules = (Set<String>) method.invoke(null);
                    if (naviProviderModules == null || naviProviderModules.isEmpty()) {
                        continue;
                    }
                    for (String naviProviderModule : naviProviderModules) {
                        Class<?> bc = Class.forName(naviProviderModule);
                        Method getInstance = bc.getMethod(GET_INSTANCE);
                        Object object = getInstance.invoke(null);
                        if (object instanceof NaviProvider) {
                            mNaviProvider = (NaviProvider) object;
                            break;
                        }
                    }
                } catch (ClassNotFoundException exe) {
                    // Ignore, possible that class is not present
                } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
                    Log.e(TAG, e.getMessage());
                }
            }
        }
        return mNaviProvider;
    }
}
