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

package com.amazon.sampleapp.connectivity;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.amazon.aace.connectivity.AlexaConnectivity;
import com.amazon.sampleapp.connectivity.R;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AlexaConnectivityHandler extends AlexaConnectivity {
    public enum ConnectivityState {
        INIT("Please Select a State"),
        /**
         * This state represents when managedProvider under Connectivity API is set to NOT_MANAGED
         * @hideinitializer
         */
        NOT_MANAGED("Full Experience (Not Managed)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * DataPlanType is PAID
         *
         * @hideinitializer
         */
        PAID("Full Experience (Paid)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * DataPlanType is TRIAL
         * @hideinitializer
         */
        TRIAL("Full Experience (Trial)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * DataPlanType is TRIAL and DataPlanEndDate in a date about to past endDate
         * @hideinitializer
         */
        TRIAL_EXPIRING("Full Experience (Trial Expiring in 5 days)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * DataPlanType is AMAZON_SPONSORED
         * @hideinitializer
         */
        AMAZON_SPONSORED("Partial Experience (Amazon Sponsored)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * Terms and Condition is declined by user
         * @hideinitializer
         */
        TERMS_DECLINED("Partial Experience (Terms Declined)"),
        /**
         * This state represents when managedProvider under Connectivity API is set to MANAGED, Provider is AMAZON,
         * DataPlanType is AMAZON_SPONSORED and TRIAL is not available since it is expired.
         * @hideinitializer
         */
        TRIAL_EXPIRED("Partial Experience (Trial Expired)");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        ConnectivityState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }
    private static final String sTag = "AlexaConnectivityHandler";
    private static final String sNOT_MANAGED_PROVIDER = "NOT_MANAGED";
    private static final String sMANAGED_PROVIDER = "MANAGED";
    private static final String sMANAGED_PROVIDER_ID = "AMAZON";
    private static final String sDATAPLAN_SPONSORED = "AMAZON_SPONSORED";
    private static final String sDATAPLAN_PAID = "PAID";
    private static final String sDATAPLAN_TRIAL = "TRIAL";
    private static final String sTERMS_ACCEPTED = "ACCEPTED";
    private static final String sTERMS_DECLINED = "DECLINED";
    private static final DateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'");
    public static final String PREFERENCES_FILE = "ConnectivityStatePrefs";
    public static final String POSITION_KEY = "POSITION";

    private final Activity mActivity;
    private int mConnectionStatePos;
    private Spinner mConnectivitySpinner;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    protected String mConnectivityPayload;

    public AlexaConnectivityHandler(SampleAppContext sampleAppContext) {
        Log.i(sTag, "Creating AlexaConnectivityHandler");
        mActivity = sampleAppContext.getActivity();
        mConnectivityPayload =
                "{\"managedProvider\":{\"type\":\"NOT_MANAGED\"},\"dataPlan\":{\"type\":\"AMAZON_SPONSORED\"}}";
        // The value provided by SimpleDateFormat Z doesn't include the colon separator specified in
        // RFC 3339 (https://tools.ietf.org/html/rfc3339#section-5.6). We either need to split the
        // [+-] hours from the minutes and rejoin with colon, or set UTC TimeZone and literal Z.
        sDateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        setupGUI();
    }

    @Override
    public String getConnectivityState() {
        Log.i(sTag, "Getting ConnectivityState Overrided in Connectivity Handler");
        return mConnectivityPayload;
    }

    public void onConnectivityStateItemSelected(ConnectivityState state) {
        if (state == ConnectivityState.INIT)
            return;
        Log.i(sTag, "Connectivity state dropdown changed");
        String payload = connectivityPayloadGenerator(state);
        Log.i(sTag, "Connectivity Connectivity Payload: \n" + payload);
        connectivityStateChange();
    }

    public String connectivityPayloadGenerator(ConnectivityState state) {
        JSONObject payload = new JSONObject();
        JSONObject managedProvider = new JSONObject();
        JSONObject dataPlan = new JSONObject();

        // Create Calendar object to setup for future date
        Calendar calendar = Calendar.getInstance();

        switch (state) {
            case NOT_MANAGED:
                try {
                    managedProvider.put("type", sNOT_MANAGED_PROVIDER);
                    payload.put("managedProvider", managedProvider);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag, "Error setting payload to not managed provider: " + e.getMessage());
                }
                break;

            case PAID:
                try {
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    dataPlan.put("type", sDATAPLAN_PAID);
                    payload.put("termsStatus", sTERMS_ACCEPTED);
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag, "Error setting payload to paid full experience: " + e.getMessage());
                }
                break;

            case TRIAL:
                try {
                    // Get today's date and add arbitary date within 30 day upsell notice date
                    calendar.setTime(new Date());
                    calendar.add(Calendar.DATE, 30);
                    dataPlan.put("type", sDATAPLAN_TRIAL);
                    dataPlan.put("endDate", sDateFormat.format(calendar.getTime()));
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    payload.put("termsStatus", sTERMS_ACCEPTED);
                    String[] dataplanList = {sDATAPLAN_PAID, sDATAPLAN_SPONSORED};
                    payload.put("dataPlansAvailable", new JSONArray(dataplanList));
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag, "Error setting payload to trial full experience: " + e.getMessage());
                }
                break;

            case AMAZON_SPONSORED:
                try {
                    dataPlan.put("type", sDATAPLAN_SPONSORED);
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    payload.put("termsStatus", sTERMS_ACCEPTED);
                    String[] dataplanList = {sDATAPLAN_PAID, sDATAPLAN_TRIAL};
                    payload.put("dataPlansAvailable", new JSONArray(dataplanList));
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag, "Error setting payload to partial experience mode: " + e.getMessage());
                }
                break;

            case TERMS_DECLINED:
                try {
                    dataPlan.put("type", sDATAPLAN_SPONSORED);
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    payload.put("termsStatus", sTERMS_DECLINED);
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag,
                            "Error setting payload to partial experience mode with terms declined: " + e.getMessage());
                }
                break;

            // Switch to partial experience mode once trial is expired with only paid option available to select
            case TRIAL_EXPIRED:
                try {
                    dataPlan.put("type", sDATAPLAN_SPONSORED);
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    payload.put("termsStatus", sTERMS_ACCEPTED);
                    String[] dataplanList = {sDATAPLAN_PAID};
                    payload.put("dataPlansAvailable", new JSONArray(dataplanList));
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag,
                            "Error setting payload to partial experience mode with trial expired: " + e.getMessage());
                }
                break;

            case TRIAL_EXPIRING:
                try {
                    // Get today's date and add arbitary date of 5 days to trigger trial expiring notice
                    calendar.setTime(new Date());
                    calendar.add(Calendar.DATE, 5);
                    dataPlan.put("type", sDATAPLAN_TRIAL);
                    dataPlan.put("endDate", sDateFormat.format(calendar.getTime()));
                    managedProvider.put("type", sMANAGED_PROVIDER);
                    managedProvider.put("id", sMANAGED_PROVIDER_ID);
                    payload.put("termsStatus", sTERMS_ACCEPTED);
                    String[] dataplanList = {sDATAPLAN_PAID, sDATAPLAN_SPONSORED};
                    payload.put("dataPlansAvailable", new JSONArray(dataplanList));
                    payload.put("managedProvider", managedProvider);
                    payload.put("dataPlan", dataPlan);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag, "Error setting payload to trial mode with trial expiring in 5 days: " + e.getMessage());
                }
                break;

            default:
                try {
                    payload = new JSONObject(mConnectivityPayload);
                } catch (JSONException e) {
                    payload = new JSONObject();
                    Log.e(sTag,
                            "Error setting to previous selected state, Please select a valid state. " + e.getMessage());
                }
                break;
        }
        mConnectivityPayload = payload.toString();
        return mConnectivityPayload;
    }

    public void setupGUI() {
        mConnectivitySpinner = mActivity.findViewById(R.id.connectivitySpinner);
        ArrayAdapter<AlexaConnectivityHandler.ConnectivityState> adapter =
                new ArrayAdapter<AlexaConnectivityHandler.ConnectivityState>(
                        mActivity, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        for (AlexaConnectivityHandler.ConnectivityState state : AlexaConnectivityHandler.ConnectivityState.values()) {
            adapter.add(state);
        }

        mConnectivitySpinner.setAdapter(adapter);

        if (readInstanceState(mActivity)) {
            mConnectivitySpinner.setSelection(mConnectionStatePos);
        }

        mConnectivitySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                AlexaConnectivityHandler.ConnectivityState state =
                        (AlexaConnectivityHandler.ConnectivityState) parent.getItemAtPosition(position);
                mConnectionStatePos = position;
                if (!writeInstanceState(mActivity)) {
                    Toast.makeText(mActivity, "Failed to write state!", Toast.LENGTH_LONG).show();
                }
                onConnectivityStateItemSelected(state);
            }
            public void onNothingSelected(AdapterView<?> parent) {}
        });
    }

    /**
     * Read the previous selected item for dropdown menu of the spinner from the preferences file
     * @param acitivity - Current Activity
     */
    public boolean readInstanceState(Activity activity) {
        // Get the SharedPreferences object for sample from local storage
        SharedPreferences perf = activity.getSharedPreferences(PREFERENCES_FILE, Context.MODE_PRIVATE);

        // Get the position and value dropdown from local storage, or a default value as 0 if no menu option selected
        this.mConnectionStatePos = perf.getInt(POSITION_KEY, 1);
        return (perf.contains(POSITION_KEY));
    }

    /**
     * Write the current selected dropdown menu option position to local storage
     * @param acitivity - Current Activity
     */
    public boolean writeInstanceState(Activity activity) {
        // Get the SharedPreferences object for Sample App
        SharedPreferences perf = activity.getSharedPreferences(this.PREFERENCES_FILE, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = perf.edit();

        // Use SharedPreferences editor to write dropdown menu position with its key value
        editor.putInt(POSITION_KEY, this.mConnectionStatePos);

        // Commit the editor change to local storage
        return (editor.commit());
    }
}