package com.amazon.alexaautoclientservice.modules.alexaClient;

import androidx.annotation.NonNull;

public interface ConnectionStateObserver { void onConnectionStateChanged(@NonNull String connectionState); }
