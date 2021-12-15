package com.amazon.alexa.auto.apl.handler;

import static org.mockito.Mockito.times;

import android.content.Context;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.render.APLPresenter;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;

@RunWith(RobolectricTestRunner.class)
public class APLHandlerTest {
    private Context mContext;
    private AACSMessageSender mAACSSender;
    private APLLayout mAPLLayout;
    private APLPresenter mAPLPresenter;
    private APLLocalInfoHandler mAPLLocalInfoHandler;

    private APLHandler mClassUnderTest;

    @Before
    public void setup() {
        mContext = Mockito.mock(Context.class);
        mAACSSender = Mockito.mock(AACSMessageSender.class);
        mAPLLayout = Mockito.mock(APLLayout.class);
        mAPLPresenter = Mockito.mock(APLPresenter.class);
        mAPLLocalInfoHandler = Mockito.mock(APLLocalInfoHandler.class);

        mClassUnderTest = new APLHandler(new WeakReference<>(mContext), mAACSSender, mAPLLayout);
        mClassUnderTest.mPresenter = mAPLPresenter;
        mClassUnderTest.mAPLLocalInfoHandler = mAPLLocalInfoHandler;
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
        Mockito.verify(mClassUnderTest.mAPLLocalInfoHandler, times(1)).clearLocalInfoData();
    }

    @Test
    public void executeCommandTest() {
        mClassUnderTest.executeCommands("samplePayload", "sampleToken");
        Mockito.verify(mClassUnderTest.mPresenter, times(1)).onExecuteCommands("samplePayload", "sampleToken");
    }

    @Test
    public void handleAPLRuntimePropertiesTest() {
        mClassUnderTest.handleAPLRuntimeProperties("sampleAPLProperties");
        Mockito.verify(mClassUnderTest.mPresenter, times(1)).onAPLRuntimeProperties("sampleAPLProperties");
    }
}
