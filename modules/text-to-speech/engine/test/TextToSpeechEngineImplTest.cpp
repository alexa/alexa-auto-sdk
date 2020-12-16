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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AACE/TextToSpeech/TextToSpeech.h"
#include "AACE/Audio/AudioStream.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechEngineImpl.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechServiceInterface.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechSynthesizerInterface.h"
#include "AACE/Engine/TextToSpeech/PrepareSpeechResult.h"

namespace aace {
namespace test {
namespace unit {

/**
 * Mock Text to Speech platform interface.
 */
class MockTextToSpeechPlatformInterface : public aace::textToSpeech::TextToSpeech {
public:
    MOCK_METHOD4(
        prepareSpeech,
        bool(
            const std::string& speechId,
            const std::string& text,
            const std::string& provider,
            const std::string& options));
    MOCK_METHOD2(getCapabilities, bool(const std::string& requestId, const std::string& provider));
    MOCK_METHOD2(prepareSpeechFailed, void(const std::string& speechId, const std::string& reason));
    MOCK_METHOD3(
        prepareSpeechCompleted,
        void(
            const std::string& speechId,
            std::shared_ptr<aace::audio::AudioStream> preparedAudio,
            const std::string& metadata));
    MOCK_METHOD2(capabilitiesReceived, void(const std::string& requestId, const std::string& capabilities));
};

/**
 * Mock Text to Speech Service interface.
 */
class MockTextToSpeechServiceInterface : public aace::engine::textToSpeech::TextToSpeechServiceInterface {
public:
    MOCK_METHOD1(
        getTextToSpeechProvider,
        std::shared_ptr<aace::engine::textToSpeech::TextToSpeechSynthesizerInterface>(const std::string& name));

    MOCK_METHOD2(
        registerTextToSpeechProvider,
        void(
            const std::string& textToSpeechProviderName,
            std::shared_ptr<aace::engine::textToSpeech::TextToSpeechSynthesizerInterface> textToSpeechProvider));
};

/**
 * Mock Text to Speech Synthesizer interface.
 */
class MockTextToSpeechSynthesizerInterface : public aace::engine::textToSpeech::TextToSpeechSynthesizerInterface {
public:
    MOCK_METHOD3(
        prepareSpeech,
        std::future<aace::engine::textToSpeech::PrepareSpeechResult>(
            const std::string& speechId,
            const std::string& text,
            const std::string& options));
    MOCK_METHOD1(getCapabilities, std::future<std::string>(const std::string& requestId));
};

/**
 * Unit test creation of TextToSpeechEngineImpl class.
 */
class TextToSpeechEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockTextToSpeechPlatformInterface =
            std::make_shared<testing::StrictMock<MockTextToSpeechPlatformInterface>>();
        m_mockTextToSpeechServiceInterface = std::make_shared<testing::StrictMock<MockTextToSpeechServiceInterface>>();
        m_mockTextToSpeechSynthesizerInterface =
            std::make_shared<testing::StrictMock<MockTextToSpeechSynthesizerInterface>>();
        m_textToSpeechEngineImpl = aace::engine::textToSpeech::TextToSpeechEngineImpl::create(
            m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    }

    void TearDown() override {
        m_textToSpeechEngineImpl->shutdown();
    }

    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> m_textToSpeechEngineImpl;

    /// @c TextToSpeech platform interface mock implementation
    std::shared_ptr<aace::textToSpeech::TextToSpeech> m_mockTextToSpeechPlatformInterface;

    std::shared_ptr<testing::StrictMock<MockTextToSpeechServiceInterface>> m_mockTextToSpeechServiceInterface;
    std::shared_ptr<testing::StrictMock<MockTextToSpeechSynthesizerInterface>> m_mockTextToSpeechSynthesizerInterface;
};

/**
 * @test create
 */
TEST_F(TextToSpeechEngineImplTest, create) {
    EXPECT_NE(nullptr, m_textToSpeechEngineImpl);
}

/**
 * @test createWithNullTextToSpeechPlatform
 */
TEST_F(TextToSpeechEngineImplTest, createWithNullTextToSpeechPlatform) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl =
        engine::textToSpeech::TextToSpeechEngineImpl::create(nullptr, m_mockTextToSpeechServiceInterface);
    EXPECT_EQ(nullptr, testTextToSpeechEngineImpl);
}

/**
 * @test createWithNullTextToSpeechServiceInterface
 */
TEST_F(TextToSpeechEngineImplTest, createWithNullTextToSpeechServiceInterface) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl =
        engine::textToSpeech::TextToSpeechEngineImpl::create(m_mockTextToSpeechPlatformInterface, nullptr);
    EXPECT_EQ(nullptr, testTextToSpeechEngineImpl);
}

