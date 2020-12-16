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

package com.amazon.alexaautoclientservice.aacs_extra;

import android.content.Context;

import org.json.JSONObject;

public interface AACSContext {
    String CERTS_DIR = "certsDir";
    String MODEL_DIR = "modelsDir";
    String PRODUCT_DSN = "productDsn";
    String APPDATA_DIR = "appDataDir";
    String JSON = "json";
    Context getContext();
    String getData(String key);
    boolean isPlatformInterfaceEnabled(String name);
    void deregisterAASBPlatformInterface(String module, String name);
    JSONObject getConfiguration(String configKey);
}
