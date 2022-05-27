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
package com.amazon.alexa.auto.setup.workflow;

/**
 * Alexa setup workflow step.
 */
public class WorkflowStep {
    private final String mEvent;
    private final String mType;
    private final String mResource;

    /**
     * Constructor.
     * @param event Setup workflow event.
     * @param type  Setup workflow type, each workflow step could be a view or an action.
     * @param resource Resource to execute workflow step.
     */
    public WorkflowStep(String event, String type, String resource) {
        mEvent = event;
        mType = type;
        mResource = resource;
    }

    public String getEvent() {
        return mEvent;
    }

    public String getType() {
        return mType;
    }

    public String getResource() {
        return mResource;
    }
}
