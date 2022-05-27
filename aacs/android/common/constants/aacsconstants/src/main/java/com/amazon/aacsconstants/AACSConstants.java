/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aacsconstants;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.util.Log;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

public class AACSConstants {
    private static final String TAG = AACSConstants.class.getSimpleName();
    /**
     * @deprecated
     * Recommended to use {@link #getAACSPackageName(WeakReference)}  getAACSPackageName}
     */
    public static final String AACS_PACKAGE_NAME = "com.amazon.alexaautoclientservice";
    public static final String AACS_CLASS_NAME = "com.amazon.alexaautoclientservice.AlexaAutoClientService";
    public static String AACS = "AACS";
    private static String _AACS_PACKAGE_NAME = null; // This should remain private non final

    public static final String NEW_CONFIG = "newConfig";
    public static final String CONFIG_MODULE = "configModule";
    public static final String PAYLOAD = "payload";
    public static final String AACS_GENERAL_CONFIG = "aacs.general";
    public static final String AACS_AASB_CONFIG = "aacs.aasb";
    public static final String AACS_AMAZONLITE_CONFIG = "aacs.amazonLite";
    public static final String AACS_COASSISTANT = "aacs.coassistant";

    public static final String AACS_PROPERTY_URI = "com.amazon.alexaautoclientservice.AACSPropertyContentProvider";
    public static final String[] ALEXA_PROPERTIES = {AACSPropertyConstants.WAKEWORD_SUPPORTED,
            AACSPropertyConstants.FIRMWARE_VERSION, AACSPropertyConstants.LOCALE,
            AACSPropertyConstants.COUNTRY_SUPPORTED, AACSPropertyConstants.TIMEZONE,
            AACSPropertyConstants.WAKEWORD_ENABLED, AACSPropertyConstants.OPERATING_COUNTRY,
            AACSPropertyConstants.VERSION, AACSPropertyConstants.NETWORK_INTERFACE};

    public static final String REPLY_TO_PACKAGE = "replyToPackage";
    public static final String REPLY_TO_CLASS = "replyToClass";
    public static final String REPLY_TYPE = "replyType";
    public static final String REPLY_TYPE_RECEIVER = "RECEIVER";

    public static class IntentAction {
        public static final String GET_SERVICE_METADATA = "GetServiceMetadata";
        public static final String GET_SERVICE_METADATA_REPLY = "GetServiceMetadataReply";
        public static final String ENABLE_SYNC_SYSTEM_PROPERTY_CHANGE =
                "com.amazon.aacs.syncSystemPropertyChange.enable";
        public static final String DISABLE_SYNC_SYSTEM_PROPERTY_CHANGE =
                "com.amazon.aacs.syncSystemPropertyChange.disable";
    }

    public static class IntentCategory {
        public static final String GET_SERVICE_METADATA = "GetServiceMetadataTopic";
        public static final String SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT = "com.amazon.aacs.syncSystemPropertyChange";
    }

    public static class ServiceMetadata {
        public static final String EXTRA_MODULE_LIST = "extrasModuleList";
        public static final String METADATA = "metaData";
    }

    // AACS State
    public enum State { STARTED, WAIT_FOR_LVC_CONFIG, CONFIGURED, ENGINE_INITIALIZED, STOPPED }
    public static final String ACTION_STATE_CHANGE = "com.amazon.aacs.service.statechanged";

    // AACS Permission
    public static final String AACS_PERMISSION = "com.amazon.alexaautoclientservice";
    public static final String AACS_PING_PERMISSION = "com.amazon.alexaautoclientservice.ping";
    public static final String AACS_SERVICE_METADATA_PERMISSION =
            "com.amazon.alexaautoclientservice.getservicemetadata";

    /**
     * This method returns the package name of AACS Service dynamically.
     * If AACS Service is running in a separate application, it returns value of {@link #AACS_PACKAGE_NAME}
     * AACS_PACKAGE_NAME. Otherwise if AACS Service is included in the client app as an AAR, it returns the client app
     * package name.
     * @param contextWk Weak reference of Android context for getting a package manager
     * @return AACS Service package name
     */
    public static final String getAACSPackageName(@NonNull WeakReference<Context> contextWk) {
        if (_AACS_PACKAGE_NAME == null) {
            PackageManager packageManager = contextWk.get().getPackageManager();
            try {
                _AACS_PACKAGE_NAME = AACS_PACKAGE_NAME;
                PackageInfo packageInfo =
                        packageManager.getPackageInfo(contextWk.get().getPackageName(), PackageManager.GET_SERVICES);
                for (ServiceInfo serviceInfo : packageInfo.services) {
                    if (serviceInfo.name.equals(AACS_CLASS_NAME)) {
                        _AACS_PACKAGE_NAME = contextWk.get().getPackageName();
                        Log.d(TAG, String.format("Setting PACKAGE_NAME %s", _AACS_PACKAGE_NAME));
                        break;
                    }
                }
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
                return null;
            }
        }
        return _AACS_PACKAGE_NAME;
    }
}
