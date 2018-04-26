/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/// The sample rate of microphone audio data.
static const unsigned int SAMPLE_RATE_HZ = 16000;

/// The number of audio channels.
static const unsigned int NUM_CHANNELS = 1;

/// The size of each word within the stream.
static const size_t WORD_SIZE = 2;

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

/// The size of the ring buffer.
static const size_t BUFFER_SIZE_IN_SAMPLES = (SAMPLE_RATE_HZ)*AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count();

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SpeechRecognizerEngineImpl");
 
SpeechRecognizerEngineImpl::SpeechRecognizerEngineImpl(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::capabilityAgents::aip::AudioInputProcessor> audioInputProcessor,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer ) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_speechRecognizerPlatformInterface( speechRecognizerPlatformInterface ),
        m_directiveSequencer( directiveSequencer ),
        m_audioInputProcessor( audioInputProcessor ),
        m_expectingAudio( false ),
        m_wakewordEnabled( false ),
        m_state( alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::IDLE ) {
    
    // setup the audio format
    m_compatibleAudioFormat.sampleRateHz = SAMPLE_RATE_HZ;
    m_compatibleAudioFormat.sampleSizeInBits = WORD_SIZE * CHAR_BIT;
    m_compatibleAudioFormat.numChannels = NUM_CHANNELS;
    m_compatibleAudioFormat.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
    m_compatibleAudioFormat.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;
}

