/****************************************************************
Copyright (C)2013. OBIGO Inc. All rights reserved.
This software is covered by the license agreement between
the end user and OBIGO Inc. and may be
used and copied only in accordance with the terms of the
said agreement.
OBIGO Inc. assumes no responsibility or
liability for any errors or inaccuracies in this software,
or any consequential, incidental or indirect damage arising
out of the use of the software.
 ****************************************************************/

#ifndef AIDAEMON_IPC_HANDLER_H_
#define AIDAEMON_IPC_HANDLER_H_

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "SampleApp/Logger/LoggerHandler.h"

#include "SampleApp/VPA/AIDaemon-dbus-generated.h"
/* TODO
#include "SampleApp/InteractionManager.h"
*/

//using namespace alexaClientSDK::sampleApp;

namespace AIDAEMON {
class IPCHandler {

private:
    IPCHandler();
    IPCHandler(const IPCHandler& other);
    ~IPCHandler() {};
    void sendData(rapidjson::Document *ipcdataa);
    /* TODO
    void handleAudioControl(std::string data);
    void handleStartTTS(std::string data);
    void handleStopTTS(std::string data);    
    void updateConext(std::string data);
    */

    std::string getValueFromJson(json &data, std::string key);
    void recursive_mkdir(const char *path, mode_t mode);
 
public:
    static IPCHandler* GetInstance();
    static void OnBusAcquired(
            GDBusConnection * connection, 
            const gchar * name, gpointer 
            user_data);
    static gboolean on_handle_send_messages(
            AIDaemon *object,
            GDBusMethodInvocation *invocation,
            const gchar *arg_Data,
            gpointer user_data );
    bool makeDBusServer();

    void setLogger(std::weak_ptr<sampleApp::logger::LoggerHandler> loggerHandler) {m_loggerHandler = std::move(loggerHandler);}
    void log(sampleApp::logger::LoggerHandler::Level level, const std::string tag, const std::string &message);

    void sendMessage(std::string MethodID, rapidjson::Document *data);
    void sendMessage(std::string MethodID, std::string data);

    void sendAIStatus(std::string status = std::string(), std::string reason = std::string());
    /* TODO    
    void sendMessage(std::string MethodID, int data);
    void setAudioError(bool bError) {m_bAudioError = bError;};
    bool getAudioerror() {return m_bAudioError;};
    void setAuthCode(std::string code);
    void setConfigured(std::string data);
    void waitForConfiguration();

    void setInteractionManager(std::shared_ptr<InteractionManager> manager);
    std::shared_ptr<InteractionManager> getInteractionManager() {return m_interactionManager;};
    */

private:
    std::string m_aiStatus;
    std::string m_aiStatusReason;
    std::string m_authcode;
    bool        m_configured;
    std::condition_variable m_waitForConfigure;
    std::mutex  m_configureMtx;
public:
    static IPCHandler                   *instance;
    std::thread                         *m_pServerThread;
    guint                               m_idBus;
    GMainLoop                           *m_pLoop;
    AIDaemon                            *m_pDBusInterface = nullptr;
    AIDaemonSkeleton                    *m_pskeleton = nullptr;    
    //std::shared_ptr<InteractionManager> m_interactionManager;
    bool                                m_bAudioError = false;

    std::weak_ptr<sampleApp::logger::LoggerHandler> m_loggerHandler{};
};
}  // namespace AIDAEMON

#endif //AIDAEMON_IPC_HANDLER_H_
