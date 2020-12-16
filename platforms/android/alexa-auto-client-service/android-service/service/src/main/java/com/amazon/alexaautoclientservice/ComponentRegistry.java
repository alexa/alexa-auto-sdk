/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ComponentRegistry {
    private static final String TAG = AACSConstants.AACS + "-" + ComponentRegistry.class.getSimpleName();

    private static final ComponentRegistry COMPONENT_REGISTRY_INSTANCE = new ComponentRegistry();
    private static final int PACKAGE_MANAGER_QUERY_FLAGS = 0;

    private Map<String, List<TargetComponent>> mTargetCache;

    public static ComponentRegistry getInstance() {
        return COMPONENT_REGISTRY_INSTANCE;
    }

    private ComponentRegistry() {
        mTargetCache = new HashMap<>();
    }

    List<TargetComponent> findTarget(Context context, String topic, String action) {
        List<TargetComponent> targets;

        // Query config
        targets = queryConfig(context, topic);
        if (targets != null) {
            return targets;
        }

        String cacheKey = topic + "+" + action;

        // Query mTargetCache
        Log.v(TAG, String.format("No target specified in config for topic=%s, checking mTargetCache", topic));
        targets = mTargetCache.get(cacheKey);
        if (targets != null) {
            Log.i(TAG,
                    String.format("Found targets=%s for topic=%s action=%s in mTargetcache", targets, topic, action));
            return targets;
        }

        // Query package manager
        Log.v(TAG,
                String.format("No targets found in mTargetCache for topic=%s action=%s, querying package manager",
                        topic, action));
        targets = queryPackageManager(context, topic, action);
        if (targets == null) {
            Log.e(TAG, String.format("No targets found for topic=%s action=%s ", topic, action));
            return null;
        }

        Log.i(TAG, String.format("Caching topic=%s, action=%s with targets=%s", topic, action, targets));
        mTargetCache.put(cacheKey, targets);
        return targets;
    }

    private List<TargetComponent> queryConfig(Context context, String topic) {
        JSONArray packageNames = FileUtil.getIntentTargets(context, topic, "package");
        JSONArray classNames = FileUtil.getIntentTargets(context, topic, "class");
        JSONArray types = FileUtil.getIntentTargets(context, topic, "type");

        if (packageNames == null) {
            Log.e(TAG, String.format("Target package names for topic=%s is null", topic));
            return null;
        }

        if (classNames == null) {
            Log.e(TAG, String.format("Target class names for topic=%s is null", topic));
            return null;
        }

        if (types == null) {
            Log.e(TAG, String.format("Target types for topic=%s is null", topic));
            return null;
        }

        if (!(packageNames.length() == classNames.length() && packageNames.length() == types.length())) {
            Log.e(TAG,
                    String.format("Sizes of lists packageNames, classNames, types in config do not match for topic=%s",
                            topic));
            return null;
        }

        int size = packageNames.length();
        List<TargetComponent> targets = new ArrayList<>();

        for (int i = 0; i < size; i++) {
            TargetComponent target;
            try {
                String className = classNames.getString(i);
                if (className.charAt(0) == '.') {
                    className = packageNames.getString(i) + className;
                }
                target = TargetComponent.withComponent(
                        new ComponentName(packageNames.getString(i), className), getTargetType(types.getString(i)));
            } catch (JSONException ex) {
                Log.e(TAG, "Error reading JSONArray's from config");
                break;
            }
            if (checkPermission(context, AACSConstants.AACS_PERMISSION, target)) {
                Log.v(TAG,
                        String.format(
                                "Target for topic=%s found in config: %s", topic, target.component.getClassName()));
                targets.add(target);
            }
        }

        if (targets.size() > 0) {
            return targets;
        } else {
            Log.w(TAG,
                    String.format(
                            "None of the specified targets in config for topic=%s have the AACS permission", topic));
            return null;
        }
    }

    private boolean checkPermission(Context context, String permission, TargetComponent target) {
        if (context == null) {
            Log.e(TAG, "Service context unavailable to check target permission.");
            return false;
        }

        if (permission == null) {
            Log.e(TAG, "Null permission.");
            return false;
        }

        return context.getPackageManager().checkPermission(permission, target.packageName)
                == PackageManager.PERMISSION_GRANTED;
    }

    private List<TargetComponent> queryPackageManager(Context context, String topic, String action) {
        if (context == null) {
            Log.e(TAG, "Service context unavailable to query package manager.");
            return null;
        }

        final Intent queryIntent = new Intent(IPCConstants.AASB_INTENT_PREFIX + action);
        queryIntent.addCategory(IPCConstants.AASB_INTENT_PREFIX + topic);

        List<TargetComponent> targets = new ArrayList<>();

        // Query broadcast receivers
        queryReceivers(context, queryIntent, targets, topic, action);

        // Query activities
        queryActivities(context, queryIntent, targets, topic, action);

        // Query services
        queryServices(context, queryIntent, targets, topic, action);

        if (targets.size() == 0) {
            Log.e(TAG, String.format("No targets found by package manager for topic=%s action=%s", topic, action));
            return null;
        }

        return targets;
    }

    private void queryReceivers(
            Context context, Intent queryIntent, List<TargetComponent> targets, String topic, String action) {
        List<ResolveInfo> receivers =
                context.getPackageManager().queryBroadcastReceivers(queryIntent, PACKAGE_MANAGER_QUERY_FLAGS);
        filterByPermission(context, receivers, targets, TargetComponent.Type.RECEIVER, topic, action);
    }

    private void queryActivities(
            Context context, Intent queryIntent, List<TargetComponent> targets, String topic, String action) {
        List<ResolveInfo> receivers =
                context.getPackageManager().queryIntentActivities(queryIntent, PACKAGE_MANAGER_QUERY_FLAGS);
        filterByPermission(context, receivers, targets, TargetComponent.Type.ACTIVITY, topic, action);
    }

    private void queryServices(
            Context context, Intent queryIntent, List<TargetComponent> targets, String topic, String action) {
        List<ResolveInfo> receivers =
                context.getPackageManager().queryIntentServices(queryIntent, PACKAGE_MANAGER_QUERY_FLAGS);
        filterByPermission(context, receivers, targets, TargetComponent.Type.SERVICE, topic, action);
    }

    private void filterByPermission(Context context, List<ResolveInfo> infos, List<TargetComponent> targets,
            TargetComponent.Type type, String topic, String action) {
        if (infos.size() > 0) {
            for (ResolveInfo info : infos) {
                TargetComponent target = null;
                if (type == TargetComponent.Type.ACTIVITY || type == TargetComponent.Type.RECEIVER) {
                    target = TargetComponent.withComponent(
                            new ComponentName(info.activityInfo.packageName, info.activityInfo.name), type);
                } else if (type == TargetComponent.Type.SERVICE) {
                    target = TargetComponent.withComponent(
                            new ComponentName(info.serviceInfo.packageName, info.serviceInfo.name), type);
                }
                if (checkPermission(context, AACSConstants.AACS_PERMISSION, target)) {
                    if (target != null) {
                        Log.v(TAG,
                                String.format("Target in type=%s for topic=%s action=%s found by package manager: %s",
                                        type, topic, action, target.component.getClassName()));
                        targets.add(target);
                    }
                }
            }
        } else {
            Log.i(TAG, String.format("Targets in type=%s for topic=%s not found by package manager", type, topic));
        }
    }

    public static TargetComponent.Type getTargetType(String type) {
        switch (type) {
            case "ACTIVITY":
                return TargetComponent.Type.ACTIVITY;
            case "RECEIVER":
                return TargetComponent.Type.RECEIVER;
            case "SERVICE":
                return TargetComponent.Type.SERVICE;
            default:
                Log.w(TAG, String.format("Unknown target type: %s", type));
                return TargetComponent.Type.UNKNOWN;
        }
    }

    public void cleanUp() {
        if (mTargetCache != null) {
            mTargetCache.clear();
            Log.v(TAG, "Target Cache in Component Registry is cleaned");
        }
    }
}