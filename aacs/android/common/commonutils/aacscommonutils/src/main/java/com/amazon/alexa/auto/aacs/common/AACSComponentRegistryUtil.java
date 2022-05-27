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
package com.amazon.alexa.auto.aacs.common;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.TargetComponent;

import java.util.ArrayList;
import java.util.List;

/**
 * Util class to look up targets for a specific intent using Package Manager and filter out the ones that have
 * a specific permission.
 */
public class AACSComponentRegistryUtil {
    private static final String TAG = AACSConstants.AACS + "-" + AACSComponentRegistryUtil.class.getSimpleName();
    private static final int PACKAGE_MANAGER_QUERY_FLAGS = 0;

    /**
     * Query targets with package manager.
     * @param context A context for getting access to package manager.
     * @param intent The intent to be queried about.
     * @param permission The permission the targets must have to receive the intent.
     * @return List of TargetComponent.
     */
    public static List<TargetComponent> queryPackageManager(
            @NonNull Context context, @NonNull Intent intent, @NonNull String permission) {
        List<TargetComponent> targets = new ArrayList<>();

        // Query broadcast receivers
        List<ResolveInfo> receivers =
                context.getPackageManager().queryBroadcastReceivers(intent, PACKAGE_MANAGER_QUERY_FLAGS);
        if (receivers.size() > 0)
            filterByPermission(context, intent, targets, receivers, TargetComponent.Type.RECEIVER, permission);

        // Query activities
        List<ResolveInfo> activities =
                context.getPackageManager().queryIntentActivities(intent, PACKAGE_MANAGER_QUERY_FLAGS);
        if (activities.size() > 0)
            filterByPermission(context, intent, targets, activities, TargetComponent.Type.ACTIVITY, permission);

        // Query services
        List<ResolveInfo> services =
                context.getPackageManager().queryIntentServices(intent, PACKAGE_MANAGER_QUERY_FLAGS);
        if (services.size() > 0)
            filterByPermission(context, intent, targets, services, TargetComponent.Type.SERVICE, permission);

        if (targets.size() == 0) {
            Log.e(TAG,
                    String.format("No targets found by package manager for intent action=%s category=%s",
                            intent.getAction(), intent.getCategories()));
            return null;
        }

        return targets;
    }

    private static void filterByPermission(Context context, Intent intent, List<TargetComponent> targets,
            List<ResolveInfo> targetInfos, TargetComponent.Type type, String permission) {
        for (ResolveInfo info : targetInfos) {
            TargetComponent target = null;
            if (type == TargetComponent.Type.ACTIVITY || type == TargetComponent.Type.RECEIVER) {
                target = TargetComponent.withComponent(
                        new ComponentName(info.activityInfo.packageName, info.activityInfo.name), type);
            } else if (type == TargetComponent.Type.SERVICE) {
                target = TargetComponent.withComponent(
                        new ComponentName(info.serviceInfo.packageName, info.serviceInfo.name), type);
            }
            if (checkPermission(context, permission, target)) {
                if (target != null) {
                    Log.v(TAG,
                            String.format(
                                    "Target in type=%s for intent action=%s category=%s found by package manager: %s",
                                    type, intent.getAction(), intent.getCategories(), target.component.getClassName()));
                    targets.add(target);
                }
            }
        }
    }

    /**
     * Check if the given target has the permission.
     * @param context A context for getting access to package manager.
     * @param permission The permission to be checked if the target has it.
     * @param target The target to be checked.
     * @return true if the target has the permission, false otherwise.
     */
    public static boolean checkPermission(Context context, String permission, TargetComponent target) {
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
}
