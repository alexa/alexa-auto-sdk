/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Navigation/NavigationHandler.h"
#include "SampleApp/ApplicationContext.h"

#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace navigation {

// max number of waypoints allowable in context
static const int MAXIMUM_WAYPOINTS_PREVIOUS_DESTINATION_LIST = 100;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NavigationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

NavigationHandler::NavigationHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    isOverrideActive = false;
    m_currentNavigationState = "";
    setupUI();
}

std::weak_ptr<Activity> NavigationHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> NavigationHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::navigation::Navigation interface
void NavigationHandler::startNavigation(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "startNavigation");
    auto activity = m_activity.lock();
    if (!activity) {
        navigationError(
            aace::navigation::NavigationEngineInterface::ErrorType::NAVIGATION_START_FAILED,
            aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
            "");
        return;
    }
    if (!isOverrideActive) {
        json navigationState = json::parse(payload);
        json currentState;
        currentState["state"] = "NAVIGATING";
        currentState["waypoints"] = navigationState.at("waypoints");
        currentState["shapes"] = json::array();
        m_currentNavigationState = currentState.dump();
    }
    isOverrideActive = false;

    //Updating Previous Destinations List
    updatePreviousDestinations(payload);
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(getStartNavigationPayloadString(payload), View::Type::Navigation);
            navigationEvent(aace::navigation::NavigationEngineInterface::EventName::NAVIGATION_STARTED);
        } else {
            navigationError(
                aace::navigation::NavigationEngineInterface::ErrorType::NAVIGATION_START_FAILED,
                aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
                "");
        }
    });
}

void NavigationHandler::navigateToPreviousWaypoint() {
    log(logger::LoggerHandler::Level::INFO, "navigateToPreviousWaypoint");
    if (previousDestinations.size() == 0) {
        log(logger::LoggerHandler::Level::ERROR, "Previous Destinations list is empty");
        navigationError(
            aace::navigation::NavigationEngineInterface::ErrorType::PREVIOUS_NAVIGATION_START_FAILED,
            aace::navigation::NavigationEngineInterface::ErrorCode::NO_PREVIOUS_WAYPOINTS,
            "");
        return;
    }
    auto activity = m_activity.lock();
    if (!activity) {
        navigationError(
            aace::navigation::NavigationEngineInterface::ErrorType::PREVIOUS_NAVIGATION_START_FAILED,
            aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
            "");
        return;
    }

    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            json previousWaypoint = json::parse(previousDestinations.front());
            json document;
            json entry = parseWaypoint(previousWaypoint);
            json previousDestinations = json::array();
            previousDestinations.push_back(entry);
            document["waypoints"] = previousDestinations;
            card->set(document.dump(), View::Type::Navigation);
            navigationEvent(aace::navigation::NavigationEngineInterface::EventName::PREVIOUS_NAVIGATION_STARTED);
        } else {
            navigationError(
                aace::navigation::NavigationEngineInterface::ErrorType::PREVIOUS_NAVIGATION_START_FAILED,
                aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
                "");
        }
    });
}

void NavigationHandler::showPreviousWaypoints() {
    log(logger::LoggerHandler::Level::INFO, "showPreviousWaypoints");
    if (previousDestinations.size() == 0) {
        log(logger::LoggerHandler::Level::ERROR, "Previous Destinations list is empty ");
        navigationError(
            aace::navigation::NavigationEngineInterface::ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED,
            aace::navigation::NavigationEngineInterface::ErrorCode::NO_PREVIOUS_WAYPOINTS,
            "");
        return;
    }
    auto activity = m_activity.lock();
    if (!activity) {
        log(logger::LoggerHandler::Level::ERROR, "Activity lock acquire failed");
        navigationError(
            aace::navigation::NavigationEngineInterface::ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED,
            aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
            "");
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(getPreviousWaypointsString(), View::Type::Navigation);
            navigationEvent(aace::navigation::NavigationEngineInterface::EventName::PREVIOUS_WAYPOINTS_SHOWN);
        } else {
            navigationError(
                aace::navigation::NavigationEngineInterface::ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED,
                aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR,
                "");
        }
    });
}

