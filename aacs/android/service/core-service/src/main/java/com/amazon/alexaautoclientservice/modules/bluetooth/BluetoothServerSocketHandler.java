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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothServerSocket;
import android.content.Context;
import android.util.Log;

import java.io.IOException;
import java.util.UUID;

public class BluetoothServerSocketHandler extends com.amazon.aace.bluetooth.BluetoothServerSocket {
    private static final String TAG = BluetoothServerSocketHandler.class.getSimpleName();

    private final BluetoothServerSocket mServerSocket;

    BluetoothServerSocketHandler(Context context, String name, String uuid) throws IOException {
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();

        Log.i(TAG, "listenUsingRfcommWithServiceRecord: name=" + name + ", uuid=" + uuid);
        mServerSocket = bluetoothAdapter.listenUsingRfcommWithServiceRecord(name, UUID.fromString(uuid));
    }

    @Override
    public com.amazon.aace.bluetooth.BluetoothSocket accept() {
        try {
            Log.i(TAG, "Accepting socket");
            return new BluetoothSocketHandler(mServerSocket.accept());
        } catch (IOException e) {
            Log.e(TAG, "failed to accept incoming connection: " + e.getMessage());
        }
        return null;
    }

    @Override
    public void close() {
        try {
            Log.i(TAG, "Closing socket");
            mServerSocket.close();
        } catch (IOException e) {
            Log.e(TAG, "failed to accept incoming connection: " + e.getMessage());
        }
    }
}
