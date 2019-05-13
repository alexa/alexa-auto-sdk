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

#ifndef SAMPLEAPP_VIEWS_H
#define SAMPLEAPP_VIEWS_H

// C++ Standard Library
#include <iostream> // std::clog
#include <memory>   // std::shared_ptr
#include <mutex>    // std::mutex etc.
#include <string>   // std::string

namespace sampleApp {

static std::mutex g_clogMutex;

// Asynchronous clog output
struct aclog {
    std::unique_lock<std::mutex> lock;
    aclog() : lock(std::unique_lock<std::mutex>(g_clogMutex)) {}

    template <typename T> aclog &operator<<(const T &t) {
        std::clog << t;
        return *this;
    }

    aclog &operator<<(std::ostream &(*fp)(std::ostream &)) {
        std::clog << fp;
        return *this;
    }
};

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
    View(const std::string &id);

  public:
    enum class Type {
        // CBL
        CBLCode,
        CBLCodeExpired,

        // Navigation/CancelNavigation
        // Navigation/SetDestination
        Navigation,

        // TemplateRuntime/RenderTemplate
        Template,
        /*1*/ BodyTemplate1,
        /*2*/ BodyTemplate2,
        /*3*/ ListTemplate1,
        /*4*/ WeatherTemplate,
        /*5*/ LocalSearchListTemplate1,

        // TemplateRuntime/RenderPlayerInfo
        PlayerInfo
    };
    static std::shared_ptr<View> create(const std::string &id);
    template <typename Head> void print(Head head) { aclog() << head << std::flush; }
    template <typename Head, typename... Tail> void print(Head head, Tail... tail) {
        aclog() << head << ' ';
        print(tail...);
    }
    template <typename Head> void printLine(Head head) { aclog() << head << std::endl; }
    template <typename Head, typename... Tail> void printLine(Head head, Tail... tail) {
        aclog() << head << ' ';
        printLine(tail...);
    }
    void printRuler() { printLine(m_ruler); }
    virtual auto clear(Type type) -> void;
    virtual auto getId() -> std::string;
    virtual auto getText() -> std::string;
    virtual auto set(const std::string &string, Type type) -> void;
    virtual auto setId(const std::string &id) -> void;
    virtual auto setText(const std::string &text) -> void;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ContentView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ContentView : public View {
  protected:
    ContentView(const std::string &id);

  public:
    static std::shared_ptr<ContentView> create(const std::string &id);
    auto clear(Type type) -> void override;
    auto set(const std::string &string, Type type) -> void override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextView
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextView : public View {
  protected:
    TextView(const std::string &id);

  public:
    static std::shared_ptr<TextView> create(const std::string &id);
    auto setText(const std::string &text) -> void override;
};

} // namespace sampleApp

#endif // SAMPLEAPP_VIEWS_H
