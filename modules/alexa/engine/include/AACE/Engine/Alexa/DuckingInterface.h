/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_DUCKING_INTERFACE_H
#define AACE_ENGINE_ALEXA_DUCKING_INTERFACE_H

namespace aace {
namespace engine {
namespace alexa {

/**
 * @c DuckingInterface provides an interface that allows implementation class to receive ducking calls
 */
class DuckingInterface {
public:
    /**
     * Starts channel volume attenuation for the underlying speaker.
     *
     * @return @c true if the operation succeeded, @c false otherwise. The API returns true, if the
     * channel is already attenuated.
     */
    virtual bool startDucking() = 0;

    /**
     * Restores the channel volume for the underlying speaker.
     *
     * @return @c true if the operation succeeded, @c false otherwise. The API returns true, if the
     * channel was not attenuated.
     */
    virtual bool stopDucking() = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_ALEXA_DUCKING_INTERFACE_H
