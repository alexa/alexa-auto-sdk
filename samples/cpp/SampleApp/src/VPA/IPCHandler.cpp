/*
Copyright (C)2013. OBIGO Inc. All rights reserved.
This software is covered by the license agreement between
the end user and OBIGO Inc. and may be
used and copied only in accordance with the terms of the
said agreement.
OBIGO Inc. assumes no responsibility or
liability for any errors or inaccuracies in this software,
or any consequential, incidental or indirect damage arising
out of the use of the software.
 */

#include "SampleApp/VPA/IPCHandler.h"
#include "SampleApp/VPA/base64.h"
#include "SampleApp/VPA/AIDaemon-IPC.h"

#include "SampleApp/Event.h"

/* TODO
#include "SampleApp/ConsolePrinter.h"

#include <AVSCommon/AVS/NamespaceAndName.h>
#include <ContextManager/ContextManager.h>
*/

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>

using Level = sampleApp::logger::LoggerHandler::Level;

namespace AIDAEMON {

/* TODO
using namespace alexaClientSDK::sampleApp;
*/

IPCHandler* IPCHandler::instance = nullptr;

IPCHandler::IPCHandler() {
    m_pServerThread = nullptr;
    m_idBus = 0;
    m_pLoop = nullptr;
    m_pDBusInterface = nullptr;
    m_pskeleton = nullptr;

    m_aiStatus = "";
    m_aiStatusReason = "";
    m_authcode = "";
    m_configured = false;
}

/* TODO
void IPCHandler::setInteractionManager(std::shared_ptr<InteractionManager> manager) {
    m_interactionManager = manager;
}
*/

IPCHandler* IPCHandler::GetInstance() {
    if (instance == nullptr) {
        //ConsolePrinter::simplePrint("Create IPCHandler");
        instance = new IPCHandler();
    }
    
    return instance;
}

void IPCHandler::OnBusAcquired(GDBusConnection * connection, const gchar * name, gpointer user_data) {
    //ConsolePrinter::simplePrint(__PRETTY_FUNCTION__);

    IPCHandler* handler = IPCHandler::GetInstance();

    std::stringstream logbuffer;
    logbuffer << "DBus Connection : " << connection;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    logbuffer << "DBus Name : " << name;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    if (handler->m_pDBusInterface != nullptr) {
        //ConsolePrinter::simplePrint("DBus Interface is not NULL");
        return;
    }

    if (handler->m_pskeleton != nullptr) {
        //ConsolePrinter::simplePrint("Skeleton is not Null");
        return;
    }

    handler->m_pDBusInterface = aidaemon__skeleton_new() ;
    logbuffer << "DBus skeleton : " << handler->m_pDBusInterface;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    gulong handlerID = g_signal_connect (
        handler->m_pDBusInterface, 
        "handle-send-messages", 
        G_CALLBACK (IPCHandler::on_handle_send_messages), 
        nullptr);
    logbuffer << "DBus HandlerID : " << handlerID;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    handler->m_pskeleton = AIDAEMON__SKELETON (handler->m_pDBusInterface);
    logbuffer << "AIDaemon skeleton : " << handler->m_pskeleton;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    gboolean result = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(handler->m_pskeleton),
        connection,
        AIDAEMON::SERVER_OBJECT.c_str(),
        nullptr);
    logbuffer << "Export Result : " << result;
    //ConsolePrinter::simplePrint(logbuffer.str());
    logbuffer.str("");

    handler->sendAIStatus(AIDAEMON::AI_STATUS_READY);
}

bool IPCHandler::makeDBusServer() {
    //ConsolePrinter::simplePrint(__PRETTY_FUNCTION__);

    if (m_pServerThread != nullptr) {
        //ConsolePrinter::simplePrint("DBus Server Thread has already created");
        return false;
    }

    m_pServerThread = new std::thread([&] {
          m_idBus = g_bus_own_name(G_BUS_TYPE_SESSION, AIDAEMON::SERVER_INTERFACE.c_str(),
                                  G_BUS_NAME_OWNER_FLAGS_NONE, IPCHandler::OnBusAcquired,
                                  nullptr, nullptr, nullptr, nullptr);
          if (! m_idBus) {
              //ConsolePrinter::simplePrint("Fail to get bus");
          } else {
              //ConsolePrinter::simplePrint("Success to get bus");
              m_pLoop = g_main_loop_new(nullptr, false);
            if (m_pLoop) {
                //ConsolePrinter::simplePrint("Success to create a new GMainLoop");
    			g_main_loop_run(m_pLoop);
    		} else {
                //ConsolePrinter::simplePrint("Fail to create a new GMainLoop");
    		}
          }
        });
    return true;
}

