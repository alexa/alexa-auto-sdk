/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.apps.common.util

import androidx.core.os.LocaleListCompat

/**
 * Class for Locale related utilities
 */
object LocaleUtil {
    private val TAG = LocaleUtil::class.java.simpleName

    fun parseAlexaLocaleStringToAndroidLocaleList(alexaLocaleString: String): LocaleListCompat {
        return LocaleListCompat.forLanguageTags(
            alexaLocaleString.split("/".toRegex())[0]
        )
    }

    fun parseAndroidLocaleListToAlexaLocaleString(androidLocaleList: LocaleListCompat): String {
        return androidLocaleList[0].toString().replace("_", "-")
    }
}
