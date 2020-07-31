/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.maccandroid;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.media.MediaBrowserServiceCompat;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.PackageMetadata;
import com.amazon.maccandroid.model.errors.CapabilityAgentError;
import com.amazon.maccandroid.model.players.DiscoveredPlayer;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class DiscoverAndReportMediaAppsHandler extends Handler {
    private static final String TAG = DiscoverAndReportMediaAppsHandler.class.getSimpleName();

    public static final int START_DISCOVERY = 1;
    public static final int REPORT_DISCOVERED_MEDIA_APPS = 2;
    public static final int WAIT_FOR_AUTHORIZATION = 3;
    public static final int REPORT_AUTHORIZATION_COMPLETE = 4;
    public static final String EMP_METADATA_FLAG = "com.amazon.alexa.externalmediaplayer.metadata";

    private Context mContext;
    private PackageManager mPackageManager;

    DiscoverAndReportMediaAppsHandler(Looper looper, Context context) {
        super(looper);
        mContext = context;
        mPackageManager = context.getPackageManager();
        Log.d(TAG, "DiscoverAndReportMediaAppsHandler constructor called");
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);
        switch (msg.what) {
            case START_DISCOVERY: {
                Log.i(TAG, "START_DISCOVERY ");
                discoverMediaApps();
                cleanUpRemovedApps();
                break;
            }

            case REPORT_DISCOVERED_MEDIA_APPS:
                Log.i(TAG, "REPORT_DISCOVERED_MEDIA_APPS ");
                Collection<MediaApp> mediaApps = MediaAppsRepository.getInstance().getDiscoveredMediaApps().values();
                for (MediaApp mediaApp : mediaApps) {
                    if (MediaAppsRepository.getInstance().isAuthorizedApp(mediaApp.getLocalPlayerId())) {
                        Log.i(TAG,
                                "REPORT_DISCOVERED_MEDIA_APPS | appid: " + mediaApp.getLocalPlayerId()
                                        + " is already authorized no need to report again");
                        mediaApps.remove(mediaApp);
                    }
                    Log.i(TAG, "REPORT_DISCOVERED_MEDIA_APPS media apps " + mediaApp.getLocalPlayerId());
                }
                MediaAppsStateReporter.getInstance().reportDiscoveredPlayers(
                        DiscoveredPlayer.convertMediaApps(new ArrayList<MediaApp>(mediaApps)));
                break;
            default:
                break;
        }
    }

    /**
     * Cleans up previously authorized apps that were removed or uninstalled from the system
     */
    private void cleanUpRemovedApps() {
        MediaAppsRepository appsRepository = MediaAppsRepository.getInstance();
        for (MediaApp app : appsRepository.getAuthorizedMediaApps().values()) {
            if (!appsRepository.isDiscoveredApp(app.getLocalPlayerId())) {
                Log.i(TAG,
                        "cleanUpRemovedApps | appId: " + app.getLocalPlayerId()
                                + " is no longer discovered removing app");
                appsRepository.removeMediaApp(app.getLocalPlayerId());
            }
        }
    }

    /**
     * Discovers MACC compliant media apps on the system.
     */
    public void discoverMediaApps() {
        // Clear discovered apps so we don't keep apps that could have been uninstalled or changed
        MediaAppsRepository.getInstance().clearDiscoveredApps();

        // Build an Intent that only has the MediaBrowserService action and query
        // the PackageManager for apps that have services registered that can
        // receive it and get the meta data component associated with it
        final Intent mediaBrowserIntent = new Intent(MediaBrowserServiceCompat.SERVICE_INTERFACE);
        final List<ResolveInfo> services =
                mPackageManager.queryIntentServices(mediaBrowserIntent, PackageManager.GET_META_DATA);

        if (services == null || services.isEmpty())
            return;

        Log.i(TAG, "services: " + services);

        for (final ResolveInfo resolveInfo : services) {
            Log.i(TAG,
                    "Discovered app: " + resolveInfo.serviceInfo.packageName
                            + " | meta data: " + resolveInfo.serviceInfo.metaData);
            if (resolveInfo == null || resolveInfo.serviceInfo == null
                    || resolveInfo.serviceInfo.applicationInfo == null || resolveInfo.serviceInfo.metaData == null
                    || !resolveInfo.serviceInfo.metaData.containsKey(EMP_METADATA_FLAG)) {
                continue;
            }

            String packageName = resolveInfo.serviceInfo.packageName;
            String className = resolveInfo.serviceInfo.name;
            Log.i(TAG, "App name " + packageName);

            PackageMetadata appMetaData = getPlayerMetaData(resolveInfo);
            if (appMetaData == null) {
                Log.e(TAG, "Something wen't wrong when parsing meta data");
                continue;
            }
            // if authorized mediaApp already exists, use it. don't create another new one
            MediaAppsRepository appsRepository = MediaAppsRepository.getInstance();
            if (appsRepository.isAuthorizedApp(packageName)) {
                appsRepository.addDiscoveredMediaApp(appsRepository.getAuthorizedMediaApp(packageName));
            } else {
                MediaApp mediaApp = MediaApp.create(
                        mContext, packageName, className, appMetaData.getSpiVersion(), appMetaData.getPlayerCookie());
                appsRepository.addDiscoveredMediaApp(mediaApp);
                mediaApp.connect(new MediaAppsConnectionListener() {
                    @Override
                    public void onConnectionSuccessful() {
                        Log.i(TAG, "onConnectionSuccessful");
                    }

                    @Override
                    public void onConnectionFailure(CapabilityAgentError error) {
                        Log.e(TAG, "onConnectionFailure | " + error.toString());
                    }
                });
            }
        }
    }

    /**
     * Uses a resolveInfo object to fetch the metadata in that package
     * @param resolveInfo
     * @return Metadata object
     */
    /*package*/ PackageMetadata getPlayerMetaData(ResolveInfo resolveInfo) {
        int resId = resolveInfo.serviceInfo.metaData.getInt(EMP_METADATA_FLAG);
        if (resId == 0) {
            Log.i(TAG, "Invalid resource provided: " + resId);
            return null;
        }

        JSONObject empMetaData = getPackageMetaDataFromResourceId(resolveInfo.serviceInfo.packageName, resId);
        Log.i(TAG, "empData: " + empMetaData);
        if (empMetaData == null) {
            Log.i(TAG, "invalid empMetaData provided");
            return null;
        }

        String playerCookie = null, spiVersion = null;

        try {
            spiVersion = empMetaData.getString("spiVersion");
        } catch (JSONException e) {
            e.printStackTrace();
        }

        try {
            playerCookie = empMetaData.getString("playerCookie");
        } catch (JSONException e) {
            e.printStackTrace();
        }

        if (spiVersion == null) {
            Log.i(TAG, "no spi version specified ");
            return null;
        }

        return new PackageMetadata(spiVersion, playerCookie);
    }

    /**
     * Given the package  name and resource id this function will return a JSON object
     * that corresponds to the metadata declared in the package.
     * @param packageName
     * @param resourceId
     * @return JSONObject which encapsulates the metadata declared
     */
    private JSONObject getPackageMetaDataFromResourceId(String packageName, int resourceId) {
        try {
            InputStream resourceInputStream =
                    mPackageManager.getResourcesForApplication(packageName).openRawResource(resourceId);
            BufferedReader streamReader = new BufferedReader(new InputStreamReader(resourceInputStream, "UTF-8"));
            StringBuilder responseStrBuilder = new StringBuilder();
            String inputStr;
            while ((inputStr = streamReader.readLine()) != null) responseStrBuilder.append(inputStr);
            return new JSONObject(responseStrBuilder.toString());
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }
}
