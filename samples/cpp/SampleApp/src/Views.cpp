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

#include "SampleApp/Views.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  View
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string View::m_ruler = "\n" + std::string(80, '#') + "\n";

View::View(const std::string &id) : m_id{std::move(id)} {}

std::shared_ptr<View> View::create(const std::string &id) { return std::shared_ptr<View>(new View(id)); }

void View::clear(const Type type) {}

std::string View::getId() { return m_id; }

std::string View::getText() { return m_text; }

void View::set(const std::string &, const Type type) {}

void View::setId(const std::string &id) { m_id = id; }

void View::setText(const std::string &text) { m_text = text; }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ContentView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ContentView::ContentView(const std::string &id) : View{id} {}

std::shared_ptr<ContentView> ContentView::create(const std::string &id) { return std::shared_ptr<ContentView>(new ContentView(id)); }

void ContentView::clear(const Type type) {
    switch (type) {
        case Type::Navigation:
            printLine("Navigation canceled");
            break;
        case Type::Template:
            printLine("Template cleared");
            break;
        case Type::PlayerInfo:
            printLine("PlayerInfo cleared");
            break;
        default:
            break;
    }
}

void ContentView::set(const std::string &string, const Type type) {
    static const unsigned menuColumns = 80;
    auto titleRuler = std::string(menuColumns, '#');
    std::stringstream stream;
    stream << std::endl << titleRuler << std::endl << std::endl;
    switch (type) {
        case Type::CBLCode:
            try {
                auto object = json::parse(string);
                Ensures(object.is_object());
                if (object.count("code")) {
                    stream << "Code: " << object.at("code").get<std::string>() << std::endl;
                }
                if (object.count("url")) {
                    stream << "URL:  " << object.at("url").get<std::string>() << std::endl;
                }
            } catch (std::exception &e) {
                stream << "CBLCode parser error" << e.what() << std::endl;
            }
            break;
        case Type::CBLCodeExpired:
            try {
                auto object = json::parse(string);
                Ensures(object.is_object());
                if (object.count("message")) {
                    stream << object.at("message").get<std::string>() << std::endl;
                }
            } catch (std::exception &e) {
                stream << "CBLCodeExpired parser error" << e.what() << std::endl;
            }
            break;
        case Type::Navigation:
            try {
                auto object = json::parse(string);
                Ensures(object.is_object());
                if (object.count("destination")) {
                    auto destination = object.at("destination");
                    stream << "Address:   " << destination.at("singleLineDisplayAddress").get<std::string>() << std::endl;
                    if (destination.count("coordinate")) {
                        auto coordinate = destination.at("coordinate");
                        stream << "Latitude:  " << coordinate.at("latitudeInDegrees").get<double>() << std::endl;
                        stream << "Longitude: " << coordinate.at("longitudeInDegrees").get<double>() << std::endl;
                    }
                }
            } catch (std::exception &e) {
                stream << "Navigation parser error" << e.what() << std::endl;
            }
            break;
        case Type::Template:
            try {
                auto object = json::parse(string);
                Ensures(object.is_object());
                if (object.count("title")) {
                    auto title = object.at("title");
                    if (title.count("mainTitle")) {
                        stream << title.at("mainTitle").get<std::string>() << std::endl;
                    }
                    if (title.count("subTitle")) {
                        stream << title.at("subTitle").get<std::string>() << std::endl;
                    }
                    stream << std::endl;
                }
                // clang-format off
                static const std::map<std::string, Type> TemplateTypeEnumerator{
                    {"BodyTemplate1", Type::BodyTemplate1},
                    {"BodyTemplate2", Type::BodyTemplate2},
                    {"ListTemplate1", Type::ListTemplate1},
                    {"WeatherTemplate", Type::WeatherTemplate},
                    {"LocalSearchListTemplate1", Type::LocalSearchListTemplate1}
                };
                // clang-format on
                switch (TemplateTypeEnumerator.at(object["type"])) {
                    case Type::BodyTemplate1:
                        if (object.count("textField")) {
                            stream << object.at("textField").get<std::string>() << std::endl;
                        }
                        break;
                    case Type::BodyTemplate2:
                        if (object.count("textField")) {
                            stream << object.at("textField").get<std::string>() << std::endl;
                        }
                        if (object.count("image")) {
                            auto image = object.at("image");
                            if (image.count("contentDescription")) {
                                stream << image.at("contentDescription").get<std::string>() << std::endl;
                            }
                        }
                        break;
                    case Type::ListTemplate1:
                        if (object.count("listItems") && object.at("listItems").is_array()) {
                            for (auto &item : object.at("listItems")) {
                                if (item.count("leftTextField")) {
                                    stream << item.at("leftTextField").get<std::string>() << " ";
                                }
                                if (item.count("rightTextField")) {
                                    stream << item.at("rightTextField").get<std::string>() << " ";
                                }
                                stream << std::endl;
                            }
                        }
                        break;
                    case Type::WeatherTemplate:
                        if (object.count("currentWeather")) {
                            stream << "Current Weather:  " << object.at("currentWeather").get<std::string>() << std::endl;
                        }
                        if (object.count("description")) {
                            stream << "Description:      " << object.at("description").get<std::string>() << std::endl;
                        }
                        if (object.count("highTemperature")) {
                            stream << "High Temperature: " << object.at("highTemperature").at("value").get<std::string>() << std::endl;
                        }
                        if (object.count("lowTemperature")) {
                            stream << "Low Temperature:  " << object.at("lowTemperature").at("value").get<std::string>() << std::endl;
                        }
                        if (object.count("weatherForecast") && object.at("weatherForecast").is_array()) {
                            stream << std::endl << "Weather Forecast: " << std::endl;
                            for (auto &item : object.at("weatherForecast")) {
                                stream << item.at("date").get<std::string>() << ", " << item.at("day").get<std::string>() << ", "
                                       << item.at("highTemperature").get<std::string>() << " - " << item.at("lowTemperature").get<std::string>() << ", "
                                       << item.at("image").at("contentDescription").get<std::string>() << std::endl;
                            }
                        }
                        break;
                    case Type::LocalSearchListTemplate1:
                        if (object.count("listItems") && object.at("listItems").is_array()) {
                            for (auto &item : object.at("listItems")) {
                                if (item.count("leftTextField")) {
                                    stream << item.at("leftTextField").get<std::string>() << " ";
                                }
                                if (item.count("rightPrimaryTextField")) {
                                    stream << item.at("rightPrimaryTextField").get<std::string>() << " ";
                                }
                                if (item.count("rightSecondaryTextField")) {
                                    stream << item.at("rightSecondaryTextField").get<std::string>() << " ";
                                }
                                stream << std::endl;
                            }
                        }
                        break;
                    default:
                        break;
                }
            } catch (std::exception &e) {
                stream << "Template parser error" << e.what() << std::endl;
            }
            break;
        case Type::PlayerInfo:
            try {
                auto object = json::parse(string);
                Ensures(object.is_object());
                if (object.count("content")) {
                    auto content = object.at("content");
                    if (content.count("title")) {
                        stream << content.at("title").get<std::string>() << std::endl;
                    }
                    if (content.count("titleSubtext1")) {
                        stream << content.at("titleSubtext1").get<std::string>() << std::endl;
                    }
                    if (content.count("titleSubtext2")) {
                        stream << content.at("titleSubtext2").get<std::string>() << std::endl;
                    }
                    stream << std::endl;
                    if (content.count("header")) {
                        stream << content.at("header").get<std::string>() << std::endl;
                    }
                    if (content.count("headerSubtext1")) {
                        stream << content.at("headerSubtext1").get<std::string>() << std::endl;
                    } else if (content.count("provider")) {
                        auto provider = content.at("provider");
                        if (provider.count("name")) {
                            stream << provider.at("name").get<std::string>() << std::endl;
                        }
                    }
                    if (content.count("art")) {
                        auto art = content.at("art");
                        if (art.count("contentDescription")) {
                            stream << art.at("contentDescription").get<std::string>() << std::endl;
                        }
                    }
                }
            } catch (std::exception &e) {
                stream << "PlayerInfo parser error" << e.what() << std::endl;
            }
            break;
        default:
            break;
    }
    stream << std::endl << titleRuler << std::endl << std::endl;
    print(stream.str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TextView::TextView(const std::string &id) : View{id} {}

std::shared_ptr<TextView> TextView::create(const std::string &id) { return std::shared_ptr<TextView>(new TextView(id)); }

void TextView::setText(const std::string &text) {
    View::setText(text); // i.e. ncurses
}

} // namespace sampleApp
