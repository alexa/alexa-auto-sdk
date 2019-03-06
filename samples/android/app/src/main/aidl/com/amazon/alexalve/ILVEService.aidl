// ILVEService.aidl
package com.amazon.alexalve;

import com.amazon.alexalve.ILVECallback;

interface ILVEService {
    void registerCallback(ILVECallback callback);
    void unregisterCallback(ILVECallback callback);
    void sendMessage(String path, String payload);

    String getVersion();
    String getConfiguration();
    boolean configure(String configuration);
    boolean start();
    boolean stop();
    boolean isRunning();
}
