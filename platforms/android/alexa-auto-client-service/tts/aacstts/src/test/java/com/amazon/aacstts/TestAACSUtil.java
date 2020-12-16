package com.amazon.aacstts;

import static junit.framework.Assert.assertFalse;
import static junit.framework.Assert.assertTrue;

import static org.powermock.api.mockito.PowerMockito.doReturn;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Matchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@RunWith(PowerMockRunner.class)
@PrepareForTest(AACSUtil.class)
public class TestAACSUtil {
    private CompletableFuture<Boolean> mIsAACSRunningPingResponse;
    private Optional<Boolean> mIsAlexaClientConnected;
    private ExecutorService mExecutor;

    @Before
    public void setup() {
        PowerMockito.spy(AACSUtil.class);
        mIsAACSRunningPingResponse = new CompletableFuture<>();
        mIsAlexaClientConnected = Optional.empty();
        mExecutor = Executors.newSingleThreadExecutor();
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndSucceeds_alexaClientConnected_returnsTrue() throws Exception {
        // Test if checkIfAACSIsConnected returns true when the ping succeeds and AlexaClient is connected
        mIsAlexaClientConnected = Optional.of(true);
        mIsAACSRunningPingResponse.complete(true);
        doReturn(true).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertTrue(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndFails_alexaClientConnected_returnsTrue() throws Exception {
        // Test if checkIfAACSIsConnected returns true when the ping fails but AlexaClient is connected later
        mIsAlexaClientConnected = Optional.of(true);
        mIsAACSRunningPingResponse.complete(false);
        doReturn(false).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertTrue(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndSucceeds_alexaClientDisconnected_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns false when the ping succeeds but AlexaClient is disconnected
        mIsAlexaClientConnected = Optional.of(false);
        mIsAACSRunningPingResponse.complete(true);
        doReturn(true).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertFalse(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndFails_alexaClientDisconnected_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns false when the ping fails and AlexaClient is disconnected
        mIsAlexaClientConnected = Optional.of(false);
        mIsAACSRunningPingResponse.complete(false);
        doReturn(false).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertFalse(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndSucceeds_alexaClientEmpty_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns true when AlexaClient ConnectionStatus
        // is not present but mIsAACSRunningPingResponse is done and is true.
        mIsAACSRunningPingResponse.complete(true);
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertTrue(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingNotDoneAndSucceedsLater_alexaClientEmpty_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns true when AlexaClient ConnectionStatus
        // is not present, mIsAACSRunningPingResponse is not done but pingAACSToCheckIfConnected returns true.
        doReturn(true).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertTrue(result.get());
        assertTrue(mIsAACSRunningPingResponse.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingIsDoneAndFails_alexaClientEmpty_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns true when AlexaClient ConnectionStatus
        // is not present, mIsAACSRunningPingResponse is done and is false.
        mIsAACSRunningPingResponse.complete(false);
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertFalse(result.get());
    }

    @Test
    public void checkIfAACSIsConnected_pingNotDoneAndFailsLater_alexaClientEmpty_returnsfalse() throws Exception {
        // Test if checkIfAACSIsConnected returns false when AlexaClient ConnectionStatus
        // is not present, mIsAACSRunningPingResponse is not done and pingAACSToCheckIfConnected returns false.
        doReturn(false).when(AACSUtil.class, "pingAACSToCheckIfConnected", Mockito.any());
        CompletableFuture<Boolean> result =
                AACSUtil.checkIfAACSIsConnected(mIsAlexaClientConnected, mIsAACSRunningPingResponse, mExecutor, null);
        assertFalse(result.get());
        assertFalse(mIsAACSRunningPingResponse.get());
    }
}
