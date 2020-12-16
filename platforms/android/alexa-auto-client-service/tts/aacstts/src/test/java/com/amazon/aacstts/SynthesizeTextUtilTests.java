package com.amazon.aacstts;

import static com.amazon.aacsipc.AACSSender.StreamFetchedFromReceiverCallback;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.anyString;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.TextToSpeech;

import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.aacstts.models.GetCapabilitiesPayload;
import com.amazon.aacstts.models.PrepareSpeechMessageOptions;
import com.amazon.aacstts.models.PrepareSpeechMessagePayload;
import com.amazon.aacstts.models.TTSSynthesisFutureResponse;

import junit.framework.Assert;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Class to unit test the {@link SynthesizeTextUtil} class
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(SynthesizeTextUtil.class)
public class SynthesizeTextUtilTests {
    private static final String TEST_VOICE_ID = "OEM_VOICE";
    private static final String TEST_LOCALE = "en-US";
    private static final int TEST_NETWORK_TIMEOUT = 100;
    private static final String TEST_TOPIC = TTSConstants.TOPIC;
    private static final String TEST_AACS_ACTION = TTSConstants.Action.PREPARE_SPEECH;
    private static final String TEST_SPEECH_ID = "testSpeechId";
    private static final String TEST_STREAM_ID = "testStreamId";
    private static final String TEST_AUDIO_FOMAT_MP3 = "MP3";
    private static final String TEST_TEXT = "text";
    private static final String TEST_PROVIDER = "textProvider";

    @Mock
    private AACSSender mockAACSSender;
    @Mock
    private TargetComponent mockTargetComponent;
    @Mock
    private Context mockContext;
    @Mock
    private SynthesisCallback mockCallback;
    @Mock
    private CompletableFuture<Object> mockWaitForReplyFuture;
    @Mock
    private TTSSynthesisFutureResponse mockTTSSynthesisFutureResponse;
    @Mock
    private CompletableFuture<Boolean> mockAASBMessageResponse;
    @Mock
    private Object mockWaitingCompletableFutureResponse;

    @Captor
    ArgumentCaptor<String> capabilitiesRequestPayloadCaptor;

    @InjectMocks
    SynthesizeTextUtil mSynthesizeTextUtil;

    @Before
    public void setup() throws Exception {
        GetCapabilitiesPayload testGetCapabilities = new GetCapabilitiesPayload();

        PowerMockito.whenNew(CompletableFuture.class).withNoArguments().thenReturn(mockWaitForReplyFuture);
        PowerMockito.whenNew(TTSSynthesisFutureResponse.class)
                .withAnyArguments()
                .thenReturn(mockTTSSynthesisFutureResponse);
        PowerMockito.whenNew(GetCapabilitiesPayload.class).withNoArguments().thenReturn(testGetCapabilities);

        when(mockTTSSynthesisFutureResponse.getAACSSender()).thenReturn(mockAACSSender);
        when(mockTTSSynthesisFutureResponse.getTargetComponent()).thenReturn(mockTargetComponent);
        when(mockTTSSynthesisFutureResponse.getContext()).thenReturn(mockContext);
        when(mockTTSSynthesisFutureResponse.getSynthesisCallback()).thenReturn(mockCallback);
        mSynthesizeTextUtil.mIsStreamingEnabled = true;
    }

    /**
     * @see SynthesizeTextUtil#sendPrepareSpeechMessage(int, PrepareSpeechMessagePayload, String, String,
     *         SynthesisCallback)
     */
    @Test
    public void sendPrepareSpeechMessage_validArguments_doesNotReportSynthesisError() throws Exception {
        PrepareSpeechMessagePayload payload = constructPrepareSpeechPayload();

        when(mockAACSSender.sendAASBMessageAnySize(
                     payload.toJsonString(), TEST_AACS_ACTION, TEST_TOPIC, mockTargetComponent, mockContext))
                .thenReturn(mockAASBMessageResponse);

        when(mockWaitForReplyFuture.get(TEST_NETWORK_TIMEOUT, TimeUnit.MILLISECONDS))
                .thenReturn(mockWaitingCompletableFutureResponse);
        when(mockWaitForReplyFuture.isDone()).thenReturn(true);

        mSynthesizeTextUtil.sendPrepareSpeechMessage(
                TEST_NETWORK_TIMEOUT, payload, TEST_TOPIC, TEST_AACS_ACTION, mockCallback);

        verify(mockCallback, times(0)).error(TextToSpeech.ERROR_SYNTHESIS);
        verify(mockCallback, times(0)).error(TextToSpeech.ERROR_NETWORK_TIMEOUT);
    }