void NavigationHandler::showAlternativeRoutes(aace::navigation::Navigation::AlternateRouteType alternateRouteType) {
    log(logger::LoggerHandler::Level::INFO,
        "showAlternateRoutes:payload=" + getAlternateRouteTypeString(alternateRouteType));

    std::string alternateRouteTypeString = "";
    switch (alternateRouteType) {
        case (aace::navigation::Navigation::AlternateRouteType::DEFAULT):
            alternateRouteTypeString = "DEFAULT";
            break;
        case (aace::navigation::Navigation::AlternateRouteType::SHORTER_TIME):
            alternateRouteTypeString = "SHORTER_TIME";
            break;
        case (aace::navigation::Navigation::AlternateRouteType::SHORTER_DISTANCE):
            alternateRouteTypeString = "SHORTER_DISTANCE";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "showAlternativeRoutes:invalidAlternateRouteType");
    }

    std::string payload = "{\"inquiryType\": \"" + alternateRouteTypeString +
                          "\", \"alternateRoute\": {\"labels\": [\"US-101 N\"], \"savings\": [{\"type\":\"TIME\", "
                          "\"amount\": \"12.0\", \"unit\": \"MINUTE\"}]}}";
    showAlternativeRoutesSucceeded(payload);
}

void NavigationHandler::controlDisplay(ControlDisplay controlDisplay) {
    log(logger::LoggerHandler::Level::INFO, "controlDisplay:payload=" + getControlDisplayString(controlDisplay));

    aace::navigation::NavigationEngineInterface::EventName eventName;
    switch (controlDisplay) {
        case (aace::navigation::Navigation::ControlDisplay::SHOW_ROUTE_OVERVIEW):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ROUTE_OVERVIEW_SHOWN;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SHOW_DIRECTIONS_LIST):
            eventName = aace::navigation::NavigationEngineInterface::EventName::DIRECTIONS_LIST_SHOWN;
            break;
        case (aace::navigation::Navigation::ControlDisplay::ZOOM_IN):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ZOOMED_IN;
            break;
        case (aace::navigation::Navigation::ControlDisplay::ZOOM_OUT):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ZOOMED_OUT;
            break;
        case (aace::navigation::Navigation::ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION):
            eventName = aace::navigation::NavigationEngineInterface::EventName::MAP_CENTERED;
            break;
        case (aace::navigation::Navigation::ControlDisplay::ORIENT_NORTH):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ORIENTED_NORTH;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_NORTH):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_NORTH;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_UP):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_UP;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_EAST):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_EAST;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_RIGHT):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_RIGHT;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_SOUTH):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_SOUTH;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_DOWN):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_DOWN;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_WEST):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_WEST;
            break;
        case (aace::navigation::Navigation::ControlDisplay::SCROLL_LEFT):
            eventName = aace::navigation::NavigationEngineInterface::EventName::SCROLLED_LEFT;
            break;
        case (aace::navigation::Navigation::ControlDisplay::MUTE_ROUTE_GUIDANCE):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_MUTED;
            break;
        case (aace::navigation::Navigation::ControlDisplay::UNMUTE_ROUTE_GUIDANCE):
            eventName = aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_UNMUTED;
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "controlDisplay:invalidControlDisplayValue");
            return;
    }
    navigationEvent(eventName);
}

void NavigationHandler::announceManeuver(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "announceManeuver:payload=" + payload);
    json announceManeueverPayload = json::parse(payload);
    std::string type = announceManeueverPayload.at("type");
    aace::navigation::NavigationEngineInterface::EventName eventName;
    if (type == "TURN") {
        eventName = aace::navigation::NavigationEngineInterface::EventName::TURN_GUIDANCE_ANNOUNCED;
    } else if (type == "EXIT") {
        eventName = aace::navigation::NavigationEngineInterface::EventName::EXIT_GUIDANCE_ANNOUNCED;
    } else if (type == "ENTER") {
        eventName = aace::navigation::NavigationEngineInterface::EventName::ENTER_GUIDANCE_ANNOUNCED;
    } else if (type == "MERGE") {
        eventName = aace::navigation::NavigationEngineInterface::EventName::MERGE_GUIDANCE_ANNOUNCED;
    } else if (type == "LANE") {
        eventName = aace::navigation::NavigationEngineInterface::EventName::LANE_GUIDANCE_ANNOUNCED;
    } else {
        log(logger::LoggerHandler::Level::ERROR, "announceManeuver:invalidManueverTypeValue");
        return;
    }
    navigationEvent(eventName);
}

void NavigationHandler::announceRoadRegulation(aace::navigation::Navigation::RoadRegulation roadRegulation) {
    log(logger::LoggerHandler::Level::INFO,
        "announceRoadRegulation:payload=" + getRoadRegulationString(roadRegulation));
    aace::navigation::NavigationEngineInterface::EventName eventName;
    switch (roadRegulation) {
        case aace::navigation::Navigation::RoadRegulation::CARPOOL_RULES:
            eventName = aace::navigation::NavigationEngineInterface::EventName::CARPOOL_RULES_REGULATION_ANNOUNCED;
            break;
        case aace::navigation::Navigation::RoadRegulation::SPEED_LIMIT:
            eventName = aace::navigation::NavigationEngineInterface::EventName::SPEED_LIMIT_REGULATION_ANNOUNCED;
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "announceRoadRegulation:invalidRoadRegulationValue");
            return;
    }
    navigationEvent(eventName);
}