void IPCHandler::sendMessage(std::string MethodID, rapidjson::Document *data) {

    rapidjson::Document ipcdata(rapidjson::kObjectType);

    ipcdata.AddMember(IPC_METHODID, 
        rapidjson::Value().SetString(MethodID.c_str(), MethodID.length(), ipcdata.GetAllocator()), 
        ipcdata.GetAllocator());
    ipcdata.AddMember(IPC_DATA, *data, ipcdata.GetAllocator());

    sendData(&ipcdata);
}
/* TODO  
void IPCHandler::sendMessage(std::string MethodID, int data) {

    rapidjson::Document ipcdata(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = ipcdata.GetAllocator();

    ipcdata.AddMember(rapidjson::StringRef(AIDAEMON::IPC_METHODID), MethodID, allocator);
    if (data == AIDAEMON::NULL_DATA) {
        rapidjson::Document nulldata;
        ipcdata.AddMember(rapidjson::StringRef(AIDAEMON::IPC_DATA), nulldata, allocator);
    } else {
        ipcdata.AddMember(rapidjson::StringRef(AIDAEMON::IPC_DATA), data, allocator);
    }
    
    sendData(&ipcdata);
}
*/
void IPCHandler::sendMessage(std::string MethodID, std::string data) {

    rapidjson::Document ipcdata(rapidjson::kObjectType);

    ipcdata.AddMember(IPC_METHODID, 
        rapidjson::Value().SetString(MethodID.c_str(), MethodID.length(), ipcdata.GetAllocator()), 
        ipcdata.GetAllocator());
    ipcdata.AddMember(IPC_DATA, 
        rapidjson::Value().SetString(data.c_str(), data.length(), ipcdata.GetAllocator()),
        ipcdata.GetAllocator());

    sendData(&ipcdata);
}

void IPCHandler::sendData(rapidjson::Document *ipcdata) {

    rapidjson::StringBuffer ipcdataBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(ipcdataBuf);
    if (!ipcdata->Accept(writer)) {
        //ConsolePrinter::simplePrint("ERROR: Build json");
        return;
    }

    std::string tempipcdata = ipcdataBuf.GetString();
    std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(tempipcdata.c_str()), tempipcdata.length());
    //ConsolePrinter::simplePrint(encoded.c_str());

    aidaemon__emit_send_messages(m_pDBusInterface, encoded.c_str());   
}

