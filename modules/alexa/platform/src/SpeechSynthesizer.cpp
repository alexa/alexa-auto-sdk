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

#include <AACE/Alexa/SpeechSynthesizer.h>

namespace aace {
namespace alexa {

SpeechSynthesizer::~SpeechSynthesizer() = default;  // key function
#ifdef OBIGO_AIDAEMON
bool SpeechSynthesizer::startTTS(std::string startEvent, std::string finishEvent) {
  if (auto m_speechSynthesizerEngineInterface_lock = m_speechSynthesizerEngineInterface.lock()) {
    return m_speechSynthesizerEngineInterface_lock->onstartTTS(startEvent, finishEvent);
  } else {
    return false;
  }
}

void SpeechSynthesizer::setEngineInterface(
    std::shared_ptr<aace::alexa::SpeechSynthesizerEngineInterface> speechSynthesizerEngineInterface) {
  m_speechSynthesizerEngineInterface = speechSynthesizerEngineInterface;
}
#endif
}  // namespace alexa
}  // namespace aace
