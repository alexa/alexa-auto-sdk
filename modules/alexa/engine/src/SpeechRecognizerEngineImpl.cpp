/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <exception>
#include <climits>

#include "AACE/Engine/Alexa/UPLService.h"
#include "AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h"
#include "AACE/Engine/Alexa/AlexaMetrics.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SpeechRecognizerEngineImpl");

SpeechRecognizerEngineImpl::SpeechRecognizerEngineImpl( std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface, const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat ) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_speechRecognizerPlatformInterface( speechRecognizerPlatformInterface ),
        m_audioFormat( audioFormat ),
        m_wordSize( audioFormat.sampleSizeInBits / CHAR_BIT ),
        m_state( alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::IDLE ) {
}

bool SpeechRecognizerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::UserInactivityMonitorInterface> userInactivityMonitor,
    std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder,
    std::shared_ptr<aace::engine::alexa::WakewordEngineAdapter> wakewordEngineAdapter ) {

    try
    {
        ThrowIfNull( directiveSequencer, "invalidDirectiveSequencer" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );

        m_audioInputProcessor = alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::create(directiveSequencer, messageSender, contextManager, focusManager, dialogUXStateAggregator, exceptionSender, userInactivityMonitor, speechEncoder);
        ThrowIfNull( m_audioInputProcessor, "couldNotCreateAudioInputProcessor" );

        ThrowIfNot( initializeAudioInputStream(), "initializeAudioInputStreamFailed" );

        // add dialog state observer to aip
        m_audioInputProcessor->addObserver( shared_from_this() );
        m_audioInputProcessor->addObserver( dialogUXStateAggregator );
        
        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_audioInputProcessor ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_audioInputProcessor ), "registerCapabilityFailed");

        m_wakewordEngineAdapter = wakewordEngineAdapter;
        if( m_wakewordEngineAdapter != nullptr ) {
            ThrowIfNot( m_wakewordEngineAdapter->initialize( m_audioInputStream, m_audioFormat ), "wakewordInitializeFailed" );
            m_wakewordEngineAdapter->addKeyWordObserver( shared_from_this() );
        }
        
        m_directiveSequencer = directiveSequencer;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<SpeechRecognizerEngineImpl> SpeechRecognizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface,
    const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::UserInactivityMonitorInterface> userInactivityMonitor,
    std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder,
    std::shared_ptr<aace::engine::alexa::WakewordEngineAdapter> wakewordEngineAdapter ) {

    std::shared_ptr<SpeechRecognizerEngineImpl> speechRecognizerEngineImpl = nullptr;

    try
    {
        ThrowIfNull( speechRecognizerPlatformInterface, "invlaidSpeechRecognizerPlatformInterface" );

        speechRecognizerEngineImpl = std::shared_ptr<SpeechRecognizerEngineImpl>(new SpeechRecognizerEngineImpl(speechRecognizerPlatformInterface, audioFormat));

        ThrowIfNot( speechRecognizerEngineImpl->initialize( directiveSequencer, messageSender, contextManager, focusManager, dialogUXStateAggregator, capabilitiesDelegate, exceptionSender, userInactivityMonitor, speechEncoder, wakewordEngineAdapter ), "initializeSpeechRecognizerEngineImplFailed" );

        return speechRecognizerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( speechRecognizerEngineImpl != nullptr ) {
            speechRecognizerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void SpeechRecognizerEngineImpl::doShutdown()
{
    if ( m_audioInputWriter != nullptr ) {
        m_audioInputWriter->close();
    }

    if( m_audioInputProcessor != nullptr ) {
        m_audioInputProcessor->shutdown();
    }

    if( m_wakewordEngineAdapter != nullptr ) {
        m_wakewordEngineAdapter->removeKeyWordObserver( shared_from_this() );
    }

    if( m_speechRecognizerPlatformInterface != nullptr ) {
        m_speechRecognizerPlatformInterface->setEngineInterface( nullptr );
    }
}

bool SpeechRecognizerEngineImpl::initializeAudioInputStream()
{
    try
    {
        size_t size = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize( m_audioFormat.sampleRateHz * AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count(), m_wordSize, MAX_READERS );
        auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>( size );
        ThrowIfNull( buffer, "couldNotCreateAudioInputBuffer" );
        
        // create the audio input stream
        m_audioInputStream = alexaClientSDK::avsCommon::avs::AudioInputStream::create( buffer, m_wordSize, MAX_READERS );
        ThrowIfNull( m_audioInputStream, "couldNotCreateAudioInputStream" );
        
        // create the audio input writer
        m_audioInputWriter = m_audioInputStream->createWriter( alexaClientSDK::avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE );
        ThrowIfNull( m_audioInputWriter, "couldNotCreateAudioInputWriter" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initializeAudioInputStream").d("reason", ex.what()));
        m_audioInputStream.reset();
        m_audioInputWriter.reset();
        return false;
    }
}

void SpeechRecognizerEngineImpl::setExpectingAudioState( bool state )
{
    if( m_expectingAudio != state ) {
        m_expectingAudio = state;
        m_expectingAudioState_cv.notify_all();
    }
}

bool SpeechRecognizerEngineImpl::waitForExpectingAudioState( bool state, const std::chrono::seconds duration )
{
    std::unique_lock<std::mutex> lock( m_expectingAudioMutex );
    
    return m_expectingAudioState_cv.wait_for( lock, duration, [this, state]() {
        return state == m_expectingAudio;
    });
}

// SpeechRecognizer
bool SpeechRecognizerEngineImpl::onStartCapture( Initiator initiator, uint64_t keywordBegin, uint64_t keywordEnd, const std::string& keyword )
{
    try
    {
        ThrowIf( m_state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::RECOGNIZING, "alreadyRecognizing" );

        // create a new audio provider for the specified initiator type
        std::shared_ptr<alexaClientSDK::capabilityAgents::aip::AudioProvider> audioProvider;

        switch( initiator )
        {
            case Initiator::HOLD_TO_TALK:
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_audioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK, false, true, false );
                break;
                
            case Initiator::TAP_TO_TALK:
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_audioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD, true, true, true );
                break;
        
            case Initiator::WAKEWORD:
                ThrowIf( keywordBegin == SpeechRecognizerEngineInterface::UNSPECIFIED_INDEX, "invalidKeywordBeginIndex" );
                ThrowIf( keywordEnd == SpeechRecognizerEngineInterface::UNSPECIFIED_INDEX, "invalidKeywordEndIndex" );
                ThrowIf( keyword.empty(), "invalidKeyword" );
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_audioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD, true, false, true );
                break;
        }

        // start the recognize event
        ThrowIfNot( startCapture( audioProvider, static_cast<alexaClientSDK::capabilityAgents::aip::Initiator>( initiator ), keywordBegin, keywordEnd, keyword ), "startCaptureFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onStartCapture").d("reason", ex.what()));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::onStopCapture()
{
    try
    {
        ThrowIfNot( m_audioInputProcessor->stopCapture().get(), "stopCaptureFailed" );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onStopCapture").d("reason", ex.what()));
        return false;
    }
}

ssize_t SpeechRecognizerEngineImpl::write( const int16_t* data, const size_t size )
{
    try
    {
        ThrowIfNot( waitForExpectingAudioState( true ), "audioNotExpected" );
        ThrowIfNull( m_audioInputWriter, "nullAudioInputWriter" );
        
        ssize_t result = m_audioInputWriter->write( data, size );
        ThrowIf( result < 0, "errorWritingData" );
        
        return result;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"write").d("reason", ex.what()));
        return -1;
    }
}

