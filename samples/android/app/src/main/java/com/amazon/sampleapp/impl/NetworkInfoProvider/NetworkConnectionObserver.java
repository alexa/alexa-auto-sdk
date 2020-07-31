package com.amazon.sampleapp.impl.NetworkInfoProvider;

import com.amazon.aace.network.NetworkInfoProvider.NetworkStatus;

// An interface for an observer of network status changes
public interface NetworkConnectionObserver {
    // notify observer of connection status change
    void onConnectionStatusChanged(NetworkStatus status);
}
