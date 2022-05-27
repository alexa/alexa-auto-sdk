/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Message/Navigation/Navigation/AnnounceManeuverMessage.h>
#include <AASB/Message/Navigation/Navigation/AnnounceRoadRegulationMessage.h>
#include <AASB/Message/Navigation/Navigation/CancelNavigationMessage.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplayMessage.h>
#include <AASB/Message/Navigation/Navigation/GetNavigationStateMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationErrorMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationEventMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigateToPreviousWaypointMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesSucceededMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowPreviousWaypointsMessage.h>
#include <AASB/Message/Navigation/Navigation/StartNavigationMessage.h>

#include <fstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

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

using MessageBroker = aace::core::MessageBroker;

NavigationHandler::NavigationHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    isOverrideActive = false;
    m_currentNavigationState = "";
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> NavigationHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> NavigationHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void NavigationHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to AnnounceManeuver Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAnnounceManeuverMessage(message); },
        AnnounceManeuverMessage::topic(),
        AnnounceManeuverMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AnnounceRoadRegulation Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAnnounceRoadRegulationMessage(message); },
        AnnounceRoadRegulationMessage::topic(),
        AnnounceRoadRegulationMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to CancelNavigation Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleCancelNavigationMessage(message); },
        CancelNavigationMessage::topic(),
        CancelNavigationMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ControlDisplay Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleControlDisplayMessage(message); },
        ControlDisplayMessage::topic(),
        ControlDisplayMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetNavigationState Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetNavigationStateMessage(message); },
        GetNavigationStateMessage::topic(),
        GetNavigationStateMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to NavigateToPreviousWaypoint Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleNavigateToPreviousWaypointMessage(message); },
        NavigateToPreviousWaypointMessage::topic(),
        NavigateToPreviousWaypointMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ShowAlternativeRoutes Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleShowAlternativeRoutesMessage(message); },
        ShowAlternativeRoutesMessage::topic(),
        ShowAlternativeRoutesMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ShowPreviousWaypoints Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleShowPreviousWaypointsMessage(message); },
        ShowPreviousWaypointsMessage::topic(),
        ShowPreviousWaypointsMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to StartNavigation Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStartNavigationMessage(message); },
        StartNavigationMessage::topic(),
        StartNavigationMessage::action());
}

void NavigationHandler::handleAnnounceManeuverMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AnnounceManeuverMessage");
    AnnounceManeuverMessage msg = json::parse(message);
    announceManeuver(msg.payload.payload);
}

void NavigationHandler::handleAnnounceRoadRegulationMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AnnounceRoadRegulationMessage");
    AnnounceRoadRegulationMessage msg = json::parse(message);
    announceRoadRegulation(msg.payload.roadRegulation);
}

void NavigationHandler::handleCancelNavigationMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received CancelNavigationMessage");
    cancelNavigation();
}

void NavigationHandler::handleControlDisplayMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ControlDisplayMessage");
    ControlDisplayMessage msg = json::parse(message);
    controlDisplay(msg.payload.controlDisplay);
}

void NavigationHandler::handleGetNavigationStateMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetNavigationStateMessage");

    // Publish the "GetNavigationStateReply" message
    GetNavigationStateMessage msg = json::parse(message);
    GetNavigationStateMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.navigationState = getNavigationState();
    m_messageBroker->publish(replyMsg.toString());
}

void NavigationHandler::handleNavigateToPreviousWaypointMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received NavigateToPreviousWaypointMessage");
    navigateToPreviousWaypoint();
}

void NavigationHandler::handleShowAlternativeRoutesMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Recevied ShowAlternativeRoutesMessage");
    ShowAlternativeRoutesMessage msg = json::parse(message);
    showAlternativeRoutes(msg.payload.alternateRouteType);
}

void NavigationHandler::handleShowPreviousWaypointsMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ShowPreviousWaypointsMessage");
    showPreviousWaypoints();
}

void NavigationHandler::handleStartNavigationMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StartNavigationMessage");
    StartNavigationMessage msg = json::parse(message);
    startNavigation(msg.payload.payload);
}

void NavigationHandler::navigationError(ErrorType type, ErrorCode code, const std::string& description) {
    // Publish the "NavigationError" message
    NavigationErrorMessage msg;
    msg.payload.type = type;
    msg.payload.code = code;
    msg.payload.description = description;
    m_messageBroker->publish(msg.toString());
}

void NavigationHandler::navigationEvent(EventName event) {
    // Publish the "NavigationEvent" message
    NavigationEventMessage msg;
    msg.payload.event = event;
    m_messageBroker->publish(msg.toString());
}

void NavigationHandler::showAlternativeRoutesSucceeded(const std::string& payload) {
    // Publish the "ShowAlternativeRoutesSucceeded" message
    ShowAlternativeRoutesSucceededMessage msg;
    msg.payload.payload = payload;
    m_messageBroker->publish(msg.toString());
}