/**
 * @test prepareSpeechWithEmptySpeechId
 */
TEST_F(TextToSpeechEngineImplTest, prepareSpeechWithEmptySpeechId) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string speechId = "";
    const std::string text = "TEST";
    const std::string provider = "text-to-speech-provider";
    const std::string options = "TEST";
    EXPECT_FALSE(testTextToSpeechEngineImpl->onPrepareSpeech(speechId, text, provider, options))
        << "Call to onPreapreSpeech() expected to fail!";
}

/**
 * @test prepareSpeechWithEmptyText
 */
TEST_F(TextToSpeechEngineImplTest, prepareSpeechWithEmptyText) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string speechId = "TEXT_TO_SPEECH-1";
    const std::string text = "";
    const std::string provider = "text-to-speech-provider";
    const std::string options = "TEST";
    EXPECT_FALSE(testTextToSpeechEngineImpl->onPrepareSpeech(speechId, text, provider, options))
        << "Call to onPreapreSpeech() expected to fail!";
}

/**
 * @test prepareSpeechWithInvalidProvider
 */
TEST_F(TextToSpeechEngineImplTest, prepareSpeechWithInvalidProvider) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string speechId = "TEXT_TO_SPEECH-1";
    const std::string text = "TEST";
    const std::string provider = "INVALID_PROVIDER";
    const std::string options = "";
    EXPECT_CALL(*m_mockTextToSpeechServiceInterface, getTextToSpeechProvider("INVALID_PROVIDER"))
        .Times(1)
        .WillOnce(testing::Return(nullptr));
    EXPECT_FALSE(testTextToSpeechEngineImpl->onPrepareSpeech(speechId, text, provider, options))
        << "Call to onPreapreSpeech() expected to fail!";
}

/**
 * @test prepareSpeechWithInvalidOptions
 */
TEST_F(TextToSpeechEngineImplTest, prepareSpeechWithInvalidOptions) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string speechId = "TEXT_TO_SPEECH-1";
    const std::string text = "TEST";
    const std::string provider = "text-to-speech-provider";
    const std::string options = "NOT_A_JSON";

    EXPECT_CALL(*m_mockTextToSpeechServiceInterface, getTextToSpeechProvider(provider))
        .Times(1)
        .WillOnce(testing::Return(m_mockTextToSpeechSynthesizerInterface));
    EXPECT_FALSE(testTextToSpeechEngineImpl->onPrepareSpeech(speechId, text, provider, options))
        << "Call to onPreapreSpeech() expected to fail!";
}

/**
 * @test prepareSpeechWithValidParameters
 */
TEST_F(TextToSpeechEngineImplTest, prepareSpeechWithValidParameters) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string speechId = "TEXT_TO_SPEECH-1";
    const std::string text = "TEST";
    const std::string provider = "text-to-speech-provider";
    const std::string options = "";
    EXPECT_CALL(*m_mockTextToSpeechServiceInterface, getTextToSpeechProvider(provider))
        .Times(1)
        .WillOnce(testing::Return(m_mockTextToSpeechSynthesizerInterface));
    EXPECT_TRUE(testTextToSpeechEngineImpl->onPrepareSpeech(speechId, text, provider, options))
        << "Call to onPreapreSpeech() expected to pass!";
}

/**
 * @test getCapabilitiesWithEmptyRequestId
 */
TEST_F(TextToSpeechEngineImplTest, getCapabilitiesWithEmptyRequestId) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string requestId = "";
    const std::string provider = "text-to-speech-provider";
    EXPECT_FALSE(testTextToSpeechEngineImpl->onGetCapabilities(requestId, provider))
        << "Call to ononGetCapabilities() expected to fail!";
}

/**
 * @test getCapabilitiesWithInvalidProvider
 */
TEST_F(TextToSpeechEngineImplTest, getCapabilitiesWithInvalidProvider) {
    std::shared_ptr<aace::engine::textToSpeech::TextToSpeechEngineImpl> testTextToSpeechEngineImpl;
    testTextToSpeechEngineImpl = engine::textToSpeech::TextToSpeechEngineImpl::create(
        m_mockTextToSpeechPlatformInterface, m_mockTextToSpeechServiceInterface);
    const std::string requestId = "TEXT_TO_SPEECH-1";
    const std::string provider = "INVALID_PROVIDER";
    EXPECT_CALL(*m_mockTextToSpeechServiceInterface, getTextToSpeechProvider(provider))
        .Times(1)
        .WillOnce(testing::Return(nullptr));
    EXPECT_FALSE(testTextToSpeechEngineImpl->onGetCapabilities(requestId, provider))
        << "Call to onPreapreSpeech() expected to fail!";
}

}  // namespace unit
}  // namespace test
}  // namespace aace
