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
package com.amazon.alexaautoclientservice.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class BroadcastReceiverScanner {
    private static final String TAG = "BroadcastReceiverScanner";
    private static final String PACKAGE = "package";
    private static final String CONTEXT_BROADCAST_RECEIVERS = "ContextBroadcastReceivers";
    private static final String GET_RECEIVERS_BUILDER = "getReceiversBuilder";
    private static final String GET_INSTANCE = "getInstance";
    private static final String GET_CATEGORIES = "getCategories";
    private static final String GET_ACTIONS = "getActions";
    private static final String GET_PERMISSION = "getPermission";
    Map<BroadcastReceiver, Pair<IntentFilter, String>> mBroadcastReceiverMap;

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
                    Log.d(TAG, key + " is a key in the bundle");
                }
                return result;
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, e.getMessage());
            e.printStackTrace();
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

    /**
     * This method provides list of all the context broadcast receivers in the alexa application.
     * This list is generated using aacs annotations. If any class must appear in the map, it should
     * use the class level annotation @ContextBroadcastReceiver and provide list of categories, actions
     * and permissions. This class must extend the {@link android.content.BroadcastReceiver} provide the
     * singleton object using getInstance() method.
     * AndroidManifest.xml file should have a meta-data tag in the application.
     * e.g.
     * <pre>{@code
     * <meta-data android:name="<Package containing the receiver class>" android:value="package"/>
     * }
     * </pre>
     *
     * @param context
     * @return Map if {@link android.content.BroadcastReceiver} and pair of IntentFilter and optional permission
     */
    public Map<BroadcastReceiver, Pair<IntentFilter, String>> getBroadcastReceivers(Context context) {
        if (mBroadcastReceiverMap == null) {
            mBroadcastReceiverMap = new HashMap<>();
            Set<String> generatedClasses;
            generatedClasses = scan(context);

            for (String generatedClass : generatedClasses) {
                try {
                    Class<?> ContextBroadcastReceivers =
                            Class.forName(generatedClass + "." + CONTEXT_BROADCAST_RECEIVERS);
                    Method method = ContextBroadcastReceivers.getMethod(GET_RECEIVERS_BUILDER);
                    Set<String> broadcastReceivers = (Set<String>) method.invoke(null);
                    if (broadcastReceivers == null || broadcastReceivers.isEmpty()) {
                        Log.w(TAG, "BroadcastReceivers Not Found in " + generatedClass);
                        continue;
                    }
                    for (String broadcastReceiver : broadcastReceivers) {
                        Log.d(TAG, broadcastReceiver);
                        Class<?> bc = Class.forName(broadcastReceiver);
                        Method getInstance = bc.getMethod(GET_INSTANCE);
                        BroadcastReceiver receiver = (BroadcastReceiver) getInstance.invoke(null);

                        IntentFilter intentFilter = new IntentFilter();
                        String permission;
                        Method getCategories = ContextBroadcastReceivers.getMethod(GET_CATEGORIES, String.class);
                        String[] categories = (String[]) getCategories.invoke(null, broadcastReceiver);

                        Method getActions = ContextBroadcastReceivers.getMethod(GET_ACTIONS, String.class);
                        String[] actions = (String[]) getActions.invoke(null, broadcastReceiver);

                        Object objPermission = null;
                        if (containsMethod(ContextBroadcastReceivers.getMethods(), GET_PERMISSION)) {
                            Method getPermission = ContextBroadcastReceivers.getMethod(GET_PERMISSION);
                            objPermission = getPermission.invoke(null);
                        }

                        if (categories != null) {
                            for (String cat : categories) {
                                intentFilter.addCategory(cat);
                            }
                        }

                        if (actions != null) {
                            for (String action : actions) {
                                intentFilter.addAction(action);
                            }
                        }

                        if (objPermission != null) {
                            permission = (String) objPermission;
                        } else {
                            permission = null;
                        }
                        mBroadcastReceiverMap.put(receiver, new Pair<>(intentFilter, permission));
                    }
                } catch (ClassNotFoundException exe) {
                    // Ignore, possible that class is not present
                } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
                    Log.e(TAG, e.getMessage());
                    e.printStackTrace();
                }
            }
        }
        return mBroadcastReceiverMap;
    }
}