void NavigationHandler::startNavigation(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "startNavigation");
    auto activity = m_activity.lock();
    if (!activity) {
        navigationError(ErrorType::NAVIGATION_START_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
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
            navigationEvent(EventName::NAVIGATION_STARTED);
        } else {
            navigationError(ErrorType::NAVIGATION_START_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
        }
    });
}

void NavigationHandler::navigateToPreviousWaypoint() {
    log(logger::LoggerHandler::Level::INFO, "navigateToPreviousWaypoint");
    if (previousDestinations.size() == 0) {
        log(logger::LoggerHandler::Level::ERROR, "Previous Destinations list is empty");
        navigationError(ErrorType::PREVIOUS_NAVIGATION_START_FAILED, ErrorCode::NO_PREVIOUS_WAYPOINTS, "");
        return;
    }
    auto activity = m_activity.lock();
    if (!activity) {
        navigationError(ErrorType::PREVIOUS_NAVIGATION_START_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
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
            navigationEvent(EventName::PREVIOUS_NAVIGATION_STARTED);
        } else {
            navigationError(ErrorType::PREVIOUS_NAVIGATION_START_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
        }
    });
}

void NavigationHandler::showPreviousWaypoints() {
    log(logger::LoggerHandler::Level::INFO, "showPreviousWaypoints");
    if (previousDestinations.size() == 0) {
        log(logger::LoggerHandler::Level::ERROR, "Previous Destinations list is empty ");
        navigationError(ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode::NO_PREVIOUS_WAYPOINTS, "");
        return;
    }
    auto activity = m_activity.lock();
    if (!activity) {
        log(logger::LoggerHandler::Level::ERROR, "Activity lock acquire failed");
        navigationError(ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(getPreviousWaypointsString(), View::Type::Navigation);
            navigationEvent(EventName::PREVIOUS_WAYPOINTS_SHOWN);
        } else {
            navigationError(ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode::INTERNAL_SERVICE_ERROR, "");
        }
    });
}

void NavigationHandler::showAlternativeRoutes(AlternateRouteType alternateRouteType) {
    log(logger::LoggerHandler::Level::INFO,
        "showAlternateRoutes:payload=" + getAlternateRouteTypeString(alternateRouteType));

    std::string alternateRouteTypeString = "";
    switch (alternateRouteType) {
        case (AlternateRouteType::DEFAULT):
            alternateRouteTypeString = "DEFAULT";
            break;
        case (AlternateRouteType::SHORTER_TIME):
            alternateRouteTypeString = "SHORTER_TIME";
            break;
        case (AlternateRouteType::SHORTER_DISTANCE):
            alternateRouteTypeString = "SHORTER_DISTANCE";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "showAlternativeRoutes:invalidAlternateRouteType");
    }

    std::string payload = "{\"inquiryType\": \"" + alternateRouteTypeString +
                          "\", \"alternateRoute\": {\"labels\": [\"US-101 N\"], \"savings\": [{\"type\":\"TIME\", "
                          "\"amount\": 12.0, \"unit\": \"MINUTE\"}]}}";
    showAlternativeRoutesSucceeded(payload);
}

