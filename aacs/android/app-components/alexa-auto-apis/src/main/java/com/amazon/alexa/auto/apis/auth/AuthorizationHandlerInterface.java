package com.amazon.alexa.auto.apis.auth;

import android.content.Context;

/**
 * Authorization handler interface will be implemented by authorization provider, such as preview mode.
 * It handles authorization requests such as refreshing auth token, sending auth access token,
 * provide auth status update, etc.
 */
public interface AuthorizationHandlerInterface {
    void initialize(Context context);

    void uninitialize();
}