gboolean IPCHandler::on_handle_send_messages(
    AIDaemon *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_Data,
    gpointer user_data ) {
    IPCHandler* handler = IPCHandler::GetInstance();

    std::string decoded = base64_decode((char*)(arg_Data));
    handler->log(Level::INFO, __PRETTY_FUNCTION__, "VPA data : " + decoded);

    json vpaData;
    try {
        vpaData = json::parse(decoded);
    } catch (std::exception &e) {
        handler->log(Level::ERROR, __PRETTY_FUNCTION__, "IPC data not parseable");
        return false;
    }

    std::string Method = handler->getValueFromJson(vpaData, std::string(AIDAEMON::IPC_METHODID));
    std::string IPCData = handler->getValueFromJson(vpaData, std::string(AIDAEMON::IPC_DATA));

    if (Method.empty()) {
        handler->log(Level::ERROR, __PRETTY_FUNCTION__, "Method is Empty");
        return false;
    }

    aidaemon__complete_send_messages(object, invocation);

    handler->log(Level::INFO, __PRETTY_FUNCTION__, "Method : " + Method);

    if (Method == AIDAEMON::METHODID_VPA_AI_STATUS) {
        handler->sendAIStatus();
    } else if (Method == AIDAEMON::METHODID_VPA_AUTH_START) {
        handler->getCBLHandler()->startCBL();
    } else if (Method == AIDAEMON::METHODID_VPA_SET_CONF) {
        handler->setConfigured(IPCData);        
    } else if (Method == AIDAEMON::METHODID_VPA_VR_START) {
        handler->sendEvent(sampleApp::Event::onStartTTS, IPCData);
    } else {
        handler->log(Level::ERROR, __PRETTY_FUNCTION__, "Cannot handle this Method : " + Method);
    }
    /* TODO
    } else if (Method == AIDAEMON::METHODID_VPA_SET_RECOGNIZE) {
        handler->m_interactionManager->getDefaultClient()->setSpeechRecognize(IPCData);
    } else if (Method == AIDAEMON::METHODID_VPA_VR_STOP) {
        handler->setAudioError(false);
        handler->m_interactionManager->microphoneToggle(AIDAEMON::MIC_OFF);
        handler->m_interactionManager->getDefaultClient()->notifyOfTapToTalkEnd();            
    } else if (Method == AIDAEMON::METHODID_VPA_EVENT) {
        //ConsolePrinter::simplePrint(Method);
        handler->m_interactionManager->sendEvent(IPCData);
    } else if (Method == AIDAEMON::METHODID_VPA_EVENT_CANCEL) {
        //ConsolePrinter::simplePrint(Method);
    }else if (Method == AIDAEMON::METHODID_VPA_AUDIO_REQUEST) {
        handler->handleAudioControl(IPCData);
    } else if (Method == AIDAEMON::METHODID_REQ_SET_CONTEXT ) {
        handler->updateConext(IPCData);
    } else if (Method == AIDAEMON::METHODID_REQ_MIC ) {
        handler->m_interactionManager->microphoneToggle(IPCData);                  
    } else if (Method == AIDAEMON::METHODID_VPA_TTS_START) {
        handler->handleStartTTS(IPCData);
    } else if (Method == AIDAEMON::METHODID_VPA_TTS_STOP) {
        handler->handleStopTTS(IPCData);          
    } else {
        //ConsolePrinter::simplePrint("ERROR: Cannot Handle this Method");
    }
    */
    return true;
}
/* TODO
void IPCHandler::handleAudioControl(std::string data) {
    //ConsolePrinter::simplePrint(__PRETTY_FUNCTION__);

    std::string action = getValueFromJson(data, AIDAEMON::AUDIO_ACTION);
    action.erase(std::remove(action.begin(), action.end(), '"'), action.end());

    if (action == AIDAEMON::AUDIO_PLAY) {
        m_interactionManager->getDefaultClient()->playMVPAAduio();
    } else if (action == AIDAEMON::AUDIO_PAUSE) {
        m_interactionManager->playbackPause();
    } else if (action == AIDAEMON::AUDIO_NEXT) {
        m_interactionManager->playbackNext();
    } else if (action == AIDAEMON::AUDIO_PREVIOUS) {
        m_interactionManager->playbackPrevious();         
    } else if (action == AIDAEMON::AUDIO_SKIP_FORWARD) {s
        m_interactionManager->playbackSkipForward();        
    } else if (action == AIDAEMON::AUDIO_SKIP_BACKWARD) {
        m_interactionManager->playbackSkipBackward();        
    } else if (action == AIDAEMON::AUDIO_SHUFFLE) {
        m_interactionManager->playbackShuffle();        
    } else if (action == AIDAEMON::AUDIO_LOOP) {
        m_interactionManager->playbackLoop();        
    } else if (action == AIDAEMON::AUDIO_REPEAT) {
        m_interactionManager->playbackRepeat();        
    } else if (action == AIDAEMON::AUDIO_THUMBS_UP) {
        m_interactionManager->playbackThumbsUp();        
    } else if (action == AIDAEMON::AUDIO_THUMBS_DOWN) {
        m_interactionManager->playbackThumbsDown();        
    } else {
        //ConsolePrinter::simplePrint("ERROR: Cannot Handle Audio Request " + action);
    } 
}

void IPCHandler::handleStartTTS(std::string data) {
    m_interactionManager->getDefaultClient()->startTTS(
        getValueFromJson(data, AIDAEMON::TTS_START_EVENT),
        getValueFromJson(data, AIDAEMON::TTS_FINISH_EVENT));
}

void IPCHandler::handleStopTTS(std::string data) {
    m_interactionManager->getDefaultClient()->stopTTS(std::string(""));
}

void IPCHandler::updateConext(std::string data) {
    using namespace alexaClientSDK::contextManager;

    //ConsolePrinter::simplePrint(__PRETTY_FUNCTION__);

    rapidjson::Document payload;
    rapidjson::ParseResult result = payload.Parse(data);
    if (!result) {
        //ConsolePrinter::simplePrint("ERROR: Context JSON not parseable");
        return;
    } 

    if (payload.HasMember(AIDAEMON::CONTEXT) && 
        payload[AIDAEMON::CONTEXT].IsArray()) {            
        rapidjson::Value& context = payload[AIDAEMON::CONTEXT];    
        for (rapidjson::SizeType i = 0; i < context.Size(); i++) {
            rapidjson::StringBuffer contextdataBuf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(contextdataBuf);
            if (!context[i][CONTEXT_PAYLOAD].Accept(writer)) {
                //ConsolePrinter::simplePrint("ERROR: Build json string");
            } else {
                alexaClientSDK::avsCommon::avs::NamespaceAndName CONTEXT_MANAGER_PHONE_CONTROL_STATE{"PhoneCallController", "PhoneCallControllerState"};
                auto setStateSuccess = ContextManager::create()->setState(
                    alexaClientSDK::avsCommon::avs::NamespaceAndName(
                        context[i][AIDAEMON::CONTEXT_HEADER][AIDAEMON::CONTEXT_HNAMESPACE].GetString(),
                        context[i][AIDAEMON::CONTEXT_HEADER][AIDAEMON::CONTEXT_HNAME].GetString()),               
                    contextdataBuf.GetString(), 
                    alexaClientSDK::avsCommon::avs::StateRefreshPolicy::NEVER );
                if ( setStateSuccess != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS ) {
                    //ConsolePrinter::simplePrint("ERROR: Update Context=> " + std::string(contextdataBuf.GetString()));
                } else {
                    //ConsolePrinter::simplePrint("Success: Update Context=> " + std::string(contextdataBuf.GetString()));   
                }
            }
        }
    } else {
        //ConsolePrinter::simplePrint("ERROR: There is not " + AIDAEMON::CONTEXT);
        return;        
    }
}
*/
void IPCHandler::sendAIStatus(std::string status, std::string reason) {
    if (status.empty()) {
        status =  m_aiStatus;
        reason = m_aiStatusReason;
    } else {
        m_aiStatus = status;
        m_aiStatusReason = reason;
    }

    log(Level::INFO, __PRETTY_FUNCTION__, "status : " + status + " reason : " + reason);

    // TODO : 
    if (status == "DISCONNECTED" && reason == "DISCONNECTEDACL_CLIENT_REQUEST") {
        status = AIDAEMON::AI_STATUS_UNAUTH;
        reason = AIDAEMON::AI_CHANGED_REASON_UNAUTH_CLIENT;
        log(Level::INFO, __PRETTY_FUNCTION__, "TO DO: check UnAuth");
    }

    rapidjson::Document aistatus(rapidjson::kObjectType);

    aistatus.AddMember(AIDAEMON::AI_STATUS, 
        rapidjson::Value().SetString(status.c_str(), status.length(), aistatus.GetAllocator()),  
        aistatus.GetAllocator());

    if (status == AIDAEMON::AI_STATUS_READY) {
        aistatus.AddMember(AIDAEMON::AI_STATUS_VERSION, 
        rapidjson::Value().SetString(std::string("AAC Test Version").c_str(), std::string("AAC Test Version").length(), aistatus.GetAllocator()),
        aistatus.GetAllocator());        
    } else if (status == AIDAEMON::AI_STATUS_NOTREADY) {
        //ConsolePrinter::simplePrint("AI Status : " + AIDAEMON::AI_STATUS_NOTREADY);
    } else if (status == AIDAEMON::AI_STATUS_UNAUTH) {
        aistatus.AddMember(AIDAEMON::AI_CHANGED_REASON, 
            rapidjson::Value().SetString(reason.c_str(), reason.length(), aistatus.GetAllocator()),
            aistatus.GetAllocator());
        if (reason == AIDAEMON::AI_CHANGED_REASON_UNAUTH_PENDING) {
            aistatus.AddMember(AIDAEMON::AI_AUTH_CODE,
                rapidjson::Value().SetString(m_authcode.c_str(), m_authcode.length(), aistatus.GetAllocator()),
                aistatus.GetAllocator());
        }
    } else {
        aistatus.AddMember(AIDAEMON::AI_CHANGED_REASON, 
            rapidjson::Value().SetString(reason.c_str(), reason.length(), aistatus.GetAllocator()),
            aistatus.GetAllocator());
    }

    AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_AI_STATUS, &aistatus);
}

