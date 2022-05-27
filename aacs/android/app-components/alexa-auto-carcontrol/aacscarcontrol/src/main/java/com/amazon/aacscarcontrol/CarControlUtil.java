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

package com.amazon.aacscarcontrol;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Objects;

public class CarControlUtil {
    public static final String TAG = CarControlUtil.class.getCanonicalName();

    public static CarControlConstants.DataType getDataType(String dataType) {
        switch (dataType) {
            case "int":
                return CarControlConstants.DataType.INT;
            case "boolean":
                return CarControlConstants.DataType.BOOLEAN;
            case "float":
                return CarControlConstants.DataType.FLOAT;
            default:
                return CarControlConstants.DataType.UNKNOWN;
        }
    }

    public static JSONObject readConfig(InputStream source) {
        JSONObject obj = null;
        InputStream inputStream = source;

        try {
            byte[] buffer = new byte[inputStream.available()];
            inputStream.read(buffer);
            String json = new String(buffer, "UTF-8");
            obj = new JSONObject(json);

            if (inputStream != null)
                inputStream.close();
        } catch (JSONException | IOException e) {
            Log.e(TAG, "Error while reading external Car Control Endpoint Configuration: " + e);
        }
        return obj;
    }

    public static void copyExternalFileToCarControl(@NonNull Context context, @NonNull Uri fileUri) {
        // Create copy file in externalFiles directory
        File externalFilesDir = new File(context.getFilesDir(), "externalFiles");
        if (!externalFilesDir.exists())
            externalFilesDir.mkdir();

        File externalFileCopy = new File(externalFilesDir, CarControlConstants.AACS_CC_ConfigMapping);
        if (externalFileCopy.exists())
            externalFileCopy.delete();

        if (!externalFilesDir.exists() || externalFileCopy.exists()) {
            Log.e(TAG, "Error while setting up externalFiles directory.");
            return;
        }

        try {
            if (!externalFileCopy.createNewFile()) {
                Log.e(TAG, "Failed to create new file.");
                return;
            }
        } catch (IOException e) {
            Log.e(TAG, String.format("Error while creating new file. Error: %s", e.getMessage()));
        }

        // Copy contents of file descriptor to copy file
        try {
            FileDescriptor fd = Objects.requireNonNull(context.getContentResolver().openFileDescriptor(fileUri, "r"))
                                        .getFileDescriptor();
            InputStream inputStream = new FileInputStream(fd);
            OutputStream outputStream = new FileOutputStream(externalFileCopy);
            byte[] buf = new byte[1024];
            int len;
            while ((len = inputStream.read(buf)) > 0) {
                outputStream.write(buf, 0, len);
            }
        } catch (FileNotFoundException e) {
            Log.w(TAG,
                    String.format("File associated with URI (%s) provided not found. Error: %s", fileUri.toString(),
                            e.getMessage()));
        } catch (IOException e) {
            Log.e(TAG,
                    String.format("Error while copying files locally for URI: %s. Error: %s", fileUri.toString(),
                            e.getMessage()));
        } catch (SecurityException e) {
            Log.e(TAG,
                    String.format("AACS does not have permissions to access URI (%s). Error: %s", fileUri.toString(),
                            e.getMessage()));
        }
    }

    public static float celcius(float f) {
        return (float) ((f - 32) / 1.8);
    }
}
