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
package com.amazon.alexa.auto.settings

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.res.Resources
import android.util.Log
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import com.amazon.aacsconstants.AACSConstants
import com.amazon.aacsconstants.AACSPropertyConstants
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController
import com.amazon.alexa.auto.apps.common.util.LocaleUtil
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager
import com.amazon.alexa.auto.settings.config.PopupKeys
import com.amazon.alexa.auto.settings.dependencies.AndroidModule
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent
import java.lang.ref.WeakReference
import java.util.*
import javax.inject.Inject

/**
 * AACSLocaleReceiver receives and handles Android locale changed intent. When system locale is changed, we want to
 * check if
 * 1. system locale is supported by Alexa, Alexa locale will be set to new system locale, and persist Alexa locale with
 * new system locale.
 * 2. system locale is not supported by Alexa. If user is signed in, we display language mismatch popup screen
 * and let user select one Alexa supported locale if he wants to. If user is not signed in, we update application's
 * locale configuration and reset Alexa locale.
 */
class AACSLocaleReceiver : BroadcastReceiver() {
    @set:Inject
    var mAlexaPropertyManager: AlexaPropertyManager? = null

    @set:Inject
    var mLocalesProvider: AlexaLocalesProvider? = null

    @SuppressLint("CheckResult")
    override fun onReceive(context: Context, intent: Intent) {
        DaggerSettingsComponent.builder().androidModule(AndroidModule(context)).build().inject(this)
        Log.d(TAG, intent.action!!)
        when (intent.action!!) {
            Intent.ACTION_LOCALE_CHANGED -> {
                mAlexaPropertyManager!!.getAlexaProperty(AACSPropertyConstants.LOCALE)
                    .subscribe { alexaLocale: Optional<String?> ->
                        val currentSystemLocale =
                            LocaleUtil.parseAndroidLocaleListToAlexaLocaleString(
                                LocaleListCompat.create(
                                    Resources.getSystem().configuration.locales[0]
                                )
                            )
                        mLocalesProvider!!.isCurrentLocaleSupportedByAlexa(currentSystemLocale)
                            .subscribe { localeSupported: Boolean ->
                                val isSignedIn: Boolean =
                                    AlexaApp.from(context).rootComponent.authController.isAuthenticated
                                if (isSignedIn) {
                                    if (localeSupported) {
                                        Log.d(
                                            TAG,
                                            "Locale is supported by Alexa, don't popup Alexa language selection screen."
                                        )
                                        mAlexaPropertyManager!!.updateAlexaProperty(
                                            AACSPropertyConstants.LOCALE,
                                            currentSystemLocale
                                        )
                                            .subscribe { successStatus: Boolean? ->
                                                if (!successStatus!!) {
                                                    Log.e(
                                                        TAG,
                                                        "Failed to update property manager to locale: $currentSystemLocale"
                                                    )
                                                } else {
                                                    enableAACSSystemLocaleSync(context)
                                                    AppCompatDelegate.setApplicationLocales(
                                                        LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                                                            currentSystemLocale
                                                        )
                                                    )
                                                }
                                            }
                                    } else {
                                        Log.d(
                                            TAG,
                                            "Locale is not supported by Alexa, popup Alexa language selection screen if logged in."
                                        )
                                        startPopup(context, currentSystemLocale, alexaLocale.get())
                                    }
                                } else {
                                    if (localeSupported) {
                                        enableAACSSystemLocaleSync(context)
                                        AppCompatDelegate.setApplicationLocales(
                                            LocaleUtil.parseAlexaLocaleStringToAndroidLocaleList(
                                                currentSystemLocale
                                            )
                                        )
                                    }
                                }
                            }
                    }
            }
        }
    }

    private fun enableAACSSystemLocaleSync(context: Context) {
        val aacsLocaleIntent = Intent(AACSConstants.IntentAction.ENABLE_SYNC_SYSTEM_PROPERTY_CHANGE)
        aacsLocaleIntent.addCategory(AACSConstants.IntentCategory.SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT)
        aacsLocaleIntent.putExtra(AACSPropertyConstants.PROPERTY, AACSPropertyConstants.LOCALE)
        aacsLocaleIntent.component = ComponentName(
            AACSConstants.getAACSPackageName(
                WeakReference(
                    context
                )
            ), AACSConstants.AACS_CLASS_NAME
        )
        AACSServiceController.checkAndroidVersionAndStartService(context, aacsLocaleIntent)
    }

    private fun startPopup(context: Context, appLocale: String, alexaLocale: String) {
        Log.d(TAG, "System Language not supported by Alexa, bring up popup")
        val intent = Intent(context, SettingsActivity::class.java)
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_SINGLE_TOP
        intent.putExtra(PopupKeys.popupType, PopupKeys.languageMismatch)
        intent.putExtra(PopupKeys.appLocale, appLocale)
        intent.putExtra(PopupKeys.alexaLocale, alexaLocale)
        context.startActivity(intent)
    }

    companion object {
        private val TAG = AACSLocaleReceiver::class.java.canonicalName
    }
}