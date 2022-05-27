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

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.MainThread;
import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.module.ModuleInterface;

import org.json.JSONObject;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import io.reactivex.rxjava3.core.Single;

/**
 * Provider class for Alexa Auto extra modules.
 */
public class ModuleProvider {
    public static final String TAG = ModuleProvider.class.getSimpleName();
    public static final String MODULES = "modules";

    public enum ModuleName { PREVIEW_MODE, ALEXA_CUSTOM_ASSISTANT, GEOLOCATION, LVC }

    public static void addModule(@NonNull Context context, String module) {
        SharedPreferences.Editor editor = context.getSharedPreferences(MODULES, 0).edit();
        String modules = context.getSharedPreferences(MODULES, 0).getString(MODULES, "");
        Preconditions.checkNotNull(modules);

        if (modules.isEmpty()) {
            editor.putString(MODULES, module);
        } else if (!modules.contains(module)) {
            modules = modules + "," + module;
            editor.putString(MODULES, modules);
        }

        editor.commit();
    }

    public static void removeModule(@NonNull Context context, String module) {
        SharedPreferences.Editor editor = context.getSharedPreferences(MODULES, 0).edit();
        String modules = context.getSharedPreferences(MODULES, 0).getString(MODULES, "");
        Preconditions.checkNotNull(modules);

        if (modules.contains(module)) {
            modules = modules.replace(module, "");
            // Remove leading comma
            modules = modules.replaceAll("^,+", "");
            // Remove trailing comma
            modules = modules.replaceAll(",+$", "");
            editor.putString(MODULES, modules);
        }

        editor.commit();
    }

    public static String getModules(@NonNull Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(MODULES, 0);
        if (sharedPreferences != null) {
            return sharedPreferences.getString(MODULES, "");
        } else {
            return "";
        }
    }

    public static boolean isPreviewModeEnabled(@NonNull Context context) {
        String extraModules = getModules(context);
        return extraModules.contains(ModuleName.PREVIEW_MODE.name());
    }

    public static boolean isAlexaCustomAssistantEnabled(@NonNull Context context) {
        String extraModules = getModules(context);
        return extraModules.contains(ModuleName.ALEXA_CUSTOM_ASSISTANT.name());
    }

    public static boolean containsModule(@NonNull Context context, @NonNull ModuleName moduleName) {
        String extraModules = getModules(context);
        return extraModules.contains(moduleName.name());
    }

    public static Optional<List<ModuleInterface>> getModuleSync(@NonNull Context context) {
        Log.i(TAG, "getModuleSync: Start scanning moduleInterface from extensions..");
        List<ModuleInterface> modules = new ArrayList<>();
        try {
            String folderName = "aacs-sample-app";
            String moduleKey = "module";
            String category = "name";
            String[] fileList = context.getAssets().list(folderName);
            for (String f : fileList) {
                InputStream is = context.getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONObject moduleKeyObj = obj.optJSONObject(moduleKey);
                    if (moduleKeyObj == null) {
                        Log.w(TAG, "module key is missing");
                        continue;
                    }
                    String moduleName = moduleKeyObj.getString(category);
                    ModuleInterface instance = (ModuleInterface) Class.forName(moduleName).newInstance();
                    modules.add(instance);
                    Log.i(TAG, "getModuleSync: load extra module:" + moduleName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "getModule: " + e.getMessage());
            return Optional.empty();
        }
        return Optional.of(modules);
    }

    public static Single<Optional<List<ModuleInterface>>> getModuleAsync(
            @NonNull Context context, @NonNull Handler handler) {
        return Single.create(emitter -> {
            handler.post(() -> {
                Optional<List<ModuleInterface>> handlerModuleOptional = getModuleSync(context);
                new Handler(Looper.getMainLooper()).post(() -> emitter.onSuccess(handlerModuleOptional));
            });
        });
    }
}
