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

#ifndef AACE_ALEXA_NOTIFICATIONS_H
#define AACE_ALEXA_NOTIFICATIONS_H

#include "AudioChannel.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c Notifications class should be extended by the platform implementation to handle Notifications from AVS and provide an indication whether notifications are available.
 */
class Notifications : public AudioChannel {
protected:
    Notifications( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~Notifications() = default;
    /**
     *  An enum class which captures the states a notifications object can be in.
     */
    enum class IndicatorState {
      /**
       *  The notifications indicator should be turned off
       */
      OFF = 0,
      /**
       *  The notifications indicator should be turned on
       */
      ON = 1,
      /**
       *  The notifications indicator state is unknown
       */
      UNKNOWN
    };
    /**
     *  Called when the notification indicator has changed state
     *
     *
     */
    virtual void setIndicator(IndicatorState state) = 0;
private:

};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_NOTIFICATIONS_H
