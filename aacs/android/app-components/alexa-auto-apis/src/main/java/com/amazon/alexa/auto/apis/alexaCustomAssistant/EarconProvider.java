package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom earcon. A component can provide custom earcon that will be played.
 */
public interface EarconProvider extends ScopedComponent {
    /**
     * Provide custom start voice earcon
     * @return The resource id of the custom start voice earcon.
     */
    int getAudioCueStartVoice();

    /**
     * Provide custom end of request earcon
     * @return The resource id of the custom end of request earcon.
     */
    int getAudioCueEnd();

    /**
     * Check if custom start voice earcon should be used.
     * @param signal Information for whether custom start voice earcon should be used. For example,
     *               the detected wake word.
     * @return true if custom start voice earcon should be used. Otherwise, return false.
     */
    boolean shouldUseAudioCueStartVoice(String signal);

    /**
     * Check if custom end of request earcon should be used.
     * @param signal Information for whether custom end of request earcon should be used.
     * @return true if custom end of request earcon should be used. Otherwise, return false.
     */
    boolean shouldUseAudioCueEnd(String signal);
}
