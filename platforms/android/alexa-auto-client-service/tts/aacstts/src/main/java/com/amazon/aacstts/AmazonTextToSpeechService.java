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

package com.amazon.aacstts;

import static com.amazon.aacstts.AACSUtil.checkIfAACSIsConnected;
import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;
import static com.amazon.aacstts.TTSConstants.DEFAULT_PROVIDER;

import android.content.ComponentName;
import android.os.Bundle;
import android.os.Message;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.text.TextUtils;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSPinger;
import com.amazon.aacsipc.AACSReceiver;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.aacstts.handler.AASBHandler;
import com.amazon.aacstts.handler.AlexaClientHandler;
import com.amazon.aacstts.handler.TTSHandler;
import com.amazon.aacstts.models.GetCapabilitiesPayload;
import com.amazon.aacstts.models.PrepareSpeechMessageOptions;
import com.amazon.aacstts.models.PrepareSpeechMessagePayload;
import com.amazon.aacstts.models.ProviderVoiceItem;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Design Doc
 * https://wiki.labcollab.net/confluence/display/Doppler/%5BAlexa%5D+Android+Text+To+Speech
 */
public class AmazonTextToSpeechService extends TextToSpeechService {
    private static final String TAG = AACS_TTS_LOG_PREFIX + AmazonTextToSpeechService.class.getSimpleName();

    /**
     * The speech pitch can have following values in Android
     * 1) 0 - 99 for low pitch
     * 2) 100 for the baseline pitch.
     * 3) > 100 for higher pitches.
     */
    private static final Integer DEFAULT_PITCH = 100;

    /**
     * The speech rate can have following values in Android
     * 1) 0 - 99 for low speech rate.
     * 2) 100 for the baseline speech rate.
     * 3) > 100 for higher speech rates.
     */
    private static final Integer SPEECH_RATE_LOW = 20;
    private static final Integer SPEECH_RATE_HIGH = 200;

    private static final int NETWORK_DEFAULT_TIMEOUT_MS = 2000; // 2 sec

    private static final String SSML_FORMAT = "<speak><prosody rate='%d%%' pitch='%d%%'>%s</prosody></speak>";
    private static final String LOCALE_FORMAT = "%s-%s";

    private MessageHandler mMessageHandler;
    private AlexaClientHandler mAlexaClientHandler;
    private AASBHandler mAASBHandler;
    private TTSHandler mTTSHandler;
    public static AACSReceiver mAACSReceiver;
    private AACSSender mAACSSender;
    private AACSPinger mAACSPinger;
    protected static TargetComponent mTarget;
    private SynthesizeTextUtil mSynthesizeTextUtil;
    ExecutorService mExecutorService;
    private static final long GET_AACS_STATUS_DEFAULT_TIMEOUT = 1000;

    private CompletableFuture<Boolean> mIsAACSRunningPingResponse;

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        initializeIPC();
        initializeMessageHandlers();
        mExecutorService = Executors.newSingleThreadExecutor();
        mIsAACSRunningPingResponse = new CompletableFuture<>();
        super.onCreate();
        Log.i(TAG, "TTS package name = " + getPackageName());
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();

        if (mAACSReceiver != null) {
            mAACSReceiver.shutdown();
            mAACSReceiver = null;
        }
        if (mExecutorService != null) {
            mExecutorService.shutdown();
            mExecutorService = null;
        }

        if (mAACSPinger != null) {
            mAACSPinger.shutdown();
        }

