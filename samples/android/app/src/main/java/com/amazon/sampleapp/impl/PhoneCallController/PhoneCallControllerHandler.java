/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.PhoneCallController;

import android.app.Activity;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.support.v7.widget.SwitchCompat;
import android.widget.CompoundButton;

import com.amazon.aace.phonecontrol.PhoneCallController;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

public class PhoneCallControllerHandler extends PhoneCallController {

    private static final String sTag = "PhoneCallController";
    private static final String sConnectionError = "503";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private RelativeLayout mCallInfoView;
    private TextView mInitiateCallButton, mTerminateCallButton;
    private TextView mCalleeNumberView, mCallStateView;
    private String mCallId, mCalleeNumber;
    private boolean mCallActivated = false;
    private ConnectionState mConnectionState = ConnectionState.DISCONNECTED;

    public PhoneCallControllerHandler( Activity activity, LoggerHandler logger ) {
        mActivity = activity;
        mLogger = logger;
        setupGUI();
    }

    @Override
    public boolean dial( String payload ) {
        String callId = "";
        String calleeNumber = "";

        try {
            JSONObject obj = new JSONObject( payload );
            mLogger.postJSONTemplate( sTag, obj.toString( 4 ) );
            callId = obj.getString( "callId" );
            calleeNumber = getCalleeDefaultAddressValue( obj );
        } catch ( JSONException e ) {
            mLogger.postError( sTag, "Error parsing dial directive payload: "
                    + e.getMessage() );
        }

        if ( mCallActivated ) {
            mLogger.postInfo( sTag, "Received dial(), Call State is already active" );
            return false;
        }

        mCallId = callId;
        mCalleeNumber = calleeNumber;

        if ( mConnectionState == ConnectionState.DISCONNECTED ) {
            mLogger.postInfo( sTag, "Received dial(), ConnectionState: DISCONNECTED" );
            return false;
        } else {
            updateViewOnDial();
            return true;
        }
    }

    private void togglePhoneConnectionState( boolean enable ) {
        if ( enable ) {
            mConnectionState = ConnectionState.CONNECTED;
            mLogger.postInfo( sTag, "ConnectionState: CONNECTED" );
            connectionStateChanged( ConnectionState.CONNECTED );
            updateViewOnConnected();
        } else {
            mConnectionState = ConnectionState.DISCONNECTED;
            mLogger.postInfo( sTag, "ConnectionState: DISCONNECTED" );
            connectionStateChanged( ConnectionState.DISCONNECTED );
            updateViewOnDisconnected();

            // Notify the Engine if phone disconnects during active call
            if ( mCallActivated ) {
                mCallActivated = false;
                mLogger.postInfo( sTag, "Call Failed, ConnectionState: DISCONNECTED" );
                callFailed( mCallId, sConnectionError, "Connection lost" );
            }
        }
    }

    private void onCallInitiated() {
        mCallActivated = true;
        mLogger.postInfo( sTag, String.format( "Call Initiated. CALL ID: %s, NUMBER: %s",
                mCallId, mCalleeNumber ) );
        callActivated( mCallId );
        updateViewOnCallInitiated();
    }

    private void onCallTerminated() {
        mCallActivated = false;
        mLogger.postInfo( sTag, String.format( "Call Terminated. CALL ID: %s, NUMBER: %s",
                mCallId, mCalleeNumber ) );
        callTerminated( mCallId );
        updateViewOnCallTerminated();
    }

    private String getCalleeDefaultAddressValue( JSONObject payload ) {
        String address = "";
        try {
            JSONObject callee = payload.getJSONObject( "callee" );
            if ( callee != null ) {
                JSONObject defaultAddress = callee.getJSONObject( "defaultAddress" );
                if ( defaultAddress != null ) {
                    return defaultAddress.getString( "value" );
                }
            }
        } catch ( JSONException e ) {
            mLogger.postError( sTag, "Error parsing callee default address value: "
                    + e.getMessage() );
        }
        return address;
    }

    /* For Updating GUI */

    private void setupGUI() {

        // Switch to toggle phone call connection state
        View switchItem = mActivity.findViewById( R.id.togglePhoneCallConnection );
        ( (TextView) switchItem.findViewById( R.id.text ) ).setText( R.string.phone_call_connection_switch);
        SwitchCompat phoneCallConnectionSwitch = switchItem.findViewById( R.id.drawerSwitch );
        phoneCallConnectionSwitch.setChecked( false );
        phoneCallConnectionSwitch.setOnCheckedChangeListener(
                new CompoundButton.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                        togglePhoneConnectionState( isChecked );
                    }
                }
        );

        mCallInfoView = mActivity.findViewById( R.id.callInfo );
        mCalleeNumberView = mActivity.findViewById( R.id.calleeNumber );
        mCallStateView = mActivity.findViewById( R.id.callState );
        mInitiateCallButton =  mActivity.findViewById( R.id.initiateCall );
        mTerminateCallButton = mActivity.findViewById( R.id.terminateCall );

        mInitiateCallButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) { onCallInitiated(); }
        });
        mTerminateCallButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) { onCallTerminated(); }
        });
    }

    private void updateViewOnDial () {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mCalleeNumberView.setText( mCalleeNumber );
                mInitiateCallButton.setVisibility( View.VISIBLE );
                mTerminateCallButton.setVisibility( View.GONE );
            }
        });
    }

    private void updateViewOnConnected() {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mCallInfoView.setVisibility( View.VISIBLE );
                mInitiateCallButton.setVisibility( View.VISIBLE );
                mTerminateCallButton.setVisibility( View.GONE );
            }
        });
    }

    private void updateViewOnDisconnected() {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mCallInfoView.setVisibility( View.GONE );
                mInitiateCallButton.setVisibility( View.GONE );
                mTerminateCallButton.setVisibility( View.GONE );
                mCallStateView.setText( R.string.call_state_idle );
            }
        });
    }

    private void updateViewOnCallInitiated() {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mInitiateCallButton.setVisibility( View.GONE );
                mTerminateCallButton.setVisibility( View.VISIBLE );
                mCallStateView.setText( R.string.call_state_active );
            }
        });
    }

    private void updateViewOnCallTerminated() {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mInitiateCallButton.setVisibility( View.VISIBLE );
                mTerminateCallButton.setVisibility( View.GONE );
                mCallStateView.setText( R.string.call_state_idle );
            }
        });
    }
}

