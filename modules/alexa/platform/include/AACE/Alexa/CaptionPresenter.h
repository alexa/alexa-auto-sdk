/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_CAPTION_PRESENTER_H
#define AACE_ALEXA_CAPTION_PRESENTER_H

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

class CaptionPresenter : public aace::core::PlatformInterface {
protected:
    CaptionPresenter() = default;

public:
    virtual ~CaptionPresenter();

    /**
     * Provides caption data for TTS
     * 
     * @param [in] lines setting caption
     */
    virtual void setCaption(const std::string& caption) = 0;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_CAPTION_PRESENTER_H