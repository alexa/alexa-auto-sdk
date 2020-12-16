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

package com.amazon.alexaautoclientservice.util;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;

import org.json.JSONObject;

import java.util.UUID;

public class AASBUtil {
    private static final String TAG = AACSConstants.AACS + "-" + AASBUtil.class.getSimpleName();

    public static String constructAASBMessage(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, @NonNull String payload) {
        String uniqueID = UUID.randomUUID().toString();
        try {
            String aasbMessage = "";
            if (replyToId.isEmpty()) {
                aasbMessage = "{\n"
                        + "  \"header\" : {\n"
                        + "    \"version\" : \"3.1\",\n"
                        + "    \"messageType\" : \"Publish\",\n"
                        + "    \"id\" : \"" + uniqueID + "\",\n"
                        + "    \"messageDescription\" : {\n"
                        + "      \"topic\" : \"" + topic + "\",\n"
                        + "      \"action\" : \"" + action + "\"\n"
                        + "    }\n"
                        + "  }\n"
                        + "}";
            } else {
                aasbMessage = "{\n"
                        + "  \"header\" : {\n"
                        + "    \"version\" : \"3.1\",\n"
                        + "    \"messageType\" : \"Reply\",\n"
                        + "    \"id\" : \"" + uniqueID + "\",\n"
                        + "    \"messageDescription\" : {\n"
                        + "      \"topic\" : \"" + topic + "\",\n"
                        + "      \"action\" : \"" + action + "\",\n"
                        + "      \"replyToId\" : \"" + replyToId + "\"\n"
                        + "    }\n"
                        + "  }\n"
                        + "}";
            }
            JSONObject msgObj = new JSONObject(aasbMessage);
            if (!payload.isEmpty()) {
                JSONObject payloadJson = new JSONObject(payload);
                msgObj.put("payload", payloadJson);
            } else {
                msgObj.put("payload", null);
            }
            return msgObj.toString();
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct AASB message");
            return "";
        }
    }

    public static String removePackageNameFromString(String input) {
        String result;
        if (input != null && input.contains(AACSConstants.AACS_PACKAGE_NAME + ".")) {
            result = input.replace(AACSConstants.AACS_PACKAGE_NAME + ".", "");
        } else {
            result = input;
        }

        return result;
    }
}
