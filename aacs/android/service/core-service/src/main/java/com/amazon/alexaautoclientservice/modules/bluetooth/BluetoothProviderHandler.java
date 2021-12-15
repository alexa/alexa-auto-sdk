/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.modules.bluetooth;

import android.content.Context;
import android.util.Log;

import com.amazon.aace.bluetooth.BluetoothProvider;
import com.amazon.aace.bluetooth.BluetoothServerSocket;
import com.amazon.aace.bluetooth.GATTServer;

import java.io.IOException;

public class BluetoothProviderHandler extends BluetoothProvider {
    private static final String TAG = BluetoothProviderHandler.class.getSimpleName();

    private final Context mContext;
    public BluetoothProviderHandler(Context context) {
        mContext = context;
    }

    // Not supporting GATT
    @Override
    public GATTServer createGATTServer() {
        return null;
    }

    @Override
    public BluetoothServerSocket listenUsingRfcomm(String name, String uuid) {
        try {
            return new BluetoothServerSocketHandler(mContext, name, uuid);
        } catch (IOException e) {
            Log.e(TAG, "failed to create BluetoothServerSocket: " + e.getMessage());
        }
        return null;
    }
}