bool NavigationHandler::cancelNavigation() {
    log(logger::LoggerHandler::Level::INFO, "cancelNavigation");
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }

    isOverrideActive = false;

    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->clear(View::Type::Navigation);
        }
    });
    return clearNavigationState();
}

// private

std::string NavigationHandler::getNavigationState() {
    log(logger::LoggerHandler::Level::INFO, "getNavigationState");
    return m_currentNavigationState;
}

// Sample App Events

bool NavigationHandler::loadNavigationState(const std::string& filepath) {
    std::ifstream i(filepath);
    json j;
    i >> j;

    if (j.is_object()) {
        auto waypoints = j.find("waypoints");
        if (waypoints != j.end() && waypoints->is_array()) {
            m_currentNavigationState = j.dump();
            updatePreviousDestinations(m_currentNavigationState);
            return true;
        } else {
            log(logger::LoggerHandler::Level::ERROR, "Cannot find json array with key of 'waypoints' ");
            return false;
        }
    } else {
        log(logger::LoggerHandler::Level::ERROR, "Navigation state data invalid");
        return false;
    }
}

bool NavigationHandler::clearNavigationState() {
    m_currentNavigationState = "";
    log(logger::LoggerHandler::Level::INFO, "Navigation state data cleared");
    return true;
}

void NavigationHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "NavigationHandler", message);
}

void NavigationHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    m_console = activity->findViewById("id:console");

    activity->registerObserver(Event::onLoadNavigationState, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onLoadNavigationState");
        if (auto console = m_console.lock()) {
            console->printLine("Loading from" + value);
        }
        isOverrideActive = true;
        return loadNavigationState(value);
    });

    activity->registerObserver(Event::onClearNavigationState, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onClearNavigationState");
        if (auto console = m_console.lock()) {
            console->printLine("Clearing nav state");
        }
        return clearNavigationState();
    });
}

// Helper functions

void NavigationHandler::updatePreviousDestinations(const std::string& payload) {
    json document = json::parse(payload);
    json waypoints = json::array();
    waypoints = document.at("waypoints");
    for (int j = waypoints.size() - 1; j >= 0; j--) {
        previousDestinations.insert(previousDestinations.begin(), waypoints[j].dump());
    }
    if (previousDestinations.size() > MAXIMUM_WAYPOINTS_PREVIOUS_DESTINATION_LIST) {
        previousDestinations.erase(
            previousDestinations.begin() + MAXIMUM_WAYPOINTS_PREVIOUS_DESTINATION_LIST, previousDestinations.end());
    }
}

std::string NavigationHandler::getStartNavigationPayloadString(const std::string& payload) {
    json document;
    json destination;
    json payloads = json::parse(payload);
    auto wayPoints = json::array();
    for (auto point : payloads.at("waypoints")) {
        json entry = parseWaypoint(point);

        if (point.at("type").get<std::string>() == "DESTINATION") {
            document["destination"] = entry;
        } else if (point.at("type").get<std::string>() == "INTERIM") {
            wayPoints.push_back(entry);
        }
    }
    document["waypoints"] = wayPoints;
    return document.dump();
}

std::string NavigationHandler::getPreviousWaypointsString() {
    json document;
    json wayPoints = json::array();
    for (auto prevDest : previousDestinations) {
        json point = json::parse(prevDest);
        json entry = parseWaypoint(point);
        wayPoints.push_back(entry);
    }
    document["waypoints"] = wayPoints;
    return document.dump();
}

json NavigationHandler::parseWaypoint(json point) {
    json entry;
    json coordinate;
    std::string address = "";
    if (point.find("address") != point.end()) {
        address = constructAddressString(point.at("address"));
    }
    entry["address"] = address;
    coordinate["latitudeInDegrees"] = point.at("coordinate")[0].get<double>();
    coordinate["longitudeInDegrees"] = point.at("coordinate")[1].get<double>();
    entry["name"] = point.find("name") != point.end() ? point.at("name").get<std::string>() : "";
    entry["coordinate"] = coordinate;
    return entry;
}

