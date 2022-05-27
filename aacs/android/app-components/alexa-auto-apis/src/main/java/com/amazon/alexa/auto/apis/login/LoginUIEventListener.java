/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.apis.login;

import com.amazon.alexa.auto.apis.app.ScopedComponent;
import com.amazon.alexa.auto.apis.auth.AuthMode;

/**
 * Listener interface to notify login UI events such as login finished
 * event, that can be used by the app to dismiss login view and launch main
 * view of the app.
 */
public interface LoginUIEventListener extends ScopedComponent {
    /**
     * Invoked by Login View to notify Login Host that Login is finished.
     */
    void loginFinished();

    /**
     * Invoked by Login View to notify Login Host that Login is switched.
     */
    void loginSwitched(AuthMode mode);
}
