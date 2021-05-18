package com.amazon.aacstts;

import static org.mockito.Matchers.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.speech.tts.TextToSpeech;

import com.amazon.aacstts.handler.AlexaClientHandler;
import com.amazon.aacstts.handler.TTSHandler;
import com.amazon.aacstts.models.ProviderVoiceItem;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;

/**
 * Class to test Alexa Text To Speech Service methods.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(AACSUtil.class)
public class AmazonTextToSpeechServiceTests {
    private final static String LOCALE_EN_US = "en-US";
    private final static String TEST_PROVIDER = TTSConstants.DEFAULT_PROVIDER;
    private final static String TEST_VOICE_ID = "testVoiceId";

    @InjectMocks
    private AmazonTextToSpeechService mAmazonTextToSpeechService;

    @Mock
    private TTSHandler mockTTSHandler;
    @Mock
    private AlexaClientHandler mockAlexaClientHandler;
    @Mock
    private ExecutorService mockExecutorService;

    @Before
    public void setup() {
        ConcurrentHashMap<String, List<ProviderVoiceItem>> capabilities = setupCapabilities();
        when(mockTTSHandler.getCapabilitiesCache()).thenReturn(capabilities);
        when(mockAlexaClientHandler.isAlexaClientConnected()).thenReturn(Optional.of(true));
        when(mockExecutorService.submit((Runnable) any())).thenReturn(new CompletableFuture<>());

        PowerMockito.mockStatic(AACSUtil.class);
        when(AACSUtil.checkIfAACSIsConnected(any(), any(), any(), any()))
                .thenReturn(CompletableFuture.completedFuture(Boolean.TRUE));
    }

    /**
     * @see AmazonTextToSpeechService#onIsLanguageAvailable(String, String, String)
     */
    @Test
    public void onIsLanguageAvailable_validISO3LanguageAndCountry_returnsLanguageAvailable() {
        int languageStatus = mAmazonTextToSpeechService.onIsLanguageAvailable("eng", "USA", "");

        Assert.assertEquals(TextToSpeech.LANG_COUNTRY_AVAILABLE, languageStatus);
    }

    /**
     * @see AmazonTextToSpeechService#onIsLanguageAvailable(String, String, String)
     */
    @Test
    public void onIsLanguageAvailable_emptyLanguagePassed_returnsLanguageMissing() {
        int languageStatus = mAmazonTextToSpeechService.onIsLanguageAvailable("", "USA", "");

        Assert.assertEquals(TextToSpeech.LANG_MISSING_DATA, languageStatus);
    }

    /**
     * @see AmazonTextToSpeechService#onIsLanguageAvailable(String, String, String)
     */
    @Test
    public void onIsLanguageAvailable_emptyCountryPassed_returnsLanguageMissing() {
        int languageStatus = mAmazonTextToSpeechService.onIsLanguageAvailable("eng", "", "");

        Assert.assertEquals(TextToSpeech.LANG_MISSING_DATA, languageStatus);
    }

    /**
     * @see AmazonTextToSpeechService#onIsLanguageAvailable(String, String, String)
     */
    @Test
    public void onIsLanguageAvailable_languageNotInCapabilitiesCache_returnsLanguageNotSupported() {
        int languageStatus = mAmazonTextToSpeechService.onIsLanguageAvailable("eng", "ESP", "");

        Assert.assertEquals(TextToSpeech.LANG_NOT_SUPPORTED, languageStatus);
    }

    /**
     * @see AmazonTextToSpeechService#onLoadLanguage(String, String, String)
     */
    @Test
    public void onLoadLanguage_theLanguageNeedsIsInCache_callsGetCapabilitiesToWarmTheCacheAndReturnsLanguagePresent() {
        int languageStatus = mAmazonTextToSpeechService.onLoadLanguage("eng", "USA", "");

        verify(mockExecutorService, times(1)).submit((Runnable) any());
        Assert.assertEquals(TextToSpeech.LANG_COUNTRY_AVAILABLE, languageStatus);
    }

    /**
     * @see AmazonTextToSpeechService#onLoadLanguage(String, String, String)
     */
    @Test
    public void
    onLoadLanguage_theLanguageNeedsIsNotInCache_callsGetCapabilitiesToWarmTheCacheAndReturnsLanguageMissing() {
        int languageStatus = mAmazonTextToSpeechService.onLoadLanguage("eng", "ESP", "");

        verify(mockExecutorService, times(1)).submit((Runnable) any());
        Assert.assertEquals(TextToSpeech.LANG_MISSING_DATA, languageStatus);
    }

    private ConcurrentHashMap<String, List<ProviderVoiceItem>> setupCapabilities() {
        List<String> supportedLocales = new ArrayList<>();
        supportedLocales.add(LOCALE_EN_US);
        ProviderVoiceItem testProviderVoiceItem = new ProviderVoiceItem(TEST_VOICE_ID, supportedLocales);

        ConcurrentHashMap<String, List<ProviderVoiceItem>> capabilities = new ConcurrentHashMap<>();
        List<ProviderVoiceItem> voiceItems = new ArrayList<>();
        voiceItems.add(testProviderVoiceItem);
        capabilities.put(TEST_PROVIDER, voiceItems);

        return capabilities;
    }
}
