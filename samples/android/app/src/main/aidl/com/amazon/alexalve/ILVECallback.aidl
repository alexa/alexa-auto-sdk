// ILVECallback.aidl
package com.amazon.alexalve;

interface ILVECallback {
        /**
         * Called when the LVE service has to communicate with the client.
         */
        void sendMessage(String path, String payload);
}
