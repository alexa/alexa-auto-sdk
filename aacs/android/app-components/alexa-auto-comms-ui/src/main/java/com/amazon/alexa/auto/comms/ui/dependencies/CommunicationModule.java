package com.amazon.alexa.auto.comms.ui.dependencies;

import android.content.Context;
import android.telecom.TelecomManager;

import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects for Alexa communication.
 */
@Module
public class CommunicationModule {
    @Provides
    @Singleton
    public BluetoothDirectiveHandler provideCommunicationDirectiveHandler(WeakReference<Context> context) {
        return new BluetoothDirectiveHandler(context);
    }

    @Provides
    @Singleton
    public TelecomManager provideTelecomManager(WeakReference<Context> context) {
        return (TelecomManager) context.get().getSystemService(Context.TELECOM_SERVICE);
    }
}
