/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Alexa/EqualizerControllerBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.EqualizerControllerBinder";

// type aliases
using EqualizerBandLevel = aace::jni::alexa::EqualizerControllerHandler::EqualizerBandLevel;
using EqualizerBand = aace::jni::alexa::EqualizerControllerHandler::EqualizerBand;

namespace aace {
namespace jni {
namespace alexa {

//
// EqualizerControllerBinder
//

EqualizerControllerBinder::EqualizerControllerBinder(jobject obj) {
    m_equalizerControllerHandler = std::make_shared<EqualizerControllerHandler>(obj);
}

//
// EqualizerControllerHandler
//

EqualizerControllerHandler::EqualizerControllerHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/alexa/EqualizerController") {
}

void EqualizerControllerHandler::setBandLevels(
    const std::vector<aace::alexa::EqualizerController::EqualizerBandLevel>& bandLevels) {
    try_with_context {
        auto equalizerBandLevel =
            aace::jni::native::JavaClass::find("com/amazon/aace/alexa/EqualizerController$EqualizerBandLevel");
        ThrowIfNull(equalizerBandLevel, "findClassFailed");

        JObjectArray arr(bandLevels.size(), equalizerBandLevel);
        ThrowIfNot(arr.isValid(), "invalidObjectArray");

        for (int j = 0; j < bandLevels.size(); j++) {
            jobject checkedBandObj;
            ThrowIfNot(JEqualizerBand::checkType(bandLevels[j].first, &checkedBandObj), "invalidBandType");

            jobject equalizerBandLevelObj = equalizerBandLevel->newInstance(
                "(Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;I)V", checkedBandObj, bandLevels[j].second);
            ThrowIfNull(equalizerBandLevelObj, "createEqualizerBandLevelFailed");

            ThrowIfNot(arr.setAt(j, equalizerBandLevelObj), "insertArrayElementFailed");
        }

        ThrowIfNot(
            m_obj.invoke<void>(
                "setBandLevels",
                "([Lcom/amazon/aace/alexa/EqualizerController$EqualizerBandLevel;)V",
                nullptr,
                arr.get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setBandLevels", ex.what());
    }
}

std::vector<EqualizerBandLevel> EqualizerControllerHandler::getBandLevels() {
    try_with_context {
        jobjectArray bandLevelsArrObj;
        ThrowIfNot(
            m_obj.invoke(
                "getBandLevels",
                "()[Lcom/amazon/aace/alexa/EqualizerController$EqualizerBandLevel;",
                (jobject*)&bandLevelsArrObj),
            "invokeMethodFailed");

        return JEqualizeBandLevel::convert(bandLevelsArrObj);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getBandLevels", ex.what());
        return {};
    }
}

//
// JEqualizeBandLevel
//

EqualizerBandLevel JEqualizeBandLevel::getBandLevel() {
    try_with_context {
        jobject bandObj;
        ThrowIfNot(
            invoke("getBand", "()Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;", &bandObj),
            "invokeMethodFailed");

        EqualizerBand checkedBandType;
        ThrowIfNot(JEqualizerBand::checkType(bandObj, &checkedBandType), "invalidBandType");

        jint level;
        ThrowIfNot(invoke("getLevel", "()I", &level), "invokeMethodFailed");

        return {checkedBandType, level};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getBandLevel", ex.what());
        return {};
    }
}

std::vector<EqualizerControllerHandler::EqualizerBandLevel> JEqualizeBandLevel::convert(jobjectArray bandLevelsArrObj) {
    try_with_context {
        // convert java band levels
        std::vector<EqualizerBandLevel> bandLevels;
        JObjectArray arr(bandLevelsArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            bandLevels.push_back(aace::jni::alexa::JEqualizeBandLevel(next).getBandLevel());
        }

        return bandLevels;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define EQUALIZER_CONTROLLER_BINDER(ref) reinterpret_cast<aace::jni::alexa::EqualizerControllerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_EqualizerController_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::EqualizerControllerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_EqualizerController_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto equalizerControllerBinder = EQUALIZER_CONTROLLER_BINDER(ref);
        ThrowIfNull(equalizerControllerBinder, "invalidEqualizerControllerBinder");
        delete equalizerControllerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_EqualizerController_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_EqualizerController_localSetBandLevels(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobjectArray arr) {
    try {
        auto equalizerControllerBinder = EQUALIZER_CONTROLLER_BINDER(ref);
        ThrowIfNull(equalizerControllerBinder, "invalidEqualizerControllerBinder");

        equalizerControllerBinder->getEqualizerController()->localSetBandLevels(
            aace::jni::alexa::JEqualizeBandLevel::convert(arr));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_EqualizerController_localSetBandLevels", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_EqualizerController_localAdjustBandLevels(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobjectArray arr) {
    try {
        auto equalizerControllerBinder = EQUALIZER_CONTROLLER_BINDER(ref);
        ThrowIfNull(equalizerControllerBinder, "invalidEqualizerControllerBinder");

        equalizerControllerBinder->getEqualizerController()->localAdjustBandLevels(
            aace::jni::alexa::JEqualizeBandLevel::convert(arr));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_EqualizerController_localAdjustBandLevels", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_EqualizerController_localResetBands(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobjectArray arr) {
    try {
        auto equalizerControllerBinder = EQUALIZER_CONTROLLER_BINDER(ref);
        ThrowIfNull(equalizerControllerBinder, "invalidEqualizerControllerBinder");

        if (arr == nullptr) {
            equalizerControllerBinder->getEqualizerController()->localResetBands();
        } else {
            std::vector<EqualizerBand> equalizerBands;
            JObjectArray bandsObjArr(arr);

            jobject bandObj;
            EqualizerBand band;

            for (int j = 0; j < bandsObjArr.size(); j++) {
                ThrowIfNot(bandsObjArr.getAt(j, &bandObj), "getArrayValueFailed");
                ThrowIfNot(aace::jni::alexa::JEqualizerBand::checkType(bandObj, &band), "invalidEqualizerBandType");

                equalizerBands.push_back(band);
            }

            equalizerControllerBinder->getEqualizerController()->localResetBands(equalizerBands);
        }
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_EqualizerController_localResetBands", ex.what());
    }
}
}
