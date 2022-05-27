/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_NAVIGATION_NAVIGATION_DISPLAY_MANAGER_ENUMS_H
#define AACE_ENGINE_NAVIGATION_NAVIGATION_DISPLAY_MANAGER_ENUMS_H

#include "AACE/Engine/Core/EngineMacros.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace navigation {

enum class DisplayMode {

    /**
     * A zoomed-out route overview for the active route.
     */
    SHOW_ROUTE_OVERVIEW,

    /**
     * A list of directions for the active route.
     */
    SHOW_DIRECTIONS_LIST,

    /**
     * Zoom in the map view.
     */
    ZOOM_IN,

    /**
     * Zoom out the map view.
     */
    ZOOM_OUT,

    /**
     * Center the map on the user's current location.
     */
    CENTER_MAP_ON_CURRENT_LOCATION,

    /**
     * Rotate the map to align the north cardinal direction up.
     */
    ORIENT_NORTH,

    /**
     * Scroll the map in the north cardinal direction.
     */
    SCROLL_NORTH,

    /**
     * Scroll the map up, relative to the orientation shown on screen.
     */
    SCROLL_UP,

    /**
     * Scroll the map in the east cardinal direction.
     */
    SCROLL_EAST,

    /**
     * Scroll the map right, relative to the orientation shown on screen.
     */
    SCROLL_RIGHT,

    /**
     * Scroll the map in the south cardinal direction.
     */
    SCROLL_SOUTH,

    /**
     * Scroll the map down, relative to the orientation shown on screen.
     */
    SCROLL_DOWN,

    /**
     * Scroll the map in the west cardinal direction.
     */
    SCROLL_WEST,

    /**
     * Scroll the map left, relative to the orientation shown on screen.
     */
    SCROLL_LEFT,

    /**
     * Mute the route guidance voice.
     */
    MUTE_ROUTE_GUIDANCE,

    /**
     * Un-mute the route guidance voice.
     */
    UNMUTE_ROUTE_GUIDANCE
};

enum class DisplayControlError {

    /**
     * Unexpected error.
     */
    INTERNAL_ERROR,

    /**
     * The requested operation is not supported.
     */
    NOT_SUPPORTED,

    /**
     * The requested operation is not allowed in the current state.
     */
    NOT_ALLOWED,

    /**
     * There is no active route.
     */
    NOT_NAVIGATING
};

enum class AlternativeRoutesQueryType {

    /**
     * Query for an alternative route with no preference for time or distance savings compared to the current route.
     */
    DEFAULT,

    /**
     * Query for an alternative route that saves travel time compared to the current route.
     */
    SHORTER_TIME,

    /**
     * Query for an alternative route that saves travel distance compared to the current route.
     */
    SHORTER_DISTANCE
};

enum class RouteSavingsType {
    /**
     * The route saves time compared to the current route.
     */
    TIME,

    /**
     * The route saves distance compared to the current route.
     */
    DISTANCE
};

enum class SavingsUnit {
    /**
     * The savings is measured in minutes.
     */
    MINUTE,

    /**
     * The savings is measured in hours.
     */
    HOUR,

    /**
     * The savings is measured in feet.
     */
    FOOT,

    /**
     * The savings is measured in yards.
     */
    YARD,

    /**
     * The savings is measured in miles.
     */
    MILE,

    /**
     * The savings is measured in meters.
     */
    METER,

    /**
     * The savings is measured in kilometers.
     */
    KILOMETER,
};

static std::string routeSavingsTypeToString(RouteSavingsType);
static std::string savingsUnitToString(SavingsUnit);

/**
 * Describes the savings afforded by a particular alternate route, relative to the current route.
 * All fields are required.
 */
struct RouteSavings {
    /// The type of savings.
    RouteSavingsType type;
    /// The amount saved.
    double amount;
    /// The unit of the savings.
    SavingsUnit unit;

    RouteSavings(RouteSavingsType savingsType, double savingsAmount, SavingsUnit savingsUnit) :
            type{savingsType}, amount{savingsAmount}, unit{savingsUnit} {};

    ~RouteSavings() = default;

    nlohmann::json toJson() const {
        nlohmann::json savingsJson = nlohmann::json();
        savingsJson["type"] = routeSavingsTypeToString(type);
        savingsJson["amount"] = amount;
        savingsJson["unit"] = savingsUnitToString(unit);
        return savingsJson;
    };
};

inline bool operator==(const RouteSavings& lhs, const RouteSavings& rhs) {
    return lhs.type == rhs.type && lhs.amount == rhs.amount && lhs.unit == rhs.unit;
}

/** 
 * Describes a particular alternate route. Alexa may speak the route details to the user. At least one label is
 * required (i.e., do not use an empty vector for "labels"). Savings descriptions are optional but should be provided
 * if the alternate route offers some savings relative to the current route.
 */
struct AlternateRoute {
    /// A list of labels describing the alternate route. E.g., ["101 North"]
    std::vector<std::string> labels;
    /// A list of savings descriptions, relative to the current route.
    std::vector<RouteSavings> savings;

    AlternateRoute(std::vector<std::string> routeLabels, std::vector<RouteSavings> routeSavings = {}) :
            labels{routeLabels}, savings{routeSavings} {};

    ~AlternateRoute() = default;

    nlohmann::json toJson() const {
        nlohmann::json routeJson = nlohmann::json();
        nlohmann::json labelsArray = nlohmann::json::array();
        nlohmann::json savingsArray = nlohmann::json::array();
        try {
            for (const auto& label : labels) {
                labelsArray.push_back(label);
            }
            for (const auto& saving : savings) {
                savingsArray.push_back(saving.toJson());
            }
            routeJson["labels"] = labelsArray;
            if (savingsArray.size() != 0) {
                routeJson["savings"] = savingsArray;
            }
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX("AlternateRoute").d("exception", ex.what()));
        }
        return routeJson;
    };

    std::string toJsonString() const {
        try {
            return toJson().dump();
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX("AlternateRoute").d("exception", ex.what()));
            return "";
        }
    };
};

inline bool operator==(const AlternateRoute& lhs, const AlternateRoute& rhs) {
    return lhs.labels == rhs.labels && lhs.savings == rhs.savings;
}

enum class AlternativeRoutesQueryError {

    /**
     * Unexpected error.
     */
    INTERNAL_ERROR,

    /**
     * The requested query is not supported.
     */
    NOT_SUPPORTED,

    /**
     * There is no active route.
     */
    NOT_NAVIGATING,

    /**
     * No routes matched the query. For example, the user is already on the best route.
     */
    NO_ROUTES_FOUND
};

// enum classes do not have built-in hash support in C++11
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
struct EnumHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

static std::unordered_map<DisplayMode, std::string, EnumHash> g_displayModeStrings = {
    {DisplayMode::SHOW_ROUTE_OVERVIEW, "SHOW_ROUTE_OVERVIEW"},
    {DisplayMode::SHOW_DIRECTIONS_LIST, "SHOW_DIRECTIONS_LIST"},
    {DisplayMode::ZOOM_IN, "ZOOM_IN"},
    {DisplayMode::ZOOM_OUT, "ZOOM_OUT"},
    {DisplayMode::CENTER_MAP_ON_CURRENT_LOCATION, "CENTER_MAP_ON_CURRENT_LOCATION"},
    {DisplayMode::ORIENT_NORTH, "ORIENT_NORTH"},
    {DisplayMode::SCROLL_NORTH, "SCROLL_NORTH"},
    {DisplayMode::SCROLL_UP, "SCROLL_UP"},
    {DisplayMode::SCROLL_EAST, "SCROLL_EAST"},
    {DisplayMode::SCROLL_RIGHT, "SCROLL_RIGHT"},
    {DisplayMode::SCROLL_SOUTH, "SCROLL_SOUTH"},
    {DisplayMode::SCROLL_DOWN, "SCROLL_DOWN"},
    {DisplayMode::SCROLL_WEST, "SCROLL_WEST"},
    {DisplayMode::SCROLL_LEFT, "SCROLL_LEFT"},
    {DisplayMode::MUTE_ROUTE_GUIDANCE, "MUTE_ROUTE_GUIDANCE"},
    {DisplayMode::UNMUTE_ROUTE_GUIDANCE, "UNMUTE_ROUTE_GUIDANCE"}};

