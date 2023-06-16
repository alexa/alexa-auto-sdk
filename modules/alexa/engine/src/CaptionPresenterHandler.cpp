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
#include <cmath>
#include <iostream>
#include "AACE/Engine/Alexa/CaptionPresenterHandler.h"
#include "AACE/Engine/Core/EngineMacros.h"

#define PADDING_LENGTH 2

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.CaptionPresenter");

void CaptionPresenterHandler::captionsPrint(const std::vector<std::string>& lines) {
    size_t maxLength = 0;
    for (auto& line : lines) {
        maxLength = std::max(line.size(), maxLength);
    }

    // The line length should be even here to allow for alignment of the header string and '#' boundaries
    if (maxLength % 2 != 0) {
        maxLength += 1;
    }

    const std::string heading(" Caption ");  // keep the number of characters in this string even
    size_t maxBorderLength = (2 * PADDING_LENGTH) + std::max(maxLength, heading.length());
    size_t headerLength = std::ceil((maxBorderLength - heading.length()) / 2);

    const std::string headingBorder(headerLength, '#');
    std::ostringstream oss;
    oss << std::endl << headingBorder << heading << headingBorder << std::endl;

    // Write each line starting and ending with '#'
    auto padBegin = std::string("#");
    padBegin.append(PADDING_LENGTH - 1, ' ');
    for (auto& line : lines) {
        auto padEnd = std::string("#");
        padEnd.insert(padEnd.begin(), maxLength - line.size() + (PADDING_LENGTH - 1), ' ');
        oss << padBegin << line << padEnd << std::endl;
    }

    const std::string footingBorder(maxBorderLength, '#');
    oss << footingBorder << std::endl;
    AACE_INFO(LX(TAG, "captions").m(oss.str()));
}

void CaptionPresenterHandler::onCaptionActivity(
    const alexaClientSDK::captions::CaptionFrame& captionFrame,
    alexaClientSDK::avsCommon::avs::FocusState focus) {
    // Note: due to the nature of console-driven text output, two concessions are being made:
    //  - The only type of focus that is handled is FOREGROUND. BACKGROUND and NONE should also be handled.
    //  - Each CaptionLine object in the CaptionFrame contains style information which should be handled according to
    //    the presentation needs of the application, and in a way which matches with CaptionPresenter::getWrapIndex.
    if (alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND == focus) {
        std::vector<std::string> captionText;
        for (auto& line : captionFrame.getCaptionLines()) {
            captionText.emplace_back(line.text);
        }
        captionsPrint(captionText);

        std::string caption;
        for (auto& line : captionText) {
            line += " ";
            caption += line;
        }
        // Notify Caption Observer
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& next : m_observers) {
            next->onNewCaption(caption);
        }
    }
}

std::pair<bool, int> CaptionPresenterHandler::getWrapIndex(const alexaClientSDK::captions::CaptionLine& captionLine) {
    // This is a simplistic implementation that relies on the fixed-width characters of a console output.
    // A "real" implementation would apply the styles and measure the width of the text as shown on a screen to
    // determine if the text should wrap.

    // This lineWidth value is artificially small to demonstrate the line wrapping functionality.
    const size_t lineWidth = 30;
    if (captionLine.text.length() > lineWidth) {
        return std::make_pair(true, lineWidth);
    }
    return std::make_pair(false, 0);
}

void CaptionPresenterHandler::addObserver(std::shared_ptr<CaptionObserverInterface> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.insert(observer);
}

void CaptionPresenterHandler::removeObserver(std::shared_ptr<CaptionObserverInterface> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.erase(observer);
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace