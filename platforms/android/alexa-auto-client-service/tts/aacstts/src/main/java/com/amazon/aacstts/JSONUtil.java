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

package com.amazon.aacstts;

/**
 * Utility Class to handle the commonly used JSON functions.
 */
public class JSONUtil {
    /**
     * Escapes the json to a escaped JSON String.
     * @param jsString - the JSON String that needs to be escaped
     * @return the escaped json string.
     */
    public static String escapeString(String jsString) {
        jsString = jsString.replace("\\", "\\\\");
        jsString = jsString.replace("\"", "\\\"");
        jsString = jsString.replace("\b", "\\b");
        jsString = jsString.replace("\f", "\\f");
        jsString = jsString.replace("\n", "\\n");
        jsString = jsString.replace("\r", "\\r");
        jsString = jsString.replace("\t", "\\t");
        jsString = jsString.replace("/", "\\/");
        return jsString;
    }
}