std::string NavigationHandler::constructAddressString(json address) {
    std::string city = address.find("city") != address.end() ? address.at("city").get<std::string>() : "";
    std::string addressLine1 =
        address.find("addressLine1") != address.end() ? address.at("addressLine1").get<std::string>() : "";
    std::string addressLine2 =
        address.find("addressLine2") != address.end() ? address.at("addressLine2").get<std::string>() : "";
    std::string addressLine3 =
        address.find("addressLine3") != address.end() ? address.at("addressLine3").get<std::string>() : "";
    std::string stateOrRegion =
        address.find("stateOrRegion") != address.end() ? address.at("stateOrRegion").get<std::string>() : "";
    std::string postalCode =
        address.find("postalCode") != address.end() ? address.at("postalCode").get<std::string>() : "";

    return addressLine1 + " " + addressLine2 + " " + addressLine3 + " " + city + " " + stateOrRegion + " " + postalCode;
}

std::string NavigationHandler::getControlDisplayString(aace::navigation::Navigation::ControlDisplay controlDisplay) {
    json payload;
    std::string mapControlValue;
    switch (controlDisplay) {
        case aace::navigation::Navigation::ControlDisplay::SHOW_ROUTE_OVERVIEW:
            mapControlValue = "SHOW_ROUTE_OVERVIEW";
            break;
        case aace::navigation::Navigation::ControlDisplay::SHOW_DIRECTIONS_LIST:
            mapControlValue = "SHOW_DIRECTIONS_LIST";
            break;
        case aace::navigation::Navigation::ControlDisplay::ZOOM_IN:
            mapControlValue = "ZOOM_IN";
            break;
        case aace::navigation::Navigation::ControlDisplay::ZOOM_OUT:
            mapControlValue = "ZOOM_OUT";
            break;
        case aace::navigation::Navigation::ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION:
            mapControlValue = "CENTER_MAP_ON_CURRENT_LOCATION";
            break;
        case aace::navigation::Navigation::ControlDisplay::ORIENT_NORTH:
            mapControlValue = "ORIENT_NORTH";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_NORTH:
            mapControlValue = "SCROLL_NORTH";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_UP:
            mapControlValue = "SCROLL_UP";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_EAST:
            mapControlValue = "SCROLL_EAST";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_RIGHT:
            mapControlValue = "SCROLL_RIGHT";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_SOUTH:
            mapControlValue = "SCROLL_SOUTH";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_DOWN:
            mapControlValue = "SCROLL_DOWN";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_WEST:
            mapControlValue = "SCROLL_WEST";
            break;
        case aace::navigation::Navigation::ControlDisplay::SCROLL_LEFT:
            mapControlValue = "SCROLL_LEFT";
            break;
        case aace::navigation::Navigation::ControlDisplay::MUTE_ROUTE_GUIDANCE:
            mapControlValue = "MUTE_ROUTE_GUIDANCE";
            break;
        case aace::navigation::Navigation::ControlDisplay::UNMUTE_ROUTE_GUIDANCE:
            mapControlValue = "UNMUTE_ROUTE_GUIDANCE";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "getControlDisplayString:invalidControlDisplay");
            break;
    }
    payload["mode"] = mapControlValue;

    return payload.dump();
}

std::string NavigationHandler::getRoadRegulationString(aace::navigation::Navigation::RoadRegulation roadRegulation) {
    json payload;
    std::string roadRegulationValue;
    switch (roadRegulation) {
        case aace::navigation::Navigation::RoadRegulation::SPEED_LIMIT:
            roadRegulationValue = "SPEED_LIMIT";
            break;
        case aace::navigation::Navigation::RoadRegulation::CARPOOL_RULES:
            roadRegulationValue = "CARPOOL_RULES";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "getRoadRegulationString:invalidRoadRegulation");
            break;
    }
    payload["type"] = roadRegulationValue;

    return payload.dump();
}

std::string NavigationHandler::getAlternateRouteTypeString(
    aace::navigation::Navigation::AlternateRouteType alternateRouteType) {
    json payload;
    std::string alternatRouteTypeValue;
    switch (alternateRouteType) {
        case aace::navigation::Navigation::AlternateRouteType::DEFAULT:
            alternatRouteTypeValue = "DEFAULT";
            break;
        case aace::navigation::Navigation::AlternateRouteType::SHORTER_TIME:
            alternatRouteTypeValue = "SHORTER_TIME";
            break;
        case aace::navigation::Navigation::AlternateRouteType::SHORTER_DISTANCE:
            alternatRouteTypeValue = "SHORTER_DISTANCE";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "getAlternateRouteTypeString:invalidAlternateRouteType");
            break;
    }
    payload["mode"] = alternatRouteTypeValue;

    return payload.dump();
}

}  // namespace navigation
}  // namespace sampleApp
