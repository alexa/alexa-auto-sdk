/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Navigation;

import android.app.Activity;
import android.os.Environment;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.navigation.Navigation;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;

public class NavigationHandler extends Navigation {

    private static String sTag = "Navigation";
    private static String sNavigationStateFilename = "NavigationState.json";
    private final Activity mActivity;
    private final LoggerHandler mLogger;

    private String mNavigationState = "";
    private SwitchCompat mNavigationStateUploadSwitch;

    public NavigationHandler(final Activity activity,
                             final LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;

        initialize();
    }

    private void initialize() {
        // initial UI setup when the app boots up

        // Switch to toggle nav context loaded state
        View switchItem = mActivity.findViewById( R.id.toggle_load_nav_state_file );
        ( (TextView) switchItem.findViewById( R.id.text ) ).setText( R.string.load_navigation_state_file);
        mNavigationStateUploadSwitch = switchItem.findViewById( R.id.drawerSwitch );
        mNavigationStateUploadSwitch.setChecked( false );

        // sets the listener on the navigation toggle controller
        mNavigationStateUploadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                View switchItem = mActivity.findViewById( R.id.navigation );
                TextView fileStatus = switchItem.findViewById( R.id.navigation_state_file_status);
                if(isChecked) {
                    if(loadNavigationState()) {
                        fileStatus.setText( R.string.navigation_state_file_loaded );
                    } else {
                        fileStatus.setText( R.string.navigation_state_file_error );
                        mActivity.runOnUiThread(
                                new Runnable() {
                                    @Override
                                    public void run() {
                                        mNavigationStateUploadSwitch.setChecked( false );
                                    }
                                }
                        );
                    }
                } else {
                    unloadNavigationState();
                    fileStatus.setText( R.string.navigation_state_file_not_loaded );
                }
            }
        });
    }

    private void unloadNavigationState() {
        synchronized (mNavigationState) {
            mNavigationState = "";
        }
    }

    private boolean loadNavigationState() {
        synchronized (mNavigationState) {
            String navigationStateRootFileName = Environment.getExternalStorageDirectory().getAbsolutePath() + "/NavigationState.json";
            File rootNavigationStateFile = new File( navigationStateRootFileName );
            StringBuilder text = new StringBuilder();
            if ( rootNavigationStateFile.exists() ) {
                try {
                    BufferedReader br = new BufferedReader(new FileReader(rootNavigationStateFile));
                    String line;

                    while ((line = br.readLine()) != null) {
                        text.append(line);
                        text.append('\n');
                    }
                    br.close();
                    mNavigationState = text.toString();
                    mLogger.postInfo(sTag, "loadNavigationState:" + mNavigationState);
                    return true;
                } catch (IOException e) {
                    mLogger.postInfo(sTag, e.getMessage());
                }
            }
            try {
                InputStream is = mActivity.getApplicationContext().getAssets().open(sNavigationStateFilename);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                mNavigationState = new String(buffer, "UTF-8");
                mLogger.postInfo(sTag, "loadNavigationState:" + mNavigationState);
                return true;
            } catch (IOException e) {
                mNavigationState = "";
                mLogger.postError(sTag, e.getMessage());
                return false;
            }
        }
    }

    @Override
    public String getNavigationState() {
        synchronized (mNavigationState) {
            return mNavigationState;
        }
    }

    @Override
    public boolean setDestination( String payload ) {
        // Handle navigation to destination here
        try {
            // Log payload
            JSONObject template = new JSONObject( payload );
            mLogger.postJSONTemplate( sTag, template.toString( 4 ) );

            // Log display card
            mLogger.postDisplayCard( template, LogRecyclerViewAdapter.SET_DESTINATION_TEMPLATE );

            return true;
        } catch ( JSONException e ) {
            mLogger.postError( sTag, e.getMessage() );
            return false;
        }
    }

    @Override
    public boolean cancelNavigation() {
        mLogger.postInfo( sTag, "Cancel Navigation Called" );

        mActivity.runOnUiThread(
            new Runnable() {
                @Override
                public void run() {
                    mNavigationStateUploadSwitch.setChecked( false );
                }
            }
        );
        return true;
    }
}
