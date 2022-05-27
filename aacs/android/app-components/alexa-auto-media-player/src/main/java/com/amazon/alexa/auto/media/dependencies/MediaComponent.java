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
package com.amazon.alexa.auto.media.dependencies;

import com.amazon.alexa.auto.media.browse.AlexaMediaBrowseService;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Media Dependencies.
 */
@Component(modules = {AndroidModule.class, AACSModule.class, MediaModule.class})
@Singleton
public interface MediaComponent {
    /**
     * Inject dependencies for @c AlexaMediaBrowseService.
     *
     * @param service Service where dependencies are injected.
     */
    void injectMediaBrowseService(AlexaMediaBrowseService service);
}
