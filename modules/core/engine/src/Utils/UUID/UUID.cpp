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

#include <AACE/Engine/Utils/UUID/UUID.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <climits>
#include <algorithm>
#include <functional>

namespace aace {
namespace engine {
namespace utils {
namespace uuid {

/// The UUID version (Version 4), shifted into the correct position in the byte.
static const uint8_t UUID_VERSION_VALUE = 4 << 4;

/// The UUID variant (Variant 1), shifted into the correct position in the byte.
static const size_t UUID_VARIANT_VALUE = 2 << 6;

/// Separator used between UUID fields.
static const std::string SEPARATOR("-");

/// Number of bits in the replacement value.
static const size_t MAX_NUM_REPLACEMENT_BITS = CHAR_BIT;

/// Number of bits in a hex digit.
static const size_t BITS_IN_HEX_DIGIT = 4;

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.utils.uuid.UUID");

/**
 * Randomly generate a string of hex digits. Before the conversion of hex to string,
 * the function allows replacement of the bits of the first two generated hex digits.
 * Replacement happens starting at the most significant, to the least significant bit.
 *
 * @param ibe A random number generator.
 * @param numDigits The number of hex digits [0-9],[a-f] to generate.
 * @param replacementBits The replacement value for up to the first two digits generated.
 * @param numReplacementBits The number of bits of @c replacementBits to use.
 *
 * @return A hex string of length @c numDigits.
 */
static const std::string generateHexWithReplacement(
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t>& ibe,
    unsigned int numDigits,
    uint8_t replacementBits,
    unsigned short numReplacementBits) {
    try {
        ThrowIf(numReplacementBits > MAX_NUM_REPLACEMENT_BITS, "replacingMoreBitsThanProvided");
        ThrowIf(numReplacementBits > (numDigits * BITS_IN_HEX_DIGIT), "replacingMoreBitsThanGenerated");

        // Makes assumption that 1 digit = 4 bits.
        std::vector<uint8_t> bytes(ceil(numDigits / 2.0));
        std::generate(bytes.begin(), bytes.end(), std::ref(ibe));

        // Replace the specified number of bits from the first byte.
        bytes.at(0) &= (0xff >> numReplacementBits);
        replacementBits &= (0xff << (MAX_NUM_REPLACEMENT_BITS - numReplacementBits));
        bytes.at(0) |= replacementBits;

        std::ostringstream oss;
        for (const auto& byte : bytes) {
            oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
        }

        std::string bytesText = oss.str();
        // Remove the last digit for odd numDigits case.
        bytesText.resize(numDigits);

        return bytesText;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "generateHexWithReplacement").d("reason", ex.what()));
        return std::string();
    }
}

/**
 * Randomly generate a string of hex digits.
 *
 * @param ibe A random number generator.
 * @param numDigits The number of hex digits [0-9],[a-f] to generate.
 *
 * @return A hex string of length @c numDigits.
 */
static const std::string generateHex(
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t>& ibe,
    unsigned int numDigits) {
    return generateHexWithReplacement(ibe, numDigits, 0, 0);
}

const std::string generateUUID() {
    static bool seeded = false;
    static std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> ibe;
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    if (!seeded) {
        std::random_device rd;
        ibe.seed(
            rd() +
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());
        seeded = true;
    }

    std::ostringstream uuidText;
    uuidText << generateHex(ibe, 8) << SEPARATOR << generateHex(ibe, 4) << SEPARATOR
             << generateHexWithReplacement(ibe, 4, UUID_VERSION_VALUE, 4) << SEPARATOR
             << generateHexWithReplacement(ibe, 4, UUID_VARIANT_VALUE, 2) << SEPARATOR << generateHex(ibe, 12);

    lock.unlock();

    return uuidText.str();
}

bool compare(const std::string& uuid1, const std::string& uuid2) {
    return std::equal(uuid1.begin(), uuid1.end(), uuid2.begin(), [](unsigned char a, unsigned char b) -> bool {
        return std::toupper(a) == std::toupper(b);
    });
}

}  // namespace uuid
}  // namespace utils
}  // namespace engine
}  // namespace aace
