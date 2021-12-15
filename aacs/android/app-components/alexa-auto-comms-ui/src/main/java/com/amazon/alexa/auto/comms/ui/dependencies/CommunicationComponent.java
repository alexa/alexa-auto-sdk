package com.amazon.alexa.auto.comms.ui.dependencies;

import com.amazon.alexa.auto.comms.ui.receiver.BluetoothReceiver;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Communication Dependencies.
 */
@Component(modules = {CommunicationModule.class, AndroidModule.class})
@Singleton
public interface CommunicationComponent {
    /**
     * Inject dependencies for @c BluetoothReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectBluetoothReceiver(BluetoothReceiver receiver);
}