void IPCHandler::setAuthCode(std::string code) {
    m_authcode = code;
}

void IPCHandler::setConfigured(std::string data) {
    log(Level::INFO, __PRETTY_FUNCTION__, " ");

    m_configured = true;

/* TODO
    std::string configure = getValueFromJson(data, AIDAEMON::SET_CONF_CONFIGURATION);
    std::string configPath = "AIDaemon.json";

    std::ofstream writeFile(configPath.data());
    if (writeFile.is_open()) {
        writeFile << configure;
        writeFile.close();
    } else {
        //ConsolePrinter::simplePrint("ERROR: Can't make AIDaemon.json");
    }

    std::string db = getValueFromJson(
        getValueFromJson(configure, AIDAEMON::SET_CONF_AUTH_DELEGATE), 
        AIDAEMON::SET_CONF_DB);

    db.erase(std::remove(db.begin(), db.end(), '"'), db.end());
    db = db.substr(0, db.find_last_of("\\/"));

    if (db.length() > 0) {
        //ConsolePrinter::simplePrint("DB Path : " + db);
        recursive_mkdir(db.c_str(), S_IRWXU);
        if (access(db.c_str(), F_OK) != 0) {
            //ConsolePrinter::simplePrint("ERROR: Can't create DB Path");
            return;
        } else {
            //ConsolePrinter::simplePrint("Success to create DB Path : " + db);
        }
    } else {
        //ConsolePrinter::simplePrint("ERROR: Can't fine DB Path");
        return;
    }
*/
    m_waitForConfigure.notify_one();
}


