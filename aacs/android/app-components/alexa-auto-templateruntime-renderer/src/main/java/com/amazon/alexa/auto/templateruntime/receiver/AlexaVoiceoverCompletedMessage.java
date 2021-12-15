package com.amazon.alexa.auto.templateruntime.receiver;

/**
 * Model for message that is published when Alexa completes it's voice response
 */
public class AlexaVoiceoverCompletedMessage {

    //time in milliseconds at which Alexa's voice response was completed
    private final Long completedAt;

    public AlexaVoiceoverCompletedMessage(Long completedAt) {
        this.completedAt = completedAt;
    }

    public Long getCompletedAt() {
        return completedAt;
    }

}
