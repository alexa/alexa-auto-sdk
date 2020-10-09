package com.amazon.alexaautoclientservice.modules.alexaClient;

import android.support.annotation.NonNull;

public interface AuthStateObserver { void onAuthStateChanged(@NonNull String authState); }
