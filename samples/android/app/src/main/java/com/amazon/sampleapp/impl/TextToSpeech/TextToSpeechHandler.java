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

package com.amazon.sampleapp.impl.TextToSpeech;

import android.app.Activity;
import android.content.Context;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Environment;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.amazon.aace.audio.AudioStream;
import com.amazon.aace.textToSpeech.TextToSpeech;
import com.amazon.sampleapp.FileUtils;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * The handler provides a sample implementation of the TextToSpeech platform
 * interface.
 */
public class TextToSpeechHandler extends TextToSpeech {
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final Context mContext;
    private static final String sTag = "TextToSpeech";
    private static final String ID = "TEXT_TO_SPEECH";
    private static final String TEXT_KEY = "text";
    private static final String SSML_KEY = "ssml";
    private static final String TEXT_TO_SPEECH_PROVIDER = "text-to-speech-provider";
    private long idCounter = 0;

    /**
     * Constructor for TextToSpeech handler.
     * @param activity The Android activity instance.
     * @param logger The logger instance.
     */

    public TextToSpeechHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mLogger.postInfo(sTag, "Creating TextToSpeechHandler");
        mContext = mActivity.getApplicationContext();
        try {
            setupGUI();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void prepareSpeechCompleted(String speechId, AudioStream preparedAudio, String metadata) {
        mLogger.postInfo(sTag, "speechId : " + speechId + ", metadata : " + metadata);

        // This code is for testing purpose only. OEMs should follow the correct UX guidelines to
        // play the audio returned in this API.
        try (FileOutputStream os = mContext.openFileOutput("test-audio", Context.MODE_PRIVATE)) {
            byte[] buffer = new byte[4096];
            int size;
            while (!preparedAudio.isClosed()) {
                while ((size = preparedAudio.read(buffer)) > 0) os.write(buffer, 0, size);
            }
        } catch (Exception e) {
            mLogger.postError(sTag, e);
        }

        try {
            Uri uri = Uri.fromFile(mContext.getFileStreamPath("test-audio"));
            MediaPlayer mediaPlayer = new MediaPlayer();
            mediaPlayer.setDataSource(String.valueOf(uri));
            mediaPlayer.prepare();
            mediaPlayer.start();
        } catch (Exception e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void prepareSpeechFailed(String speechId, String reason) {
        mLogger.postInfo(sTag, "speechId : " + speechId + " reason : " + reason);
    }

    @Override
    public void capabilitiesReceived(String requestId, String capabilities) {
        mLogger.postInfo(sTag, "request Id" + requestId + "capabilities : " + capabilities);
    }

    public void setupGUI() {
        try {
            mLogger.postInfo(sTag, "Setting up UI...");

            final List<String> ttsTextOnlyArray = new ArrayList<String>(
                    Arrays.asList("", "Take Exit 39B", "Take the next left", "In 500ft, take the next exit",
                            "Wrong way, turn around", "You have reached your destination"));

            final List<String> ttsSSMLOnlyArray = new ArrayList<String>(Arrays.asList(
                    "", "<speak> Turn right on 3rd Lane <sub alias=\"Mercury\">Ln</sub> to merge </speak>"));

            // The TextToSpeechTestCases.json should be used for testing purposes only.
            // The OEMs are expected to call the prepareSpeech() API with the correct text/SSML string
            String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
            String externalTTSFilePath = sdCardPath + "/TextToSpeechTestCases.json";
            if (new File(externalTTSFilePath).exists()) {
                JSONObject ttsTestCases = FileUtils.parseFileAsJSONObject(externalTTSFilePath);
                if (ttsTestCases.has(TEXT_KEY)) {
                    JSONArray textOnlyCases = ttsTestCases.getJSONArray(TEXT_KEY);
                    for (int i = 0; i < textOnlyCases.length(); i++) {
                        ttsTextOnlyArray.add(textOnlyCases.get(i).toString());
                    }
                }
                if (ttsTestCases.has(SSML_KEY)) {
                    JSONArray ssmlOnlyCases = ttsTestCases.getJSONArray(SSML_KEY);
                    for (int i = 0; i < ssmlOnlyCases.length(); i++) {
                        ttsSSMLOnlyArray.add(ssmlOnlyCases.get(i).toString());
                    }
                }
            }
            setUpTTSSpinner(ttsTextOnlyArray, TEXT_KEY);
            setUpTTSSpinner(ttsSSMLOnlyArray, SSML_KEY);
        } catch (Exception e) {
            mLogger.postError(sTag, e);
        }
    }

    public String generateId() {
        return String.valueOf(++idCounter);
    }

    public void setUpTTSSpinner(List<String> testCases, String type) {
        Spinner textToSpeechSpinner = null;
        if (type.equals("text")) {
            textToSpeechSpinner = (Spinner) mActivity.findViewById(R.id.textToSpeechTextOnlySpinner);
        } else if (type.equals("ssml")) {
            textToSpeechSpinner = (Spinner) mActivity.findViewById(R.id.textToSpeechSSMLOnlySpinner);
        }
        final String[] ttsStrings = new String[testCases.size()];
        ArrayAdapter<String> textToSpeechAdapter = new ArrayAdapter<String>(
                mActivity, android.R.layout.simple_spinner_item, testCases.toArray(ttsStrings));
        textToSpeechAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        textToSpeechSpinner.setAdapter(textToSpeechAdapter);
        textToSpeechSpinner.setSelection(0);

        textToSpeechSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String ttsString = testCases.get(position);
                String speechId = ID + "-" + generateId();
                String options = "";
                Toast.makeText(mActivity, "Generating speech " + ttsString + "with Speech ID : " + speechId,
                             Toast.LENGTH_SHORT)
                        .show();
                prepareSpeech(speechId, ttsString, TEXT_TO_SPEECH_PROVIDER, options);
            }
            public void onNothingSelected(AdapterView<?> parent) {}
        });
    }
}