package com.amazon.alexa.auto.app.common.ui;

import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.android.controller.ActivityController;

import java.util.Optional;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(RobolectricTestRunner.class)
public class TwoChoiceDialogTest {
    private static final String DIALOG_KEY = "dlg-key";
    private static final String TITLE = "title";
    private static final String CONTENT = "content";
    private static final String BTN1TEXT = "btn1";
    private static final String BTN2TEXT = "btn2";

    private ActivityController<FragmentActivity> mActivityController;
    private FragmentActivity mActivity;
    private TwoChoiceDialog mClassUnderTest;

    public static class EventReceiver {
        private CountDownLatch mBtn1Latch = new CountDownLatch(1);
        private CountDownLatch mBtn2Latch = new CountDownLatch(1);
        private TwoChoiceDialog.Button1Clicked mBtn1ClickEvent;
        private TwoChoiceDialog.Button2Clicked mBtn2ClickEvent;

        @Subscribe
        public void onBtn1EventReceived(TwoChoiceDialog.Button1Clicked clicked) {
            mBtn1ClickEvent = clicked;
            mBtn1Latch.countDown();
        }

        @Subscribe
        public void onBtn1EventReceived(TwoChoiceDialog.Button2Clicked clicked) {
            mBtn2ClickEvent = clicked;
            mBtn2Latch.countDown();
        }

        public Optional<TwoChoiceDialog.Button1Clicked> awaitForBtn1(int timeoutMs) {
            try {
                if (mBtn1Latch.await(timeoutMs, TimeUnit.MILLISECONDS))
                    return Optional.of(mBtn1ClickEvent);
            } catch (Exception exception) {
            }
            return Optional.empty();
        }

        public Optional<TwoChoiceDialog.Button2Clicked> awaitForBtn2(int timeoutMs) {
            try {
                if (mBtn2Latch.await(timeoutMs, TimeUnit.MILLISECONDS))
                    return Optional.of(mBtn2ClickEvent);
            } catch (Exception exception) {
            }
            return Optional.empty();
        }
    }

    @Before
    public void setup() {
        mActivityController = Robolectric.buildActivity(FragmentActivity.class);
        mActivityController.create().start().resume();
        mActivity = mActivityController.get();
        mClassUnderTest = TwoChoiceDialog.createDialog(
                new TwoChoiceDialog.Params(DIALOG_KEY, TITLE, CONTENT, BTN1TEXT, BTN2TEXT));
    }

    @Test
    public void testBtn1Click() {
        EventReceiver eventReceiver = new EventReceiver();
        EventBus.getDefault().register(eventReceiver);

        mClassUnderTest.show(mActivity.getSupportFragmentManager(), "test");
        mClassUnderTest.mBtn1.performClick();

        Optional<TwoChoiceDialog.Button1Clicked> event = eventReceiver.awaitForBtn1(10000);
        Assert.assertTrue(event.isPresent());
        Assert.assertEquals(DIALOG_KEY, event.get().dialogKey);

        EventBus.getDefault().unregister(eventReceiver);
    }

    @Test
    public void testBtn2Click() {
        EventReceiver eventReceiver = new EventReceiver();
        EventBus.getDefault().register(eventReceiver);

        mClassUnderTest.show(mActivity.getSupportFragmentManager(), "test");
        mClassUnderTest.mBtn2.performClick();

        Optional<TwoChoiceDialog.Button2Clicked> event = eventReceiver.awaitForBtn2(10000);
        Assert.assertTrue(event.isPresent());
        Assert.assertEquals(DIALOG_KEY, event.get().dialogKey);

        EventBus.getDefault().unregister(eventReceiver);
    }

    @Test
    public void testDialogSurvivesScreenRotation() {
        EventReceiver eventReceiver = new EventReceiver();
        EventBus.getDefault().register(eventReceiver);

        mClassUnderTest.show(mActivity.getSupportFragmentManager(), "test");

        // Force the activity to restart (and restore from the saved instance data).
        Bundle bundle = new Bundle();
        mActivityController.saveInstanceState(bundle).pause().stop().destroy();
        mActivityController = Robolectric.buildActivity(FragmentActivity.class)
                                      .create(bundle)
                                      .start()
                                      .restoreInstanceState(bundle)
                                      .resume();
        mActivity = mActivityController.get();

        TwoChoiceDialog dialogAfterConfigChange = null;
        for (Fragment fragment : mActivity.getSupportFragmentManager().getFragments()) {
            if (fragment instanceof TwoChoiceDialog) {
                dialogAfterConfigChange = (TwoChoiceDialog) fragment;
                break;
            }
        }

        Assert.assertNotNull(dialogAfterConfigChange);
        dialogAfterConfigChange.mBtn2.performClick();

        Optional<TwoChoiceDialog.Button2Clicked> event = eventReceiver.awaitForBtn2(10000);
        Assert.assertTrue(event.isPresent());
        Assert.assertEquals(DIALOG_KEY, event.get().dialogKey);

        EventBus.getDefault().unregister(eventReceiver);
    }
}
