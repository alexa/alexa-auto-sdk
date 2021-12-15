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

#ifndef SAMPLEAPP_VIEWS_H
#define SAMPLEAPP_VIEWS_H

// C++ Standard Library
#include <iostream>  // std::clog
#include <memory>    // std::shared_ptr
#include <mutex>     // std::mutex etc.
#include <sstream>   // std::stringstream
#include <string>    // std::string
#include <vector>    // std::vector

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  View
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class View {
private:
    std::string m_id{};
    std::string m_text{};
    static const std::string m_ruler;

protected:
    View(const std::string& id);

public:
    virtual ~View() = default;

    enum class Type {
        // CBL
        CBLCode,
        CBLCodeExpired,
        CBLAuthorizationExpired,

        // Communication
        CommunicationCallDisplayInfo,

        // Navigation/CancelNavigation
        // Navigation/StartNavigation
        Navigation,

        // TemplateRuntime/RenderTemplate
        Template,
        /*1*/ BodyTemplate1,
        /*2*/ BodyTemplate2,
        /*3*/ ListTemplate1,
        /*4*/ WeatherTemplate,
        /*5*/ LocalSearchListTemplate1,
        /*6*/ LocalSearchListTemplate2,
        /*7*/ LocalSearchDetailTemplate1,
        /*8*/ TrafficDetailsTemplate,

        // TemplateRuntime/RenderPlayerInfo
        PlayerInfo,

        // Authorization/authorizationStateChanged
        AuthorizationStateChanged,

        // Authorization/authorizationError
        AuthorizationError
    };
    static std::shared_ptr<View> create(const std::string& id);
    template <typename... Args>
    void print(Args&&... args) {
        std::stringstream stream;
        print(stream, args...);
    }
    template <typename Head>
    void print(std::stringstream& stream, Head head) {
        std::clog << head << stream.str();  // keep it buffered on purpose
    }
    template <typename Head, typename... Tail>
    void print(std::stringstream& stream, Head head, Tail... tail) {
        stream << head << ' ';
        print(stream, tail...);
    }
    template <typename... Args>
    void printLine(Args&&... args) {
        std::stringstream stream;
        printLine(stream, args...);
    }
    template <typename Head>
    void printLine(std::stringstream& stream, Head head) {
        stream << head << std::endl;
        std::clog << stream.str() << std::flush;  // make it line-buffered
    }
    template <typename Head, typename... Tail>
    void printLine(std::stringstream& stream, Head head, Tail... tail) {
        stream << head << ' ';
        printLine(stream, tail...);
    }
    void printRuler() {
        printLine(m_ruler);
    }
    virtual auto clear(Type type) -> void;
    virtual auto getId() -> std::string;
    virtual auto getText() -> std::string;
    virtual auto set(const std::string& string, Type type) -> void;
    virtual auto setId(const std::string& id) -> void;
    virtual auto setText(const std::string& text) -> void;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ContentView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ContentView : public View {
protected:
    ContentView(const std::string& id);

public:
    static std::shared_ptr<ContentView> create(const std::string& id);
    auto clear(Type type) -> void override;
    auto set(const std::string& string, Type type) -> void override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextView : public View {
protected:
    TextView(const std::string& id);

public:
    static std::shared_ptr<TextView> create(const std::string& id);
    auto setText(const std::string& text) -> void override;
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_VIEWS_H
