/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexaautoclientservice.modules.propertyManager;

import com.amazon.aace.propertyManager.PropertyManager;
import com.amazon.alexaautoclientservice.AACSPropertyContentProvider;

public class PropertyManagerHandler extends PropertyManager {
    public PropertyManagerHandler() {}

    @Override
    public void propertyStateChanged(String name, String value, PropertyState state) {
        if (state == PropertyState.SUCCEEDED) {
            AACSPropertyContentProvider.updatePropertyAndNotifyObservers(name, value, true);
        } else {
            AACSPropertyContentProvider.updatePropertyAndNotifyObservers(name, value, false);
        }
    }

    @Override
    public void propertyChanged(String key, String newValue) {
        AACSPropertyContentProvider.updatePropertyAndNotifyObservers(key, newValue);
    }
}