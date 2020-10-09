package com.amazon.alexaautoclientservice.instrumentedtest;

import static junit.framework.Assert.assertFalse;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.media.MediaRecorder;
import android.os.Environment;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import com.amazon.aace.core.Engine;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONObject;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import org.junit.runner.RunWith;
import org.mockito.Mockito;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

@RunWith(AndroidJUnit4.class)
public class TestFileUtil {
    Context mContext;
    Context mTestContext;
    Engine mEngine;

    @Rule
    public TemporaryFolder mTempFolder = new TemporaryFolder();

    @Before
    public void setup() {
        mContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        mTestContext = InstrumentationRegistry.getInstrumentation().getContext();
        mEngine = Engine.create(mContext);
    }

    @After
    public void cleanup() {
        mEngine.stop();
        mEngine.dispose();
    }

    private void prepareAndSaveConfiguration(JSONObject configJson) throws Exception {
        TestUtil.addDeviceInfoToConfig(configJson, TestUtil.CLIENT_ID, TestUtil.PRODUCT_ID, TestUtil.DSN);
        JSONObject configMessage = TestUtil.constructOEMConfigMessage(
                new String[] {}, new String[] {TestUtil.escapeJsonString(configJson)});
        FileUtil.saveConfiguration(
                mContext, configMessage.getJSONArray("configFilepaths"), configMessage.getJSONArray("configStrings"));
    }

    @Test
    public void testSaveConfigurationAndStartEngine() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        ArrayList<EngineConfiguration> engineConfigurations = FileUtil.getEngineConfiguration(mContext);
        EngineConfiguration[] configurationArray =
                engineConfigurations.toArray(new EngineConfiguration[engineConfigurations.size()]);