void SpeechRecognizerEngineImpl::onKeyWordDetected( std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream, std::string keyword, alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex, alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex, std::shared_ptr<const std::vector<char>> KWDMetadata )
{
    if( m_state == AudioInputProcessorObserverInterface::State::IDLE && m_speechRecognizerPlatformInterface->wakewordDetected( keyword ) ) {
        onStartCapture( Initiator::WAKEWORD, beginIndex, endIndex, keyword );
    }
}

bool SpeechRecognizerEngineImpl::startCapture(
    std::shared_ptr<alexaClientSDK::capabilityAgents::aip::AudioProvider> audioProvider,
    alexaClientSDK::capabilityAgents::aip::Initiator initiator,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index begin,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index keywordEnd,
    const std::string& keyword ) {
    
    try
    {
        // ask the aip to start recognizing input
        ALEXA_METRIC(LX(TAG, "startCapture"), aace::engine::alexa::AlexaMetrics::Location::SPEECH_START_CAPTURE);
        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(aace::engine::alexa::UPLService::DialogState::START_CAPTURE, "", 
            m_directiveSequencer->isCurrentDialogRequestOnline());

        ThrowIfNot( m_audioInputProcessor->recognize( *audioProvider, initiator, std::chrono::steady_clock::now(), begin, keywordEnd, keyword ).get(), "recognizeFailed" );
        
        if( m_expectingAudio == false )
        {
            // let the recognizer know we are expecting audio from the platform interface
            setExpectingAudioState( true );
            
            // notify the platform that we are expecting audio... if the platform returns
            // and error then we reset the expecting audio state and throw an exception
            if( m_speechRecognizerPlatformInterface->startAudioInput() == false ) {
                setExpectingAudioState( false );
                m_audioInputProcessor->resetState();
                Throw( "platformStartAudioInputFailed" );
            }
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"startCapture").d("reason", ex.what()));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::isWakewordSupported() {
    return m_wakewordEngineAdapter != nullptr;
}

bool SpeechRecognizerEngineImpl::isWakewordEnabled() {
    return isWakewordSupported() && m_speechRecognizerPlatformInterface->isWakewordDetectionEnabled();
}

bool SpeechRecognizerEngineImpl::enableWakewordDetection()
{
    try
    {
        // check to make sure wakeword is supported
        ThrowIfNot( isWakewordSupported(), "wakewordNotSupported" );
        
        // check if wakeword detection is already enabled
        ReturnIf( m_wakewordEnabled, true );

        ThrowIfNot( m_wakewordEngineAdapter->enable(), "enableFailed" );

        // set the wakeword enabled and expecting audio flags to true
        m_wakewordEnabled = true;
        setExpectingAudioState( true );

        // tell the platform interface to start providing audio input
        ThrowIfNot( m_speechRecognizerPlatformInterface->startAudioInput(), "platformStartAudioInputFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        m_wakewordEnabled = false;
        setExpectingAudioState( false );
        AACE_ERROR(LX(TAG,"enableWakewordDetection").d("reason",ex.what()));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::disableWakewordDetection()
{
    bool success = true;

    try
    {
        // check if wakeword detection is already disabled
        ReturnIfNot( m_wakewordEnabled, true );

        // check to make sure wakeword is supported
        ThrowIfNot( isWakewordSupported(), "wakewordNotSupported" );

        // tell the platform to stop providing audio input
        ThrowIfNot( m_speechRecognizerPlatformInterface->stopAudioInput(), "platformStopAudioInputFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"disableWakewordDetection").d("reason", ex.what()));
        success = false;
    }

    m_wakewordEngineAdapter->disable();

    m_wakewordEnabled = false;
    setExpectingAudioState( false );

    return success;
}

void SpeechRecognizerEngineImpl::onStateChanged( alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State state )
{
    m_state = state;

    // state changed to BUSY means that either the StopCapture directive has been received
    // or the speech recognizer was stopped manually
    if( state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::BUSY )
    {
        ALEXA_METRIC(LX(TAG, "onStateChanged"), aace::engine::alexa::AlexaMetrics::Location::SPEECH_STOP_CAPTURE);
        ALEXA_METRIC(LX(TAG, "stopCapture").d("dialogRequestId", m_directiveSequencer->getCurrentDialogRequestId()), aace::engine::alexa::AlexaMetrics::Location::SPEECH_STOP_CAPTURE);

        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(aace::engine::alexa::UPLService::DialogState::STOP_CAPTURE, 
            m_directiveSequencer->getCurrentDialogRequestId(), m_directiveSequencer->isCurrentDialogRequestOnline());

        if( m_expectingAudio )
        {
            ALEXA_METRIC(LX(TAG, "onStateChanged"), aace::engine::alexa::AlexaMetrics::Location::END_OF_SPEECH);
            m_speechRecognizerPlatformInterface->endOfSpeechDetected();

            if( m_wakewordEnabled == false )
            {
                if( m_speechRecognizerPlatformInterface->stopAudioInput() == false ) {
                    AACE_ERROR(LX(TAG,"handleAudioStateChanged").d("reason", "platformStopAudioInputFailed"));
                }
                
                setExpectingAudioState( false );
            }
        }
    }
    else if( state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::IDLE )
    {
        if( m_expectingAudio && m_wakewordEnabled == false )
        {
            if( m_speechRecognizerPlatformInterface->stopAudioInput() == false ) {
                AACE_ERROR(LX(TAG,"handleAudioStateChanged").d("reason", "platformStopAudioInputFailed"));
            }

            setExpectingAudioState( false );
        }
    }
    else if( state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH )
    {
        // let the recognizer know we are expecting audio from the platform interface
        setExpectingAudioState( true );
        
        // call platform interface if we are starting the recognizer and the
        // wakeword engine was not already enabled...
        if( m_wakewordEnabled == false ) {
            ThrowIfNot( m_speechRecognizerPlatformInterface->startAudioInput(), "platformStartAudioInputFailed" );
        }
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace
