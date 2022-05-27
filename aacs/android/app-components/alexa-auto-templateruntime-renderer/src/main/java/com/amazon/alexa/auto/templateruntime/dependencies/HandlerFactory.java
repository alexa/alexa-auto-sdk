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
package com.amazon.alexa.auto.templateruntime.dependencies;

import static com.amazon.aacsconstants.TemplateRuntimeConstants.TEMPLATE_TYPE_BODY;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.TEMPLATE_TYPE_LIST;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.TEMPLATE_TYPE_WEATHER;

import android.content.Context;

import com.amazon.alexa.auto.templateruntime.bodytemplate.BodyTemplateDirectiveHandler;
import com.amazon.alexa.auto.templateruntime.listtemplate.ListDirectiveHandler;
import com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler;

import java.util.HashMap;
import java.util.Map;

public class HandlerFactory {
    private Map<String, TemplateDirectiveHandler> templateMap = new HashMap<String, TemplateDirectiveHandler>();

    public TemplateDirectiveHandler getHandler(Context context, String type) throws IllegalArgumentException {
        if (type.startsWith(TEMPLATE_TYPE_BODY)) {
            if (!templateMap.containsKey(TEMPLATE_TYPE_BODY)) {
                templateMap.put(TEMPLATE_TYPE_BODY, new BodyTemplateDirectiveHandler(context));
            }
            return templateMap.get(TEMPLATE_TYPE_BODY);
        } else if (TEMPLATE_TYPE_WEATHER.equals(type)) {
            if (!templateMap.containsKey(TEMPLATE_TYPE_WEATHER)) {
                templateMap.put(TEMPLATE_TYPE_WEATHER, new WeatherDirectiveHandler(context));
            }
            return templateMap.get(TEMPLATE_TYPE_WEATHER);
        } else if (TEMPLATE_TYPE_LIST.equals(type)) {
            if (!templateMap.containsKey((TEMPLATE_TYPE_LIST))) {
                templateMap.put(TEMPLATE_TYPE_LIST, new ListDirectiveHandler(context));
            }
            return templateMap.get(TEMPLATE_TYPE_LIST);
        } else {
            throw new IllegalArgumentException("Unknown template type: " + type);
        }
    }
}
