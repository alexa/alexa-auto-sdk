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

import com.amazon.aace.navigation.Navigation;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

public class NavigationHandler extends Navigation {

    private static String sTag = "Navigation";
    private LoggerHandler mLogger = null;

    public NavigationHandler( LoggerHandler logger ) { mLogger = logger; }

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
        return true;
    }
}
