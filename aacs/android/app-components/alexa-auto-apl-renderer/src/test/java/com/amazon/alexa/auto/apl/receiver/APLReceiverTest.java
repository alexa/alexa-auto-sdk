package com.amazon.alexa.auto.apl.receiver;

import static org.mockito.Mockito.spy;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.apl.APLDirective;
import com.amazon.alexa.auto.apl.TestResourceFileReader;
import com.amazon.alexa.auto.apl.receiver.APLReceiver;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class APLReceiverTest {
    @Mock
    private Context mContext;

    private APLReceiver mClassUnderTest;
    private EventBus eventBus;

    private String receiveMessageTopic;
    private String receiveMessageAction;
    private String receiveMessagePayload;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);
        mClassUnderTest = spy(new APLReceiver());
        EventBus.getDefault().register(this);
        eventBus = spy(EventBus.getDefault());
    }

    @Test
    public void handleRenderDocumentTest() {
        Intent getRenderDocumentIntent =
                generateIntent("aacs/RenderDocument.json", "com.amazon.aacs.aasb.RenderDocument");
        mClassUnderTest.onReceive(mContext, getRenderDocumentIntent);
        ArgumentCaptor<Intent> intentCaptor = ArgumentCaptor.forClass(Intent.class);
        Mockito.verify(mContext, Mockito.times(1)).startActivity(intentCaptor.capture());
        Intent launchedIntent = intentCaptor.getValue();
        Preconditions.checkNotNull(launchedIntent.getExtras());
        Assert.assertEquals(launchedIntent.getExtras().getString("payload"),
                "{\"payload\":\"{this is a sample payload for APL}\"}");
    }

    @Test
    public void handleClearDocumentTest() {
        Intent getClearDocumentIntent = generateIntent("aacs/ClearDocument.json", "com.amazon.aacs.aasb.ClearDocument");
        mClassUnderTest.onReceive(mContext, getClearDocumentIntent);
        Assert.assertEquals(receiveMessageTopic, Topic.APL);
        Assert.assertEquals(receiveMessageAction, Action.APL.CLEAR_DOCUMENT);
    }

    @Test
    public void handleUpdateAPLRuntimePropertiesTest() {
        Intent getClearDocumentIntent = generateIntent(
                "aacs/UpdateAPLRuntimeProperties.json", "com.amazon.aacs.aasb.UpdateAPLRuntimeProperties");
        mClassUnderTest.onReceive(mContext, getClearDocumentIntent);
        Assert.assertEquals(receiveMessageTopic, Topic.APL);
        Assert.assertEquals(receiveMessageAction, Action.APL.UPDATE_APL_RUNTIME_PROPERTIES);
    }

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSAPLMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle samplePayload = new Bundle();
        samplePayload.putString("message", sampleAACSAPLMessage.get());
        intent.putExtra("payload", samplePayload);
        return intent;
    }

    @Subscribe
    public void testOnReceiveEvent(APLDirective directive) {
        receiveMessageTopic = directive.message.topic;
        receiveMessageAction = directive.message.action;
        receiveMessagePayload = directive.message.payload;
    }
}