void NavigationHandler::controlDisplay(ControlDisplay controlDisplay) {
    log(logger::LoggerHandler::Level::INFO, "controlDisplay:payload=" + getControlDisplayString(controlDisplay));

    EventName eventName;
    switch (controlDisplay) {
        case (ControlDisplay::SHOW_ROUTE_OVERVIEW):
            eventName = EventName::ROUTE_OVERVIEW_SHOWN;
            break;
        case (ControlDisplay::SHOW_DIRECTIONS_LIST):
            eventName = EventName::DIRECTIONS_LIST_SHOWN;
            break;
        case (ControlDisplay::ZOOM_IN):
            eventName = EventName::ZOOMED_IN;
            break;
        case (ControlDisplay::ZOOM_OUT):
            eventName = EventName::ZOOMED_OUT;
            break;
        case (ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION):
            eventName = EventName::MAP_CENTERED;
            break;
        case (ControlDisplay::ORIENT_NORTH):
            eventName = EventName::ORIENTED_NORTH;
            break;
        case (ControlDisplay::SCROLL_NORTH):
            eventName = EventName::SCROLLED_NORTH;
            break;
        case (ControlDisplay::SCROLL_UP):
            eventName = EventName::SCROLLED_UP;
            break;
        case (ControlDisplay::SCROLL_EAST):
            eventName = EventName::SCROLLED_EAST;
            break;
        case (ControlDisplay::SCROLL_RIGHT):
            eventName = EventName::SCROLLED_RIGHT;
            break;
        case (ControlDisplay::SCROLL_SOUTH):
            eventName = EventName::SCROLLED_SOUTH;
            break;
        case (ControlDisplay::SCROLL_DOWN):
            eventName = EventName::SCROLLED_DOWN;
            break;
        case (ControlDisplay::SCROLL_WEST):
            eventName = EventName::SCROLLED_WEST;
            break;
        case (ControlDisplay::SCROLL_LEFT):
            eventName = EventName::SCROLLED_LEFT;
            break;
        case (ControlDisplay::MUTE_ROUTE_GUIDANCE):
            eventName = EventName::ROUTE_GUIDANCE_MUTED;
            break;
        case (ControlDisplay::UNMUTE_ROUTE_GUIDANCE):
            eventName = EventName::ROUTE_GUIDANCE_UNMUTED;
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
    EventName eventName;
    if (type == "TURN") {
        eventName = EventName::TURN_GUIDANCE_ANNOUNCED;
    } else if (type == "EXIT") {
        eventName = EventName::EXIT_GUIDANCE_ANNOUNCED;
    } else if (type == "ENTER") {
        eventName = EventName::ENTER_GUIDANCE_ANNOUNCED;
    } else if (type == "MERGE") {
        eventName = EventName::MERGE_GUIDANCE_ANNOUNCED;
    } else if (type == "LANE") {
        eventName = EventName::LANE_GUIDANCE_ANNOUNCED;
    } else {
        log(logger::LoggerHandler::Level::ERROR, "announceManeuver:invalidManueverTypeValue");
        return;
    }
    navigationEvent(eventName);
}

void NavigationHandler::announceRoadRegulation(RoadRegulation roadRegulation) {
    log(logger::LoggerHandler::Level::INFO,
        "announceRoadRegulation:payload=" + getRoadRegulationString(roadRegulation));
    EventName eventName;
    switch (roadRegulation) {
        case RoadRegulation::CARPOOL_RULES:
            eventName = EventName::CARPOOL_RULES_REGULATION_ANNOUNCED;
            break;
        case RoadRegulation::SPEED_LIMIT:
            eventName = EventName::SPEED_LIMIT_REGULATION_ANNOUNCED;
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

std::string NavigationHandler::getControlDisplayString(ControlDisplay controlDisplay) {
    json payload;
    std::string mapControlValue;
    switch (controlDisplay) {
        case ControlDisplay::SHOW_ROUTE_OVERVIEW:
            mapControlValue = "SHOW_ROUTE_OVERVIEW";
            break;
        case ControlDisplay::SHOW_DIRECTIONS_LIST:
            mapControlValue = "SHOW_DIRECTIONS_LIST";
            break;
        case ControlDisplay::ZOOM_IN:
            mapControlValue = "ZOOM_IN";
            break;
        case ControlDisplay::ZOOM_OUT:
            mapControlValue = "ZOOM_OUT";
            break;
        case ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION:
            mapControlValue = "CENTER_MAP_ON_CURRENT_LOCATION";
            break;
        case ControlDisplay::ORIENT_NORTH:
            mapControlValue = "ORIENT_NORTH";
            break;
        case ControlDisplay::SCROLL_NORTH:
            mapControlValue = "SCROLL_NORTH";
            break;
        case ControlDisplay::SCROLL_UP:
            mapControlValue = "SCROLL_UP";
            break;
        case ControlDisplay::SCROLL_EAST:
            mapControlValue = "SCROLL_EAST";
            break;
        case ControlDisplay::SCROLL_RIGHT:
            mapControlValue = "SCROLL_RIGHT";
            break;
        case ControlDisplay::SCROLL_SOUTH:
            mapControlValue = "SCROLL_SOUTH";
            break;
        case ControlDisplay::SCROLL_DOWN:
            mapControlValue = "SCROLL_DOWN";
            break;
        case ControlDisplay::SCROLL_WEST:
            mapControlValue = "SCROLL_WEST";
            break;
        case ControlDisplay::SCROLL_LEFT:
            mapControlValue = "SCROLL_LEFT";
            break;
        case ControlDisplay::MUTE_ROUTE_GUIDANCE:
            mapControlValue = "MUTE_ROUTE_GUIDANCE";
            break;
        case ControlDisplay::UNMUTE_ROUTE_GUIDANCE:
            mapControlValue = "UNMUTE_ROUTE_GUIDANCE";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "getControlDisplayString:invalidControlDisplay");
            break;
    }
    payload["mode"] = mapControlValue;

    return payload.dump();
}

std::string NavigationHandler::getRoadRegulationString(RoadRegulation roadRegulation) {
    json payload;
    std::string roadRegulationValue;
    switch (roadRegulation) {
        case RoadRegulation::SPEED_LIMIT:
            roadRegulationValue = "SPEED_LIMIT";
            break;
        case RoadRegulation::CARPOOL_RULES:
            roadRegulationValue = "CARPOOL_RULES";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "getRoadRegulationString:invalidRoadRegulation");
            break;
    }
    payload["type"] = roadRegulationValue;

    return payload.dump();
}

std::string NavigationHandler::getAlternateRouteTypeString(AlternateRouteType alternateRouteType) {
    json payload;
    std::string alternatRouteTypeValue;
    switch (alternateRouteType) {
        case AlternateRouteType::DEFAULT:
            alternatRouteTypeValue = "DEFAULT";
            break;
        case AlternateRouteType::SHORTER_TIME:
            alternatRouteTypeValue = "SHORTER_TIME";
            break;
        case AlternateRouteType::SHORTER_DISTANCE:
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