std::shared_ptr<SpeechRecognizerEngineImpl> SpeechRecognizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::UserActivityNotifierInterface> userActivityNotifier ) {

    try
    {
        auto audioInputProcessor = alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::create( directiveSequencer, messageSender, contextManager, focusManager, dialogUXStateAggregator, exceptionSender, userActivityNotifier );
        ThrowIfNull( audioInputProcessor, "couldNotCreateAudioInputProcessor" );
        
        auto speechRecognizerEngineImpl = std::shared_ptr<SpeechRecognizerEngineImpl>( new SpeechRecognizerEngineImpl( speechRecognizerPlatformInterface, audioInputProcessor, directiveSequencer ) );

        // add dialog state observer to aip
        audioInputProcessor->addObserver( speechRecognizerEngineImpl );
        audioInputProcessor->addObserver( dialogUXStateAggregator );
        
        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( audioInputProcessor ), "addDirectiveHandlerFailed" );

        return speechRecognizerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void SpeechRecognizerEngineImpl::doShutdown() {
    if( m_audioInputProcessor != nullptr ) {
        m_audioInputProcessor->shutdown();
    }
}

bool SpeechRecognizerEngineImpl::initializeAudioInputStream()
{
    try
    {
        size_t size = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize( BUFFER_SIZE_IN_SAMPLES, WORD_SIZE, MAX_READERS );
        auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>( size );
        ThrowIfNull( buffer, "couldNotCreateAudioInputBuffer" );
        
        // create the audio input stream
        m_audioInputStream = alexaClientSDK::avsCommon::avs::AudioInputStream::create( buffer, WORD_SIZE, MAX_READERS );
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
    m_expectingAudio = state;
    
    // notify state changed
    m_expectingAudioState_cv.notify_all();
}

bool SpeechRecognizerEngineImpl::waitForExpectingAudioState( bool state, const std::chrono::seconds duration )
{
    std::unique_lock<std::mutex> lock( m_expectingAudioMutex );
    
    return m_expectingAudioState_cv.wait_for( lock, duration, [this, state]() {
        return state == m_expectingAudio;
    });
}

// SpeechRecognizer
bool SpeechRecognizerEngineImpl::onHoldToTalk()
{
    try
    {
        ThrowIf( m_state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::RECOGNIZING, "alreadyRecognizing" );

        if( m_expectingAudio == false ) {
            ThrowIfNot( initializeAudioInputStream(), "initializeAudioInputStreamFailed" );
        }
        
        // Creating hold-to-talk audio provider
        //bool holdAlwaysReadable = false;
        //bool holdCanOverride = true;
        //bool holdCanBeOverridden = false;
        auto audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_compatibleAudioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK, false, true, false );
        
        ThrowIfNot( startCapture( audioProvider, alexaClientSDK::capabilityAgents::aip::Initiator::PRESS_AND_HOLD ), "startCaptureFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onHoldToTalk").d("reason", ex.what()));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::onTapToTalk()
{
    try
    {
        ThrowIf( m_state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::RECOGNIZING, "alreadyRecognizing" );
    
        if( m_audioInputWriter == nullptr ) {
            ThrowIfNot( initializeAudioInputStream(), "initializeAudioInputStreamFailed" );
        }
        
        // Creating hold-to-talk audio provider
        //bool holdAlwaysReadable = false;
        //bool holdCanOverride = true;
        //bool holdCanBeOverridden = false;
        auto audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_compatibleAudioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD, true, true, true );
        
        ThrowIfNot( startCapture( audioProvider, alexaClientSDK::capabilityAgents::aip::Initiator::TAP ), "startCaptureFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onTapToTalk").d("reason", ex.what()));
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

void SpeechRecognizerEngineImpl::onKeyWordDetected( std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream, std::string keyword, alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex, alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex )
{
    if( m_speechRecognizerPlatformInterface->wakewordDetected( keyword ) )
    {
        //bool wakeAlwaysReadable = true;
        //bool wakeCanOverride = false;
        //bool wakeCanBeOverridden = true;
        auto audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>( m_audioInputStream, m_compatibleAudioFormat, alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD, true, false, true );
    
        if( endIndex != alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface::UNSPECIFIED_INDEX && beginIndex == alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface::UNSPECIFIED_INDEX ) {
            startCapture( audioProvider, alexaClientSDK::capabilityAgents::aip::Initiator::TAP, endIndex );
        }
        else if( endIndex != alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface::UNSPECIFIED_INDEX && beginIndex != alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface::UNSPECIFIED_INDEX ) {
            startCapture( audioProvider, alexaClientSDK::capabilityAgents::aip::Initiator::WAKEWORD, beginIndex, endIndex, keyword );
        }
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
        ThrowIfNot( m_audioInputProcessor->recognize( *audioProvider, initiator, begin, keywordEnd, keyword ).get(), "recognizeFailed" );
        
        // let the recognizer know we are expecting audio from the platform interface
        setExpectingAudioState( true );
        
        // call platform interface if we are starting the recognizer and the
        // wakeword engine was not already enabled...
        if( m_wakewordEnabled == false ) {
            ThrowIfNot( m_speechRecognizerPlatformInterface->startAudioInput(), "platformStartAudioInputFailed" );
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        setExpectingAudioState( false );
        m_audioInputProcessor->resetState();
        AACE_ERROR(LX(TAG,"startCapture").d("reason", ex.what()));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::isWakewordSupported() {
#if defined WAKEWORD_PRYON || defined WAKEWORD_PRYONLITE
    return true;
#else
    return false;
#endif
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
        if( m_wakewordEnabled ) {
            AACE_WARN(LX(TAG,"enableWakewordDetection").d("reason", "wakewordDetectionAlreadyEnabled"));
            return true;
        }
        
        // initialize the audio input stream
        ThrowIfNot( initializeAudioInputStream(), "initializeAudioInputStreamFailed" );
        
        // create the wakeword detector
    #if defined WAKEWORD_PRYON
        m_wakewordDetector = alexaClientSDK::kwd::PryonKeywordDetector::create( m_audioInputStream, m_compatibleAudioFormat, {}, {}, PRYON_MANIFEST_PATH, PRYON_MODEL_NAME );
    #elif defined WAKEWORD_PRYONLITE
        m_wakewordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create( m_audioInputStream, m_compatibleAudioFormat, {}, {}, PRYON_LITE_THRESHOLD );
    #endif
        ThrowIfNull( m_wakewordDetector, "couldNotCreateWakewordDetector" );

        // add the keyword observer to the wakeword detector
        m_wakewordDetector->addKeyWordObserver( shared_from_this() );
        
        // set the wakeword enabled and expecting audio flags to true
        m_wakewordEnabled = true;
        setExpectingAudioState( true );

        // tell the platform interface to start providing audio input
        ThrowIfNot( m_speechRecognizerPlatformInterface->startAudioInput(), "platformStartAudioInputFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        m_wakewordDetector.reset();
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
        // check to make sure wakeword is supported
        ThrowIfNot( isWakewordSupported(), "wakewordNotSupported" );

        // check if wakeword detection is already disabled
        if( m_wakewordEnabled == false ) {
            AACE_WARN(LX(TAG,"disableWakewordDetection").d("reason", "wakewordDetectionAlreadyDisabled"));
            return true;
        }

        // tell the platform to stop providing audio input
        ThrowIfNot( m_speechRecognizerPlatformInterface->stopAudioInput(), "platformStopAudioInputFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"disableWakewordDetection").d("reason", ex.what()));
        success = false;
    }

    m_wakewordDetector.reset();
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
        if( m_expectingAudio )
        {
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
