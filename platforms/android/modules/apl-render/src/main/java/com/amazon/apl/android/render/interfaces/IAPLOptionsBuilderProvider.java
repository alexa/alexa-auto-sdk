/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.interfaces;

import androidx.annotation.NonNull;

import com.amazon.apl.android.APLOptions;

/**
 * Interface to provide an APL options builder object. The builder
 * can be used to override any of the data provides, callback handlers, etc.
 */
public interface IAPLOptionsBuilderProvider {
    @NonNull
    APLOptions.Builder getAPLOptionsBuilder();
}
