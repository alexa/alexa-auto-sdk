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

package com.amazon.aacstts.handler;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;

import android.util.Log;

import com.amazon.aacstts.SynthesizeTextUtil;
import com.amazon.aacstts.TTSConstants;
import com.amazon.aacstts.models.ProviderVoiceItem;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * TTSHandler is an AACS message handler which implements {@link IAACSMessageHandler}. It is registered
 * with {@link com.amazon.aacstts.MessageHandler} and is used for handling messages with TextToSpeech topic.
 */
public class TTSHandler implements IAACSMessageHandler {
    private static final String TAG = AACS_TTS_LOG_PREFIX + TTSHandler.class.getSimpleName();

    public static final String CAPABILITIES_KEY = "capabilities";
    public static final String CAPABILITIES_VOICES_KEY = "voices";
    public static final String CAPABILITIES_VOICE_ID_KEY = "voiceId";
    public static final String CAPABILITIES_LOCALES_KEY = "locales";

    // Setup a local cache for maintaining Languages.
    private static ConcurrentHashMap<String, List<ProviderVoiceItem>> mCapabilitiesCache;

    private SynthesizeTextUtil mSynthesizeTextUtil;

    public static final String ALEXA_VOICE_ID = "alexa";

    public static Set<String> RESTRICTED_VOICE_NAMES;
    static {
        Set<String> mRestrictedVoiceNames = new HashSet<>();
        mRestrictedVoiceNames.add(ALEXA_VOICE_ID);
        RESTRICTED_VOICE_NAMES = Collections.unmodifiableSet(mRestrictedVoiceNames);
    }

    public TTSHandler(SynthesizeTextUtil synthesizeTextUtil) {
        mSynthesizeTextUtil = synthesizeTextUtil;
        mCapabilitiesCache = new ConcurrentHashMap<>();
    }

    public ConcurrentHashMap<String, List<ProviderVoiceItem>> getCapabilitiesCache() {
        return mCapabilitiesCache;
    }

    /**
     * Remove all the mappings from the capabilities cache.
     */
    public static void clearCapabilitiesCache() {
        Log.d(TAG, "Clear capabilities cache");
        mCapabilitiesCache.clear();
    }

    @Override
    public void handleAACSMessage(String messageId, String topic, String action, String payload) {
        Log.d(TAG, "handleTTS." + action + payload);
        if (action.equals(TTSConstants.Action.GET_CAPABILITIES)) {
            handleGetCapabilities(payload);
        } else if (action.equals(TTSConstants.Action.PREPARE_SPEECH)) {
            handlePrepareSpeech(payload);
        } else if (action.equals(TTSConstants.Action.PREPARE_SPEECH_COMPLETED)) {
            handlePrepareSpeechCompleted(payload);
        } else if (action.equals(TTSConstants.Action.PREPARE_SPEECH_FAILED)) {
            handlePrepareSpeechFailed(payload);
        }
    }

    private void handleGetCapabilities(String payload) {
        Log.d(TAG, "handleGetCapabilities " + payload);

        try {
            JSONObject payloadJson = new JSONObject(payload);
            String capabilities = payloadJson.getString(CAPABILITIES_KEY);
            JSONObject capabilitiesJson = new JSONObject(capabilities);
            Iterator<String> iter = capabilitiesJson.keys();
            if (!iter.hasNext()) {
                Log.e(TAG, "getCapabilities message payload is empty.");
                return;
            }

            while (iter.hasNext()) {
                String key = iter.next();
                mCapabilitiesCache.putIfAbsent(key, new ArrayList<>());
                List<ProviderVoiceItem> voiceItems = mCapabilitiesCache.get(key);

                if (capabilitiesJson.optJSONObject(key) == null) {
                    Log.e(TAG, String.format("JSONObject is not found for key=%s.", key));
                    return;
                }

                JSONArray voices = capabilitiesJson.optJSONObject(key).optJSONArray(CAPABILITIES_VOICES_KEY);
                if (!Objects.isNull(voices)) {
                    for (int voiceIndex = 0; voiceIndex < voices.length(); voiceIndex++) {
                        ProviderVoiceItem voiceItem = getVoiceItem(voices.optJSONObject(voiceIndex));
                        if (voiceItem != null
                                && !RESTRICTED_VOICE_NAMES.contains(voiceItem.getVoiceId().toLowerCase())) {
                            voiceItems.add(voiceItem);
                        }
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, "getCapabilities JSON cannot be parsed.");
        }
    }

    private ProviderVoiceItem getVoiceItem(JSONObject jsonObject) {
        Log.d(TAG, "getVoiceItem " + jsonObject);
        ProviderVoiceItem providerVoiceItem = null;
        if (jsonObject != null) {
            String voiceId = jsonObject.optString(CAPABILITIES_VOICE_ID_KEY);
            JSONArray locales = jsonObject.optJSONArray(CAPABILITIES_LOCALES_KEY);

            List<String> supportedLocales = new ArrayList<>();
            if (!Objects.isNull(locales)) {
                for (int localeIndex = 0; localeIndex < locales.length(); localeIndex++) {
                    supportedLocales.add(locales.optString(localeIndex));
                }
            }

            providerVoiceItem = new ProviderVoiceItem(voiceId, supportedLocales);
        }
        return providerVoiceItem;
    }

    private void handlePrepareSpeech(String payload) {
        Log.d(TAG, "handlePrepareSpeech " + payload);
    }

    private void handlePrepareSpeechCompleted(String payload) {
        Log.d(TAG, "handlePrepareSpeechCompleted " + payload);
        mSynthesizeTextUtil.handlePrepareSpeechResponse(payload);
    }

    private void handlePrepareSpeechFailed(String payload) {
        Log.d(TAG, "handlePrepareSpeechFailed " + payload);
        mSynthesizeTextUtil.handlePrepareSpeechResponse(payload);
    }

    @Override
    public void cleanUp() {}
}
