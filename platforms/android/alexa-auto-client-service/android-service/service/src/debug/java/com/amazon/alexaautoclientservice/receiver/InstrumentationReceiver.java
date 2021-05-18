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

package com.amazon.alexaautoclientservice.receiver;

import static java.lang.System.currentTimeMillis;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.constants.MessageLoggerConstants;

import java.io.FileWriter;
import java.io.IOException;

public class InstrumentationReceiver extends BroadcastReceiver {
    private final String TAG = AACSConstants.AACS + "-" + InstrumentationReceiver.class.getSimpleName();
    public static boolean mIsLogEnabled;
    public static String mFileLocation;

    public InstrumentationReceiver() {
        super();
        mIsLogEnabled = false;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "Instrumentation Intent Received: " + intent.toString());
        Long currentTimeMill = currentTimeMillis();
        if (MessageLoggerConstants.START_INSTRUMENTATION.equals(intent.getAction())) {
            mIsLogEnabled = true;
            if (intent.hasExtra(MessageLoggerConstants.LOG_FILE_LOCATION)) {
                Log.i(TAG, "FileLocation: " + intent.getStringExtra(MessageLoggerConstants.LOG_FILE_LOCATION));
                mFileLocation = intent.getStringExtra(MessageLoggerConstants.LOG_FILE_LOCATION);
            } else {
                mFileLocation = "/sdcard/" + currentTimeMill.toString() + ".log";
            }
            try {
                FileWriter logFile = new FileWriter(mFileLocation, true);
                logFile.write("[");
                logFile.close();
            } catch (IOException e) {
                Log.e(TAG, "Error Writing Log entry to file: ", e);
            }
        } else if (MessageLoggerConstants.STOP_INSTRUMENTATION.equals(intent.getAction())) {
            try {
                FileWriter logFile = new FileWriter(mFileLocation, true);
                logFile.write("{}]");
                logFile.close();
            } catch (IOException e) {
                Log.e(TAG, "Error Writing Log entry to file: ", e);
            }
            mIsLogEnabled = false;
        }
    }
}