        // Clean up all the message handlers
        mMessageHandler.cleanUp();
        mSynthesizeTextUtil.cleanUp();
    }

    @Override
    protected int onIsLanguageAvailable(String lang, String country, String variant) {
        Optional<String> iso2Language = ISO3CodeUtil.getISO2LangCode(lang);
        Optional<String> iso2Country = ISO3CodeUtil.getISO2CountryCode(country);

        if (!iso2Language.isPresent() || !iso2Country.isPresent()) {
            return TextToSpeech.LANG_MISSING_DATA;
        }

        String locale = String.format(LOCALE_FORMAT, iso2Language.get(), iso2Country.get());

        // Check if the mentioned language is available in the cache.
        Optional<ProviderVoiceItem> voiceItem = getVoiceItem(locale);
        if (voiceItem.isPresent()) {
            return TextToSpeech.LANG_COUNTRY_AVAILABLE;
        }

        return TextToSpeech.LANG_NOT_SUPPORTED;
    }

    @Override
    protected String[] onGetLanguage() {
        return new String[0];
    }

    @Override
    protected int onLoadLanguage(String lang, String country, String variant) {
        Log.d(TAG, "onLoadLanguage");

        // Load the capabilities of the TextToSpeech and
        // update the cache with the supported languages.
        GetCapabilitiesPayload getCapabilitiesPayload = new GetCapabilitiesPayload();
        getCapabilitiesPayload.setProvider(DEFAULT_PROVIDER);

        Optional<String> message = AACSMessageBuilder.buildMessage(
                TTSConstants.TOPIC, TTSConstants.Action.GET_CAPABILITIES, getCapabilitiesPayload.toJsonString());

        if (message.isPresent()) {
            sendMessageToAACS(message.get(), TTSConstants.Action.GET_CAPABILITIES, TTSConstants.TOPIC);
        }

        if (onIsLanguageAvailable(lang, country, variant) == TextToSpeech.LANG_COUNTRY_AVAILABLE) {
            return TextToSpeech.LANG_COUNTRY_AVAILABLE;
        }

        return TextToSpeech.LANG_MISSING_DATA;
    }

    @Override
    protected void onStop() {
        // stop streaming of any ongoing synthesis.
        mSynthesizeTextUtil.stopSynthesis();
    }

    @Override
    protected void onSynthesizeText(SynthesisRequest synthesisRequest, SynthesisCallback synthesisCallback) {
        CompletableFuture<Boolean> isAACSConnected =
                checkIfAACSIsConnected(mAlexaClientHandler.isAlexaClientConnected(), mIsAACSRunningPingResponse,
                        mExecutorService, mAACSPinger);

        if (!isAACSConnected.getNow(false)) {
            synthesisCallback.error(TextToSpeech.ERROR_NOT_INSTALLED_YET);
            Log.e(TAG, "onSynthesizeText: AACS was not connected");
            return;
        }

        Optional<PrepareSpeechMessagePayload> payload = constructPrepareSpeechPayload(synthesisRequest);

        if (!payload.isPresent()) {
            Log.w(TAG, "onSynthesizeText: The construction of payload for the prepare speech failed");
            synthesisCallback.error(TextToSpeech.ERROR_INVALID_REQUEST);
            // update the cache if the languages do not exist.
            onLoadLanguage(
                    synthesisRequest.getLanguage(), synthesisRequest.getCountry(), synthesisRequest.getVariant());
            return;
        }

        Bundle bundle = synthesisRequest.getParams();
        int networkTimeout =
                getNetworkTimeoutInMs(bundle.getString(TextToSpeech.Engine.KEY_FEATURE_NETWORK_TIMEOUT_MS));

        try {
            mSynthesizeTextUtil.sendPrepareSpeechMessage(networkTimeout, payload.get(), TTSConstants.TOPIC,
                    TTSConstants.Action.PREPARE_SPEECH, synthesisCallback);
        } catch (Exception e) {
            synthesisCallback.error(TextToSpeech.ERROR_SYNTHESIS);
            Log.e(TAG, "onSynthesizeText: Exception occurred while trying to synthezie the text", e);
        }
        synthesisCallback.done();
    }

    /**
     * Construct the {@link PrepareSpeechMessagePayload} to be included in the synthesize request.
     */
    private Optional<PrepareSpeechMessagePayload> constructPrepareSpeechPayload(SynthesisRequest synthesisRequest) {
        final String ssmlText = generateSSMLDocument(synthesisRequest);
        final String speechId = UUID.randomUUID().toString();

        Optional<String> iso2Language = ISO3CodeUtil.getISO2LangCode(synthesisRequest.getLanguage());
        Optional<String> iso2Country = ISO3CodeUtil.getISO2CountryCode(synthesisRequest.getCountry());

        String locale;
        if (iso2Language.isPresent() && iso2Country.isPresent()) {
            locale = String.format(LOCALE_FORMAT, iso2Language.get(), iso2Country.get());
        } else {
            String errorMsg = String.format("The language or the country was not available in ISO2 format for "
                            + "country: [%s], language: [%s] ",
                    synthesisRequest.getCountry(), synthesisRequest.getLanguage());
            Log.w(TAG, errorMsg);
            return Optional.empty();
        }

        PrepareSpeechMessageOptions options;

        Optional<ProviderVoiceItem> providerVoiceItemOptional = getVoiceItem(locale);
        if (providerVoiceItemOptional.isPresent()) {
            ProviderVoiceItem providerVoiceItem = providerVoiceItemOptional.get();

            options = new PrepareSpeechMessageOptions(locale, providerVoiceItem.getVoiceId());
        } else {
            String errorMsg = String.format("The requested locale [%s] is not present in the "
                            + "Capabilities for the provider [%s]. Capabilities cache : [%s] ",
                    locale, DEFAULT_PROVIDER, mTTSHandler.getCapabilitiesCache());
            Log.w(TAG, errorMsg);
            return Optional.empty();
        }

        PrepareSpeechMessagePayload payload =
                new PrepareSpeechMessagePayload(speechId, ssmlText, DEFAULT_PROVIDER, options);
        return Optional.of(payload);
    }

    private void initializeIPC() {
        mAACSSender = new AACSSender();
        mAACSPinger = new AACSPinger(getApplicationContext(), AACSConstants.AACS_PING_PERMISSION);

        AACSReceiver.MessageReceivedCallback aasbMessageReceivedCallback = (message) -> {
            Log.d(TAG, "onAASBMessageReceived from IPC: " + message);

            Bundle bundle = new Bundle();
            Message msg = Message.obtain();
            bundle.putString("message", message);
            msg.setData(bundle);
            mMessageHandler.sendMessage(msg);
        };

        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        mAACSReceiver = builder.withAASBCallback(aasbMessageReceivedCallback).build();

        mTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.AACS_PACKAGE_NAME, AACSConstants.AACS_CLASS_NAME),
                TargetComponent.Type.SERVICE);
        mSynthesizeTextUtil = new SynthesizeTextUtil(mAACSSender, mTarget, getApplicationContext());
    }

    private void initializeMessageHandlers() {
        if (mMessageHandler != null) {
            return;
        }
        mMessageHandler = new MessageHandler();
        mMessageHandler.register(Topic.ALEXA_CLIENT, mAlexaClientHandler = new AlexaClientHandler())
                .register(Topic.AASB, mAASBHandler = new AASBHandler())
                .register(TTSConstants.TOPIC, mTTSHandler = new TTSHandler(mSynthesizeTextUtil));
    }

    private void sendMessageToAACS(String message, String action, String topic) {
        CompletableFuture<Boolean> isAACSConnected =
                checkIfAACSIsConnected(mAlexaClientHandler.isAlexaClientConnected(), mIsAACSRunningPingResponse,
                        mExecutorService, mAACSPinger);
        mExecutorService.submit(() -> {
            try {
                Boolean isConnected = isAACSConnected.get(GET_AACS_STATUS_DEFAULT_TIMEOUT, TimeUnit.MILLISECONDS);
                if (isConnected) {
                    mAACSSender.sendAASBMessageAnySize(message, action, topic, mTarget, getApplicationContext());
                    Log.d(TAG,
                            String.format(
                                    "AACS was connected, message of action=%s, topic=%s was sent.", action, topic));
                } else {
                    Log.w(TAG,
                            String.format(
                                    "AACS was not in CONNECTED state. Message of topic=%s, action=%s was not sent.",
                                    topic, action));
                }
            } catch (InterruptedException | ExecutionException | TimeoutException e) {
                Log.e(TAG,
                        String.format("failed to get isConnected future because of exception encountered: %s. "
                                        + "Message of topic=%s, action=%s was not sent.",
                                e.getMessage(), topic, action));
            }
        });
    }

    private Optional<ProviderVoiceItem> getVoiceItem(String locale) {
        Map<String, List<ProviderVoiceItem>> capabilitiesCache = mTTSHandler.getCapabilitiesCache();

        if (capabilitiesCache.containsKey(DEFAULT_PROVIDER)) {
            List<ProviderVoiceItem> availableVoiceItems = capabilitiesCache.get(DEFAULT_PROVIDER);
            for (ProviderVoiceItem voiceItem : availableVoiceItems) {
                if (voiceItem.getSupportedLocales().contains(locale)) {
                    return Optional.of(voiceItem);
                }
            }
        }
        return Optional.empty();
    }

    private String generateSSMLDocument(final SynthesisRequest synthesisRequest) {
        final String speechText = synthesisRequest.getCharSequenceText().toString();
        final Integer ssmlSpeechRate = getSpeechRateForSSML(synthesisRequest.getSpeechRate());
        final Integer ssmlPitch = getPitchForSSML(synthesisRequest.getPitch());
        return String.format(SSML_FORMAT, ssmlSpeechRate, ssmlPitch, speechText);
    }

    /**
     * The speech pitch in Android can have following values
     * 1) 0 - 99 for low pitch
     * 2) 100 for the baseline pitch.
     * 3) > 100 for higher pitches.
     *
     * They need to be adjusted to +n% or -n% for SSML tags
     * Ref: https://docs.aws.amazon.com/polly/latest/dg/supportedtags.html#prosody-tag
     */
    private int getPitchForSSML(final Integer pitch) {
        if (pitch < DEFAULT_PITCH) {
            return -1 * pitch;
        }
        return pitch - DEFAULT_PITCH;
    }

    /**
     * The speech rate in Android can have following values
     * 1) 0 - 99 for low speech rate
     * 2) 100 for the baseline speech rate.
     * 3) > 100 for higher speech rates.
     *
     * @return the speech rate will be a value between 20 and 200.
     * Ref: https://docs.aws.amazon.com/polly/latest/dg/supportedtags.html#prosody-tag
     */
    private int getSpeechRateForSSML(final Integer rate) {
        if (rate < SPEECH_RATE_LOW) {
            return SPEECH_RATE_LOW;
        } else if (rate > SPEECH_RATE_HIGH) {
            return SPEECH_RATE_HIGH;
        }
        return rate;
    }

    private int getNetworkTimeoutInMs(String networkTimeOut) {
        if (!TextUtils.isEmpty(networkTimeOut)) {
            try {
                return Integer.parseInt(networkTimeOut);
            } catch (NumberFormatException ex) {
                Log.i(TAG,
                        "Exception occurred while trying to derive netweork timeout. "
                                + "Falling back to Default timeout : " + NETWORK_DEFAULT_TIMEOUT_MS + "(ms)");
            }
        }
        return NETWORK_DEFAULT_TIMEOUT_MS;
    }
}
