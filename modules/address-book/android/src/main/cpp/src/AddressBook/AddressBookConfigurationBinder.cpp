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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/AddressBook/AddressBookConfiguration.h>

static const char* TAG = "AddressBookConfigurationBinder";

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_addressbook_config_AddressBookConfiguration_createAddressBookConfigBinder(
    JNIEnv* env,
    jobject obj,
    jboolean cleanAllAddressBooksAtStart) {
    try {
        auto config =
            aace::addressBook::config::AddressBookConfiguration::createAddressBookConfig(cleanAllAddressBooksAtStart);
        ThrowIfNull(config, "createAddressBookConfig failed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return 0;
    }
}
}