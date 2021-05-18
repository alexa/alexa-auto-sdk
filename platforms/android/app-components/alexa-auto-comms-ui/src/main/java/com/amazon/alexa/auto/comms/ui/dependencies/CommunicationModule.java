package com.amazon.alexa.auto.comms.ui.dependencies;

import android.content.Context;

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
}
