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
package com.amazon.alexa.auto.comms.ui;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.amazon.alexa.auto.comms.ui.fragment.setup.CommunicationConsentFragment;

/**
 * Activity for Contacts Consent Popup
 */
public class RequestContactsConsentActivity extends AppCompatActivity {
    private static final String TAG = RequestContactsConsentActivity.class.getCanonicalName();

    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "Testing Contacts onCreate");
        super.onCreate(savedInstanceState);
        FragmentManager fm = getSupportFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();
        Fragment fragment = new CommunicationConsentFragment();

        ft.add(android.R.id.content, fragment, "CommunicationConsentFragment");

        ft.commit();
    }

    public void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
    }

    /**
     * Listens for new Intent created onClick to stop activity
     */
    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        boolean keep = intent.getExtras().getBoolean("keep");
        Log.d(TAG, "new intent started" + keep);
        if (keep == false) {
            this.finish();
        }
    }

    protected void onStop() {
        super.onStop();
        Log.d(TAG, "Contacts onStop");
        this.finish();
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
    }
}
