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
package com.amazon.alexa.auto.navigation.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.amazon.aacsconstants.NaviProviderConstants;
import com.amazon.alexa.auto.aacs_annotation_api.ContextBroadcastReceiver;
import com.amazon.alexa.auto.navigation.dependencies.AndroidModule;
import com.amazon.alexa.auto.navigation.dependencies.DaggerNaviComponent;
import com.amazon.alexa.auto.navigation.handlers.NaviDirectiveHandler;

import javax.inject.Inject;

@ContextBroadcastReceiver(categories = {NaviProviderConstants.CATEGORY_NAVI_FAVORITES},
        actions = {NaviProviderConstants.ACTION_UPLOAD_NAVI_FAVORITES,
                NaviProviderConstants.ACTION_REMOVE_NAVI_FAVORITES})
public class NaviFavoritesReceiver extends BroadcastReceiver {
    private static NaviFavoritesReceiver INSTANCE;

    @Inject
    NaviDirectiveHandler mNaviDirectiveHandler;

    // Private constructor for singleton
    private NaviFavoritesReceiver() {}

    public static NaviFavoritesReceiver getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new NaviFavoritesReceiver();
        }
        return INSTANCE;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mNaviDirectiveHandler == null) {
            DaggerNaviComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectNaviFavoritesReceiver(this);
        }
        if (NaviProviderConstants.ACTION_UPLOAD_NAVI_FAVORITES.equals(intent.getAction())) {
            mNaviDirectiveHandler.handleNavigationFavorites(true);
        } else if (NaviProviderConstants.ACTION_REMOVE_NAVI_FAVORITES.equals(intent.getAction())) {
            mNaviDirectiveHandler.handleNavigationFavorites(false);
        }
    }
}