static std::string displayModeToString(DisplayMode mode) {
    const auto& it = g_displayModeStrings.find(mode);
    if (it != g_displayModeStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, DisplayMode mode) {
    stream << displayModeToString(mode);
    return stream;
}

static std::unordered_map<DisplayControlError, std::string, EnumHash> g_displayControlErrorStrings = {
    {DisplayControlError::INTERNAL_ERROR, "INTERNAL_ERROR"},
    {DisplayControlError::NOT_SUPPORTED, "NOT_SUPPORTED"},
    {DisplayControlError::NOT_ALLOWED, "NOT_ALLOWED"},
    {DisplayControlError::NOT_NAVIGATING, "NOT_NAVIGATING"},
};

static std::string displayControlErrorToString(DisplayControlError error) {
    const auto& it = g_displayControlErrorStrings.find(error);
    if (it != g_displayControlErrorStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, DisplayControlError error) {
    stream << displayControlErrorToString(error);
    return stream;
}

static std::unordered_map<AlternativeRoutesQueryType, std::string, EnumHash> g_altRoutesQueryStrings = {
    {AlternativeRoutesQueryType::DEFAULT, "DEFAULT"},
    {AlternativeRoutesQueryType::SHORTER_TIME, "SHORTER_TIME"},
    {AlternativeRoutesQueryType::SHORTER_DISTANCE, "SHORTER_DISTANCE"}};

static std::string altRoutesQueryTypeToString(AlternativeRoutesQueryType type) {
    const auto& it = g_altRoutesQueryStrings.find(type);
    if (it != g_altRoutesQueryStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, AlternativeRoutesQueryType type) {
    stream << altRoutesQueryTypeToString(type);
    return stream;
}

static std::unordered_map<RouteSavingsType, std::string, EnumHash> g_routesSavingsTypeStrings = {
    {RouteSavingsType::TIME, "TIME"},
    {RouteSavingsType::DISTANCE, "DISTANCE"}};

static std::string routeSavingsTypeToString(RouteSavingsType type) {
    const auto& it = g_routesSavingsTypeStrings.find(type);
    if (it != g_routesSavingsTypeStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, RouteSavingsType type) {
    stream << routeSavingsTypeToString(type);
    return stream;
}

static std::unordered_map<SavingsUnit, std::string, EnumHash> g_savingsUnitStrings = {
    {SavingsUnit::MINUTE, "MINUTE"},
    {SavingsUnit::HOUR, "HOUR"},
    {SavingsUnit::FOOT, "FOOT"},
    {SavingsUnit::YARD, "YARD"},
    {SavingsUnit::MILE, "MILE"},
    {SavingsUnit::METER, "METER"},
    {SavingsUnit::KILOMETER, "KILOMETER"}};

static std::string savingsUnitToString(SavingsUnit type) {
    const auto& it = g_savingsUnitStrings.find(type);
    if (it != g_savingsUnitStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, const SavingsUnit& unit) {
    stream << savingsUnitToString(unit);
    return stream;
}

static std::unordered_map<AlternativeRoutesQueryError, std::string, EnumHash> g_altRoutesErrorStrings = {
    {AlternativeRoutesQueryError::INTERNAL_ERROR, "INTERNAL_ERROR"},
    {AlternativeRoutesQueryError::NOT_SUPPORTED, "NOT_SUPPORTED"},
    {AlternativeRoutesQueryError::NOT_NAVIGATING, "NOT_NAVIGATING"},
    {AlternativeRoutesQueryError::NO_ROUTES_FOUND, "NO_ROUTES_FOUND"}};

static std::string altRoutesErrorToString(AlternativeRoutesQueryError type) {
    const auto& it = g_altRoutesErrorStrings.find(type);
    if (it != g_altRoutesErrorStrings.end()) {
        return it->second;
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& stream, AlternativeRoutesQueryError error) {
    stream << altRoutesErrorToString(error);
    return stream;
}

}  // namespace navigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NAVIGATION_NAVIGATION_DISPLAY_MANAGER_ENUMS_H
