package com.amazon.aacstts;

import com.amazon.aacstts.handler.TTSHandler;
import com.amazon.aacstts.models.ProviderVoiceItem;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Unit tests for {@link TTSHandler}
 */
@RunWith(MockitoJUnitRunner.class)
public class TTSHandlerTests {
    private static String TEST_VOICE_ID_OEM = "oem_voice";
    private static String TEST_VOICE_ID_ALEXA = "alexa";
    private static String TEST_LOCALE_EN_US = "en-US";
    private static String TEST_PROVIDER_ID = "testProvideId";

    private static String TEST_PREPARE_SPEECH_PAYLOAD = "{}";

    private static final String TEST_AACS_MESSAGE_ID = "testMessageId";
    private static final String TEST_AACS_TOPIC = "testAACSTopic";

    private TTSHandler mTTSHandler;

    @Mock
    private SynthesizeTextUtil mSynthesizeTextUtil;

    @Before
    public void setup() {
        mTTSHandler = new TTSHandler(mSynthesizeTextUtil);
        Mockito.doNothing().when(mSynthesizeTextUtil).handlePrepareSpeechResponse(Mockito.anyString());
    }

    /**
     * @see  TTSHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_receivedGetCapabilitiesMessage_setsTheCapabilitiesCache() throws Exception {
        mTTSHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TTSConstants.Action.GET_CAPABILITIES,
                getTtsCapabilitiesPayload(TEST_LOCALE_EN_US, TEST_VOICE_ID_OEM, TEST_PROVIDER_ID));

        ConcurrentHashMap<String, List<ProviderVoiceItem>> capabilities = mTTSHandler.getCapabilitiesCache();

        Assert.assertTrue(capabilities.containsKey(TEST_PROVIDER_ID));

        List<ProviderVoiceItem> providerVoiceItems = capabilities.get(TEST_PROVIDER_ID);
        Assert.assertEquals(1, providerVoiceItems.size());
        Assert.assertEquals(TEST_VOICE_ID_OEM, providerVoiceItems.get(0).getVoiceId());
        Assert.assertEquals(1, providerVoiceItems.get(0).getSupportedLocales().size());
        List<String> supportedLocales = providerVoiceItems.get(0).getSupportedLocales();
        Assert.assertEquals(TEST_LOCALE_EN_US, supportedLocales.get(0));
    }

    /**
     * @see  TTSHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_receivedGetCapabilitiesMessageWithAlexaVoice_doesNotSetAlexaVoiceInCapabilitiesCache()
            throws Exception {
        mTTSHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TTSConstants.Action.GET_CAPABILITIES,
                getTtsCapabilitiesPayload(TEST_LOCALE_EN_US, TTSHandler.ALEXA_VOICE_ID, TEST_PROVIDER_ID));

        ConcurrentHashMap<String, List<ProviderVoiceItem>> capabilities = mTTSHandler.getCapabilitiesCache();

        Assert.assertTrue(capabilities.containsKey(TEST_PROVIDER_ID));
        List<ProviderVoiceItem> providerVoiceItems = capabilities.get(TEST_PROVIDER_ID);
        Assert.assertEquals(0, providerVoiceItems.size());
    }

    /**
     * @see  TTSHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_receivedEmptyPayload_doesNotSetTheCapabilitiesCache() throws Exception {
        mTTSHandler.handleAACSMessage(
                TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TTSConstants.Action.GET_CAPABILITIES, "{}");

        ConcurrentHashMap<String, List<ProviderVoiceItem>> capabilities = mTTSHandler.getCapabilitiesCache();

        Assert.assertFalse(capabilities.containsKey(TEST_PROVIDER_ID));
    }

    /**
     * @see  TTSHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handlePrepareSpeech_receivedValidPayload_callsHandlePrepareSpeechResponse() {
        mTTSHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC,
                TTSConstants.Action.PREPARE_SPEECH_COMPLETED, TEST_PREPARE_SPEECH_PAYLOAD);

        Mockito.verify(mSynthesizeTextUtil, Mockito.times(1)).handlePrepareSpeechResponse(TEST_PREPARE_SPEECH_PAYLOAD);
    }

    /**
     * @see  TTSHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handlePrepareSpeechFailed_receivedValidPayload_callsHandlePrepareSpeechResponse() {
        mTTSHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TTSConstants.Action.PREPARE_SPEECH_FAILED,
                TEST_PREPARE_SPEECH_PAYLOAD);

        Mockito.verify(mSynthesizeTextUtil, Mockito.times(1)).handlePrepareSpeechResponse(TEST_PREPARE_SPEECH_PAYLOAD);
    }

    private String getTtsCapabilitiesPayload(String locale, String voiceId, String providerId) throws JSONException {
        JSONObject ttsCapabilitiesObject;

        JSONObject voiceObject = new JSONObject();

        JSONArray locales = new JSONArray();
        locales.put(locale);

        voiceObject.put("locales", locales);
        voiceObject.put("voiceId", voiceId);

        JSONArray voices = new JSONArray();
        voices.put(voiceObject);

        JSONObject voicesObject = new JSONObject();
        voicesObject.put("voices", voices);

        JSONObject provider = new JSONObject();
        provider.put(providerId, voicesObject);

        ttsCapabilitiesObject = new JSONObject();
        ttsCapabilitiesObject.put("capabilities", provider.toString());

        return ttsCapabilitiesObject.toString();
    }
}