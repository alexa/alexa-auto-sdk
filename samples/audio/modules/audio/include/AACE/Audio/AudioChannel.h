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

#ifndef AACE_AUDIO_AUDIOCHANNEL_H_
#define AACE_AUDIO_AUDIOCHANNEL_H_

#include <AACE/Alexa/MediaPlayer.h>
#include <AACE/Alexa/Speaker.h>
#include "AudioCapture.h"

namespace aace {
namespace audio {

struct AudioOutputChannel {
	std::shared_ptr<alexa::MediaPlayer> mediaPlayer;
	std::shared_ptr<alexa::Speaker> speaker;
};

struct AudioInputChannel {
	std::shared_ptr<AudioCapture> audioCapture;
};

}
}

#endif //AACE_AUDIO_AUDIOCHANNEL_H_