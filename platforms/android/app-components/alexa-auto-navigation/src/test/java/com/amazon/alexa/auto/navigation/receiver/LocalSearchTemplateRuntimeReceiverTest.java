package com.amazon.alexa.auto.navigation.receiver;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class LocalSearchTemplateRuntimeReceiverTest {
    private LocalSearchTemplateRuntimeReceiver mClassUnderTest;
    private LocalSearchDirectiveHandler mLocalSearchDirectiveHandler;

    @Before
    public void setup() {
        mClassUnderTest = new LocalSearchTemplateRuntimeReceiver();
        mLocalSearchDirectiveHandler = Mockito.mock(LocalSearchDirectiveHandler.class);
        mClassUnderTest.mLocalSearchDirectiveHandler = mLocalSearchDirectiveHandler;
    }

    @Test
    public void renderLocalSearchListTemplateTest() {
        Intent renderTemplateIntent =
                generateIntent("aacs/RenderTemplateLocalSearchList.json", "com.amazon.aacs.aasb.RenderTemplate");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), renderTemplateIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mLocalSearchDirectiveHandler, Mockito.times(1))
                .renderLocalSearchListTemplate(aacsMessageArgumentCaptor.capture());
        AACSMessage aacsMessage = aacsMessageArgumentCaptor.getValue();
        Assert.assertEquals(aacsMessage.topic, Topic.TEMPLATE_RUNTIME);
        Assert.assertEquals(aacsMessage.action, Action.TemplateRuntime.RENDER_TEMPLATE);

        Optional<String> sampleAACSNavigationMessage =
                TestResourceFileReader.readFileContent("aacs/RenderTemplateLocalSearchList.json");
        try {
            JSONObject sampleMessageJsonObject = new JSONObject(sampleAACSNavigationMessage.get());
            sampleMessageJsonObject.get("payload").equals(aacsMessage.payload);
        } catch (JSONException e) {
            throw new IllegalArgumentException();
        }
    }

    @Test
    public void renderLocalSearchDetailTemplateTest() {
        Intent renderTemplateIntent =
                generateIntent("aacs/RenderTemplateLocalSearchDetail.json", "com.amazon.aacs.aasb.RenderTemplate");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), renderTemplateIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mLocalSearchDirectiveHandler, Mockito.times(1))
                .renderLocalSearchDetailTemplate(aacsMessageArgumentCaptor.capture());
        AACSMessage aacsMessage = aacsMessageArgumentCaptor.getValue();
        Assert.assertEquals(aacsMessage.topic, Topic.TEMPLATE_RUNTIME);
        Assert.assertEquals(aacsMessage.action, Action.TemplateRuntime.RENDER_TEMPLATE);

        Optional<String> sampleAACSNavigationMessage =
                TestResourceFileReader.readFileContent("aacs/RenderTemplateLocalSearchDetail.json");
        try {
            JSONObject sampleMessageJsonObject = new JSONObject(sampleAACSNavigationMessage.get());
            sampleMessageJsonObject.get("payload").equals(aacsMessage.payload);
        } catch (JSONException e) {
            throw new IllegalArgumentException();
        }
    }

    @Test
    public void clearTemplateTest() {
        Intent clearTemplateIntent = generateIntent("aacs/ClearTemplate.json", "com.amazon.aacs.aasb.ClearTemplate");
        mClassUnderTest.onReceive(Mockito.mock(Context.class), clearTemplateIntent);
        ArgumentCaptor<AACSMessage> aacsMessageArgumentCaptor = ArgumentCaptor.forClass(AACSMessage.class);
        Mockito.verify(mLocalSearchDirectiveHandler, Mockito.times(1)).clearTemplate();
    }

    private Intent generateIntent(String resPath, String action) {
        Intent intent = new Intent(action);
        Optional<String> sampleAACSNavigationMessage = TestResourceFileReader.readFileContent(resPath);
        Bundle sampleListeningPayload = new Bundle();
        sampleListeningPayload.putString("message", sampleAACSNavigationMessage.get());
        intent.putExtra("payload", sampleListeningPayload);
        return intent;
    }
}