        assertTrue(mEngine.configure(configurationArray));
        assertTrue(mEngine.start());
    }

    @Test
    public void testMultipleConfigurationOptions() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        TestUtil.addDeviceInfoToConfig(configJson, TestUtil.CLIENT_ID, TestUtil.PRODUCT_ID, TestUtil.DSN);

        JSONObject config1 = new JSONObject().put("aacs.alexa", (JSONObject) configJson.remove("aacs.alexa"));
        JSONObject config2 = new JSONObject().put("aacs.vehicle", (JSONObject) configJson.remove("aacs.vehicle"));

        String[] configStrings = new String[] {TestUtil.escapeJsonString(config1), TestUtil.escapeJsonString(config2),
                TestUtil.escapeJsonString(configJson)};

        JSONObject configMessage = TestUtil.constructOEMConfigMessage(new String[] {}, configStrings);

        FileUtil.saveConfiguration(
                mContext, configMessage.getJSONArray("configFilepaths"), configMessage.getJSONArray("configStrings"));
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        ArrayList<EngineConfiguration> engineConfigurations = FileUtil.getEngineConfiguration(mContext);
        EngineConfiguration[] configurationArray =
                engineConfigurations.toArray(new EngineConfiguration[engineConfigurations.size()]);

        assertTrue(mEngine.configure(configurationArray));
        assertTrue(mEngine.start());
    }

    @Test
    public void testAutoSDKConfiguration() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.ENGINE_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        ArrayList<EngineConfiguration> engineConfigurations = FileUtil.getEngineConfiguration(mContext);
        EngineConfiguration[] configurationArray =
                engineConfigurations.toArray(new EngineConfiguration[engineConfigurations.size()]);

        assertTrue(mEngine.configure(configurationArray));
        assertTrue(mEngine.start());
    }

    @Test
    public void testIsPersistentService() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(!FileUtil.isPersistentSystemService(mContext));
    }

    @Test
    public void testIsStartServiceOnBootEnabled() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.isStartServiceOnBootEnabled(mContext));
    }

    @Test
    public void testIsDefaultImplementationEnabled() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        JSONObject defaultHandlersJson = configJson.getJSONObject("aacs.defaultPlatformHandlers");

        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        final String AACS_CONFIG_NETWORK_INFO_PROVIDER = "NetworkInfoProvider";
        final String AACS_CONFIG_LOCATION_PROVIDER = "LocationProvider";
        final String AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER = "ExternalMediaAdapter";

        assertTrue(FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_NETWORK_INFO_PROVIDER));
        assertTrue(FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_LOCATION_PROVIDER));
        assertTrue(FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER));

        // Test the case when the defaultPlatformHandler is disabled or the field is not present in configuration
        defaultHandlersJson.put("useDefault" + AACS_CONFIG_NETWORK_INFO_PROVIDER, false);
        defaultHandlersJson.remove("useDefault" + AACS_CONFIG_LOCATION_PROVIDER);
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(!FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_NETWORK_INFO_PROVIDER));
        assertTrue(!FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_LOCATION_PROVIDER));

        // Test the case when the value is not a boolean type
        defaultHandlersJson.put("useDefault" + AACS_CONFIG_NETWORK_INFO_PROVIDER, 1);
        defaultHandlersJson.put("useDefault" + AACS_CONFIG_LOCATION_PROVIDER, "true");
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(!FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_NETWORK_INFO_PROVIDER));
        assertTrue(!FileUtil.isDefaultImplementationEnabled(mContext, AACS_CONFIG_LOCATION_PROVIDER));
    }

    @Test
    public void testIsAudioTypeEnabled() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.isAudioInputTypeEnabled(mContext, AASBConstants.AudioInput.AudioType.VOICE));
        assertTrue(!FileUtil.isAudioInputTypeEnabled(mContext, AASBConstants.AudioInput.AudioType.COMMUNICATION));
        assertTrue(FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.ALARM));
        assertTrue(FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.EARCON));
        assertTrue(FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.NOTIFICATION));
        assertTrue(FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.RINGTONE));
        assertTrue(FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.TTS));
        assertTrue(!FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.COMMUNICATION));
        assertTrue(!FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.MUSIC));

        // Test when useDefault is set to false or is set to an invalid value
        JSONObject audioInputTypeForVoiceJson = configJson.getJSONObject("aacs.defaultPlatformHandlers")
                                                        .getJSONObject("audioInput")
                                                        .getJSONObject("audioType")
                                                        .getJSONObject(AASBConstants.AudioInput.AudioType.VOICE);
        audioInputTypeForVoiceJson.put("useDefault", false);
        JSONObject audioOutputTypeForTTSJson = configJson.getJSONObject("aacs.defaultPlatformHandlers")
                                                       .getJSONObject("audioOutput")
                                                       .getJSONObject("audioType")
                                                       .getJSONObject(AASBConstants.AudioOutput.AudioType.TTS);
        audioOutputTypeForTTSJson.put("useDefault", "true");
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(!FileUtil.isAudioInputTypeEnabled(mContext, AASBConstants.AudioInput.AudioType.VOICE));
        assertTrue(!FileUtil.isAudioOutputTypeEnabled(mContext, AASBConstants.AudioOutput.AudioType.TTS));
    }

    @Test
    public void testGetAudioSourceForAudioType() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.getAudioSourceForAudioType(mContext, AASBConstants.AudioInput.AudioType.VOICE)
                == MediaRecorder.AudioSource.MIC);
        // Test the default value when it fails to retrieve the audio source for a given type
        assertTrue(FileUtil.getAudioSourceForAudioType(mContext, AASBConstants.AudioInput.AudioType.COMMUNICATION)
                == MediaRecorder.AudioSource.MIC);

        // Test when audio source is set to VOICE_RECOGNITION
        JSONObject audioInputTypeForVoiceJson = configJson.getJSONObject("aacs.defaultPlatformHandlers")
                                                        .getJSONObject("audioInput")
                                                        .getJSONObject("audioType")
                                                        .getJSONObject(AASBConstants.AudioInput.AudioType.VOICE);
        audioInputTypeForVoiceJson.put("audioSource", "MediaRecorder.AudioSource.VOICE_RECOGNITION");
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.getAudioSourceForAudioType(mContext, AASBConstants.AudioInput.AudioType.VOICE)
                == MediaRecorder.AudioSource.VOICE_RECOGNITION);
    }

    @Test
    public void testGetLeafNodeValueFromJson() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.getLeafNodeValueFromJson(
                                   configJson, "aacs.defaultPlatformHandlers", "useDefaultLocationProvider")
                           .equals(true));
        assertTrue(FileUtil.getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "audioOutput",
                                   "audioType", "MUSIC", "useDefault")
                           .equals(false));
        assertTrue(FileUtil.getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "audioInput",
                                   "audioType", "VOICE", "audioSource")
                           .equals("MediaRecorder.AudioSource.MIC"));

        // Test when the leaf node is not a JSONObject
        JSONObject expectedJsonObj = new JSONObject();
        expectedJsonObj.put("useDefault", false);
        String expectedJsonString = expectedJsonObj.toString();
        String actualJsonString = FileUtil.getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers",
                                                  "audioOutput", "audioType", "MUSIC")
                                          .toString();
        assertTrue(expectedJsonString.equals(actualJsonString));

        // Test when the given node does not exist in the JSON object
        assertTrue(FileUtil.getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "audioOutput",
                           "audioType", "COMMUNICATION", "useDefault")
                == null);
        assertTrue(FileUtil.getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "node") == null);

        // Test when jsonObjectTree is not present in the arguments or the root json object is null
        assertTrue(FileUtil.getLeafNodeValueFromJson(configJson).equals(configJson));
        assertTrue(FileUtil.getLeafNodeValueFromJson(null, "aacs.defaultPlatformHandlers", "useDefaultLocationProvider")
                == null);
    }

    @Test
    public void testGetIntentTargets() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        // Test getting the value with a given topic and target
        assertEquals("RECEIVER", FileUtil.getIntentTargets(mContext, "CBL", "type").get(0));
        assertEquals("com.amazon.aacssampleapp1", FileUtil.getIntentTargets(mContext, "CBL", "package").get(0));
        assertEquals(".IntentReceiver", FileUtil.getIntentTargets(mContext, "CBL", "class").get(0));
        assertEquals("RECEIVER", FileUtil.getIntentTargets(mContext, "CBL", "type").get(1));
        assertEquals("com.amazon.aacssampleapp2", FileUtil.getIntentTargets(mContext, "CBL", "package").get(1));
        assertEquals(".IntentReceiver", FileUtil.getIntentTargets(mContext, "CBL", "class").get(1));

        // Test when the topic doesn't exist in the config
        assertNull(FileUtil.getIntentTargets(mContext, "ABC", "type"));

        // Test when the info field is not present
        assertNull(FileUtil.getIntentTargets(mContext, "AASB", "ABC"));
    }

    @Test
    public void testIsAudioSourceExternal() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertFalse(FileUtil.isAudioSourceExternal(mContext, "VOICE"));

        // Test when audioSource is set to "EXTERNAL"
        JSONObject audioInputTypeForVoiceJson = configJson.getJSONObject("aacs.defaultPlatformHandlers")
                                                        .getJSONObject("audioInput")
                                                        .getJSONObject("audioType")
                                                        .getJSONObject(AASBConstants.AudioInput.AudioType.VOICE);
        audioInputTypeForVoiceJson.put("audioSource", "EXTERNAL");
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertTrue(FileUtil.isAudioSourceExternal(mContext, "VOICE"));
    }

    @Test
    public void testGetAudioExternalSourceForAudioType() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        JSONObject actualJsonObj = FileUtil.getAudioExternalSourceForAudioType(mContext, "VOICE");
        String type = actualJsonObj.optString("type");
        String packageName = actualJsonObj.optString("package");
        String className = actualJsonObj.optString("class");
        assertTrue("ACTIVITY".equals(type) || "RECEIVER".equals(type) || "SERVICE".equals(type));
        assertTrue("com.amazon.aacssampleapp1".equals(packageName));
        assertTrue(".MainActivity".equals(className) || ".IntentReceiver".equals(className));
    }

    @Test
    public void testGetIPCCacheCapacity() throws Exception {
        // Test the sample configuration
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));

        // Test getting the value of cache capacity
        assertEquals(20, FileUtil.getIPCCacheCapacity(mContext));
    }

    @Test
    public void testGetVersionNumber() throws Exception {
        JSONObject configJson =
                TestUtil.readConfig(mTestContext.getAssets().open(TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE));
        prepareAndSaveConfiguration(configJson);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
        assertEquals(1.0, FileUtil.getVersionNumber(mContext), 0.0);
    }
}
