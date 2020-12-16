package com.amazon.alexaautoclientservice.modules.alexaClient;

import androidx.annotation.NonNull;

public interface AuthStateObserver { void onAuthStateChanged(@NonNull String authState); }
