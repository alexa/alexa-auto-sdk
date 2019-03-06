/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_MOCK_CONECTIONSTATUS_OBSERVER_H_
#define AACE_TEST_UNIT_MOCK_CONECTIONSTATUS_OBSERVER_H_

#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

/// This class allows us to test ConnectionStatusObserver interaction
class MockConnectionStatusObserver : public avsCommon::sdkInterfaces::ConnectionStatusObserverInterface {
public:
    MockConnectionStatusObserver() {
    }
    MOCK_METHOD2(
        onConnectionStatusChanged,
        void(
            ConnectionStatusObserverInterface::Status status,
            ConnectionStatusObserverInterface::ChangedReason reason));
};

}
}
}
#endif //AACE_TEST_UNIT_CONECTIONSTATUS_OBSERVER_H_