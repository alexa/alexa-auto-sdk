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

import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;

public class BluetoothSocketHandler extends com.amazon.aace.bluetooth.BluetoothSocket {
    private static final String TAG = BluetoothSocketHandler.class.getSimpleName();

    private final BluetoothSocket mSocket;

    BluetoothSocketHandler(BluetoothSocket socket) {
        mSocket = socket;
    }

    @Override
    public int read(byte[] data, int off, int len) {
        try {
            return mSocket.getInputStream().read(data, off, len);
        } catch (IOException e) {
            Log.e(TAG, "failed to read: " + e.getMessage());
        }
        return -1;
    }

    @Override
    public void write(byte[] data, int off, int len) {
        try {
            mSocket.getOutputStream().write(data, off, len);
        } catch (IOException e) {
            Log.e(TAG, "failed to write:" + e.getMessage());
        }
    }

    @Override
    public void close() {
        try {
            mSocket.close();
        } catch (IOException e) {
            Log.e(TAG, "failed to close: " + e.getMessage());
        }
    }
}
