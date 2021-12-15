package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.NonNull;

/**
 * Provider class for Alexa Auto extra modules.
 */
public class ModuleProvider {
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
}
