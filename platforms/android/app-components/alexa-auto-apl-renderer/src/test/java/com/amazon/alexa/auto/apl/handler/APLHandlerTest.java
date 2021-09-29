package com.amazon.alexa.auto.apl.handler;

import static org.mockito.Mockito.times;

import android.content.Context;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apl.handler.APLHandler;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.render.APLPresenter;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;

@RunWith(RobolectricTestRunner.class)
public class APLHandlerTest {
    @Mock
    private Context mContext;
    @Mock
    private AACSMessageSender mAACSSender;
    @Mock
    private APLLayout mAPLLayout;
    @Mock
    private APLPresenter mAPLPresenter;

    private APLHandler mClassUnderTest;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);
        mClassUnderTest = new APLHandler(new WeakReference<>(mContext), mAACSSender, mAPLLayout);
        mClassUnderTest.mPresenter = mAPLPresenter;
    }

    @Test
    public void renderDocumentTest() {
        mClassUnderTest.renderDocument("samplePayload", "sampleToken", "sampleWindowId");
        Mockito.verify(mClassUnderTest.mPresenter, times(1))
                .onRenderDocument("samplePayload", "sampleToken", "sampleWindowId");
    }

    @Test
    public void clearDocumentTest() {
        mClassUnderTest.clearDocument("sampleToken");
        Mockito.verify(mClassUnderTest.mPresenter, times(1)).onClearDocument("sampleToken");
    }

    @Test
    public void executeCommandTest() {
        mClassUnderTest.executeCommands("samplePayload", "sampleToken");
        Mockito.verify(mClassUnderTest.mPresenter, times(1)).onExecuteCommands("samplePayload", "sampleToken");
    }
}
