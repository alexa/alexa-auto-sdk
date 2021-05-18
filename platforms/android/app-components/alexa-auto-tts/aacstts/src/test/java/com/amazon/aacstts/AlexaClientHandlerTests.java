package com.amazon.aacstts;

import com.amazon.aacsconstants.Action;
import com.amazon.aacstts.handler.AlexaClientHandler;

import org.json.JSONException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * Unit tests for {@link AlexaClientHandler}
 */
public class AlexaClientHandlerTests {
    private AlexaClientHandler mAlexaClientHandler;
    private static final String TEST_AACS_MESSAGE_ID = "testMessageId";
    private static final String TEST_AACS_TOPIC = "testAACSTopic";

    private static final String TEST_VALID_ACTION = Action.AlexaClient.CONNECTION_STATUS_CHANGED;
    private static final String TEST_INVALID_ACTION = "invalidAction";

    @Before
    public void setup() {
        mAlexaClientHandler = new AlexaClientHandler();
    }

    /**
     * @see com.amazon.aacstts.handler.AlexaClientHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_alexaIsConnected_setsAlexaIsConnectedFlagTrue() throws JSONException {
        String payload = "{\"status\":\"CONNECTED\"}";

        mAlexaClientHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TEST_VALID_ACTION, payload);

        Assert.assertTrue(mAlexaClientHandler.isAlexaClientConnected().isPresent());
        Assert.assertTrue(mAlexaClientHandler.isAlexaClientConnected().get());
    }

    /**
     * @see com.amazon.aacstts.handler.AlexaClientHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_alexaIsNotConnected_setsAlexaIsConnectedFlagFalse() throws JSONException {
        String payload = "{\"status\":\"DISCONNECTED\"}";

        mAlexaClientHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TEST_VALID_ACTION, payload);

        Assert.assertTrue(mAlexaClientHandler.isAlexaClientConnected().isPresent());
        Assert.assertFalse(mAlexaClientHandler.isAlexaClientConnected().get());
    }

    /**
     * @see com.amazon.aacstts.handler.AlexaClientHandler#handleAACSMessage(String, String, String, String)
     */
    @Test
    public void handleAACSMessage_invalidActionIsBeingPassed_setsAlexaIsConnectedFlagFalse() throws JSONException {
        String payload = "{\"status\":\"DISCONNECTED\"}";

        mAlexaClientHandler.handleAACSMessage(TEST_AACS_MESSAGE_ID, TEST_AACS_TOPIC, TEST_INVALID_ACTION, payload);

        Assert.assertFalse(mAlexaClientHandler.isAlexaClientConnected().isPresent());
    }
}