    /**
     * @see SynthesizeTextUtil#sendPrepareSpeechMessage(int, PrepareSpeechMessagePayload, String, String,
     *         SynthesisCallback)
     */
    @Test
    public void sendPrepareSpeechMessage_networkTimeout_reportsTimeout() throws Exception {
        PrepareSpeechMessagePayload payload = constructPrepareSpeechPayload();

        when(mockAACSSender.sendAASBMessageAnySize(
                     payload.toJsonString(), TEST_AACS_ACTION, TEST_TOPIC, mockTargetComponent, mockContext))
                .thenReturn(mockAASBMessageResponse);

        when(mockWaitForReplyFuture.get(TEST_NETWORK_TIMEOUT, TimeUnit.MILLISECONDS)).thenThrow(TimeoutException.class);

        mSynthesizeTextUtil.sendPrepareSpeechMessage(
                TEST_NETWORK_TIMEOUT, payload, TEST_TOPIC, TEST_AACS_ACTION, mockCallback);

        verify(mockCallback, times(1)).error(TextToSpeech.ERROR_NETWORK_TIMEOUT);
    }

    /**
     * @see SynthesizeTextUtil#sendPrepareSpeechMessage(int, PrepareSpeechMessagePayload, String, String,
     *         SynthesisCallback)
     */
    @Test
    public void sendPrepareSpeechMessage_cancellationExceptionOccurs_reportsSynthesisError() throws Exception {
        PrepareSpeechMessagePayload payload = constructPrepareSpeechPayload();

        when(mockAACSSender.sendAASBMessageAnySize(
                     payload.toJsonString(), TEST_AACS_ACTION, TEST_TOPIC, mockTargetComponent, mockContext))
                .thenReturn(mockAASBMessageResponse);

        when(mockWaitForReplyFuture.get(TEST_NETWORK_TIMEOUT, TimeUnit.MILLISECONDS))
                .thenThrow(CancellationException.class);

        mSynthesizeTextUtil.sendPrepareSpeechMessage(
                TEST_NETWORK_TIMEOUT, payload, TEST_TOPIC, TEST_AACS_ACTION, mockCallback);

        verify(mockCallback, times(1)).error(TextToSpeech.ERROR_SYNTHESIS);
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void sendPrepareSpeechMessage_nullPayload_doesNotReportSynthesisError() {
        PrepareSpeechMessagePayload payload = null;

        mSynthesizeTextUtil.sendPrepareSpeechMessage(
                TEST_NETWORK_TIMEOUT, payload, TEST_TOPIC, TEST_AACS_ACTION, mockCallback);

        verify(mockAACSSender, times(0))
                .sendAASBMessageAnySize(
                        anyString(), anyString(), anyString(), any(TargetComponent.class), any(Context.class));
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void handlePrepareSpeechResponse_speechIdDoesNotExists_doesNotSendStreamToSynthesisCallback()
            throws Exception {
        JSONObject jsonObject = constructPrepareSpeechResponsePayload();
        jsonObject.remove(SynthesizeTextUtil.PAYLOAD_SPEECH_ID_KEY);

        mSynthesizeTextUtil.handlePrepareSpeechResponse(jsonObject.toString());

        verify(mockAACSSender, times(0))
                .fetch(anyString(), any(StreamFetchedFromReceiverCallback.class), any(TargetComponent.class),
                        any(Context.class));
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void handlePrepareSpeechResponse_theRequestHadTimedOut_cancelsFetchingOfTheStream() throws Exception {
        JSONObject jsonObject = constructPrepareSpeechResponsePayload();

        mSynthesizeTextUtil.mMessageIdToFutureMap.clear();

        mSynthesizeTextUtil.handlePrepareSpeechResponse(jsonObject.toString());

        verify(mockAACSSender, times(1)).cancelFetch(TEST_STREAM_ID, mockTargetComponent, mockContext);
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void handlePrepareSpeechResponse_errorResponseReceived_cancelsTheFutureAndTriggersGetCapabilitiesCall()
            throws Exception {
        JSONObject jsonObject = constructPrepareSpeechResponsePayload();
        jsonObject.remove(SynthesizeTextUtil.PAYLOAD_STREAM_ID_KEY);

        mSynthesizeTextUtil.mMessageIdToFutureMap.put(TEST_SPEECH_ID, mockTTSSynthesisFutureResponse);

        mSynthesizeTextUtil.handlePrepareSpeechResponse(jsonObject.toString());

        verify(mockTTSSynthesisFutureResponse, times(1))
                .cancelFuture(TTSSynthesisFutureResponse.FutureType.WAIT_FOR_REPLY);
        verify(mockAACSSender, times(1))
                .sendAASBMessageAnySize(capabilitiesRequestPayloadCaptor.capture(),
                        eq(TTSConstants.Action.GET_CAPABILITIES), eq(TTSConstants.TOPIC), eq(mockTargetComponent),
                        eq(mockContext));

        String capabilitiesPayload = capabilitiesRequestPayloadCaptor.getValue();
        JSONObject capabilitiesPayloadObject = new JSONObject(capabilitiesPayload);
        Assert.assertEquals(
                TTSConstants.DEFAULT_PROVIDER, capabilitiesPayloadObject.getJSONObject("payload").get("provider"));
        Assert.assertEquals(TTSConstants.TOPIC,
                capabilitiesPayloadObject.getJSONObject("header").getJSONObject("messageDescription").get("topic"));
        Assert.assertEquals(TTSConstants.Action.GET_CAPABILITIES,
                capabilitiesPayloadObject.getJSONObject("header").getJSONObject("messageDescription").get("action"));
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void handlePrepareSpeechResponse_streamingIsDisabled_doesNotSendStreamToSynthesisCallback()
            throws Exception {
        mSynthesizeTextUtil.mIsStreamingEnabled = false;
        mSynthesizeTextUtil.mMessageIdToFutureMap.put(TEST_SPEECH_ID, mockTTSSynthesisFutureResponse);
        JSONObject jsonObject = constructPrepareSpeechResponsePayload();

        mSynthesizeTextUtil.handlePrepareSpeechResponse(jsonObject.toString());

        verify(mockAACSSender, times(1)).cancelFetch(TEST_STREAM_ID, mockTargetComponent, mockContext);

        verify(mockAACSSender, times(0))
                .fetch(anyString(), any(StreamFetchedFromReceiverCallback.class), any(TargetComponent.class),
                        any(Context.class));
    }

    /**
     * @see SynthesizeTextUtil#handlePrepareSpeechResponse(String)
     */
    @Test
    public void handlePrepareSpeechResponse_returnsSuccessfulPayload_sendsStreamToSynthesisCallback() throws Exception {
        mSynthesizeTextUtil.mIsStreamingEnabled = true;
        mSynthesizeTextUtil.mMessageIdToFutureMap.put(TEST_SPEECH_ID, mockTTSSynthesisFutureResponse);
        JSONObject jsonObject = constructPrepareSpeechResponsePayload();

        mSynthesizeTextUtil.handlePrepareSpeechResponse(jsonObject.toString());

        verify(mockAACSSender, times(1))
                .fetch(anyString(), any(StreamFetchedFromReceiverCallback.class), any(TargetComponent.class),
                        any(Context.class));

        verify(mockTTSSynthesisFutureResponse, times(1))
                .completeFuture(TTSSynthesisFutureResponse.FutureType.WAIT_FOR_REPLY);
    }

    private PrepareSpeechMessagePayload constructPrepareSpeechPayload() {
        PrepareSpeechMessageOptions options = new PrepareSpeechMessageOptions(TEST_LOCALE, TEST_VOICE_ID);

        return new PrepareSpeechMessagePayload(TEST_SPEECH_ID, TEST_TEXT, TEST_PROVIDER, options);
    }

    private JSONObject constructPrepareSpeechResponsePayload() throws JSONException {
        JSONObject jsonObject = new JSONObject();

        jsonObject.put(SynthesizeTextUtil.PAYLOAD_SPEECH_ID_KEY, TEST_SPEECH_ID);
        jsonObject.put(SynthesizeTextUtil.PAYLOAD_STREAM_ID_KEY, TEST_STREAM_ID);
        jsonObject.put(SynthesizeTextUtil.PAYLOAD_ENCODING_KEY, TEST_AUDIO_FOMAT_MP3);

        return jsonObject;
    }
}