void IPCHandler::waitForConfiguration() {
    log(Level::INFO, __PRETTY_FUNCTION__, " ");
    
    std::unique_lock<std::mutex> lck(m_configureMtx);

    while ( (m_waitForConfigure.wait_for(lck,std::chrono::seconds(1)) == std::cv_status::timeout) &&
             !m_configured ) {
        log(Level::INFO, __PRETTY_FUNCTION__, "timeout");
        sendAIStatus(AIDAEMON::AI_STATUS_READY);
    }
}

std::string IPCHandler::getValueFromJson(json &data, std::string key) {
    std::string result("");
    IPCHandler* handler = IPCHandler::GetInstance();
    try {
        auto obj = data.at(key);
        if (obj.is_string()) {
            result = obj.get<std::string>();
        } else {
            handler->log(Level::ERROR, __PRETTY_FUNCTION__, key + " is not in data");
        }
    } catch (json::exception &e) {
        handler->log(Level::ERROR, __PRETTY_FUNCTION__, key + " cannot be parsed");
        return result;
    }
    return result;
}

void IPCHandler::recursive_mkdir(const char *path, mode_t mode) {
    //ConsolePrinter::simplePrint(__PRETTY_FUNCTION__);

    char *spath = NULL;
    const char *next_dir = NULL;

    /* dirname() modifies input! */
    spath = strdup(path);
    if (spath == NULL) {
        /* Report error, no memory left for string duplicate. */
        goto done;
    }

    /* Get next path component: */
    next_dir = dirname(spath);

    if (access(path, F_OK) == 0) {
        /* The directory in question already exists! */
        goto done;
    }

    if (strcmp(next_dir, ".") == 0 || strcmp(next_dir, "/") == 0) {
        /* We reached the end of recursion! */
        goto done;
    }

    recursive_mkdir(next_dir, mode);
    if (mkdir(path, mode) != 0) {
       /* Report error on creating directory */
       //ConsolePrinter::simplePrint("ERROR: Failed to create directory : " + std::string(path));
    }

done:
    free(spath);
    return;
}

void IPCHandler::log(sampleApp::logger::LoggerHandler::Level level, const std::string tag, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        std::cerr << tag << " " << message << "\n";
        return;
    }
    loggerHandler->log(level, tag, message);
}

bool IPCHandler::sendEvent(const sampleApp::Event &event, const std::string &value) {
    auto activityHandler = m_activity.lock();
    activityHandler->notify(event, value);
    return true;
}

}  // namespace AIDAEMON
