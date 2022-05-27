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
package com.amazon.alexa.auto.navigation.providers.external;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.NaviProviderConstants;
import com.amazon.alexa.auto.aacs_annotation_api.ContextBroadcastReceiver;

@ContextBroadcastReceiver(categories = {NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION},
        actions = {NaviProviderConstants.ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES,
                NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_STATE,
                NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_FAVORITES,
                NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_ERROR,
                NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_EVENT})
public class NaviObserver extends BroadcastReceiver {
    private static final String TAG = "NaviObserver";

    private static NaviObserver INSTANCE;
    private NaviDataListener mNaviData;

    private NaviObserver() {}

    public static NaviObserver getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new NaviObserver();
        }
        return INSTANCE;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }
        switch (intent.getAction()) {
            case NaviProviderConstants.ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES:
                String alternateRoutes =
                        intent.getStringExtra(NaviProviderConstants.PAYLOAD_ALTERNATIVE_ROUTES_RESPONSE);
                mNaviData.showAlternativeRoutesSucceeded(alternateRoutes);
                break;
            case NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_STATE:
                String navState = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_STATE);
                mNaviData.setNavState(navState);
                break;
            case NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_FAVORITES:
                String navFavorites = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_FAVORITES);
                mNaviData.setNavFavorites(navFavorites);
                break;
            case NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_ERROR:
                String errorType = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_ERROR_TYPE);
                String errorCode = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_ERROR_CODE);
                String description = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_ERROR_DESCRIPTION);
                mNaviData.setNavError(errorType, errorCode, description);
                break;
            case NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_EVENT:
                String event = intent.getStringExtra(NaviProviderConstants.PAYLOAD_NAVIGATION_EVENT);
                mNaviData.setNavEvent(event);
                break;
            default:
                Log.w(TAG, "Unexpected action received: " + intent.getAction());
                break;
        }
    }

    public void setNaviDataListener(NaviDataListener naviData) {
        mNaviData = naviData;
    }
}
