#ifndef __AIDAEMON_IPC_H__
#define __AIDAEMON_IPC_H__

#include <string>

namespace AIDAEMON {
    static const int            NULL_DATA=-1;    

    const static std::string    SERVER_OBJECT = "/com/obigo/Nissan/AIDaemon";
    const static std::string    SERVER_INTERFACE = "com.obigo.Nissan.AIDaemon";

    static const char IPC_METHODID[] = "methodId";
    static const char IPC_DATA[] = "data";

    static const std::string    DIALOGID("dialogRequestId");

// AI_STATUS
    static const std::string            METHODID_VPA_AI_STATUS("VPA_AI_STATUS");
    static const char                   METHODID_AI_STATUS[] = "AI_STATUS";
        static const char                   AI_STATUS[] = "Status";
            static const std::string            AI_STATUS_NOTREADY("NOTREADY");
            static const std::string            AI_STATUS_READY("READY");
            static const std::string            AI_STATUS_DISCONNECTED("DISCONNECTED");
            static const std::string            AI_STATUS_PENDING("PENDING");
            static const std::string            AI_STATUS_CONNECTED("CONNECTED");
            static const std::string            AI_STATUS_UNAUTH("UNAUTH");                        
        static const char                   AI_CHANGED_REASON[] = "Reason";
        static const char                   AI_STATUS_VERSION[] = "Version";
        static const char                   AI_AUTH_CODE[] = "Code";
            static const std::string        AI_CHANGED_REASON_UNAUTH_CLIENT("UNAUTH_CLIENT");
            static const std::string        AI_CHANGED_REASON_UNAUTH_PENDING("UNAUTH_PENDING");       

    static const std::string    METHODID_NOTI_DIRECTIVE("AI_DIRECTIVE");
    static const std::string    METHODID_NOTI_VR_STATE("AI_VR_STATE");
        static const std::string        AI_VR_STATE_IDLE("IDLE");
        static const std::string        AI_VR_STATE_LISTENING("LISTENING");
        static const std::string        AI_VR_STATE_THINKING("THINKING");  

    static const std::string    METHODID_VPA_SET_RECOGNIZE("VPA_SET_RECOGNIZE");                                                          
    static const std::string    METHODID_VPA_VR_START("VPA_VR_START");
    static const std::string    METHODID_VPA_VR_STOP("VPA_VR_STOP");
    static const std::string    METHODID_VPA_EVENT("VPA_EVENT");
    static const std::string    METHODID_VPA_EVENT_CANCEL("VPA_EVENT_CANCEL");
    static const std::string    METHODID_VPA_TTS_START("VPA_TTS_START");
        static const std::string        TTS_START_EVENT("Start");
        static const std::string        TTS_FINISH_EVENT("Finished");
    static const std::string    METHODID_VPA_TTS_STOP("VPA_TTS_STOP");

    static const std::string    METHODID_VPA_AUDIO_REQUEST("VPA_AUDIO_REQUEST");
        static const std::string       AUDIO_ACTION("Action");
        static const std::string       AUDIO_PLAY("PLAY");
        static const std::string       AUDIO_PAUSE("PAUSE");
        static const std::string       AUDIO_STOP("STOP");
        static const std::string       AUDIO_RESUME("RESUME");        
        static const std::string       AUDIO_NEXT("AUDIO_NEXT");
        static const std::string       AUDIO_PREVIOUS("AUDIO_PREVIOUS");
        static const std::string       AUDIO_SKIP_FORWARD("AUDIO_SKIP_FORWARD");
        static const std::string       AUDIO_SKIP_BACKWARD("AUDIO_SKIP_BACKWARD");
        static const std::string       AUDIO_SHUFFLE("AUDIO_SHUFFLE");
        static const std::string       AUDIO_LOOP("AUDIO_LOOP");
        static const std::string       AUDIO_REPEAT("AUDIO_REPEAT");
        static const std::string       AUDIO_THUMBS_UP("AUDIO_THUMBS_UP");
        static const std::string       AUDIO_THUMBS_DOWN("AUDIO_THUMBS_DOWN");

    static const std::string    METHODID_REQ_SET_CONTEXT("REQ_SET_CONTEXT");
        static const std::string       CONTEXT("context");
        static const std::string       CONTEXT_HEADER("header");
        static const std::string       CONTEXT_HNAMESPACE("namespace");
        static const std::string       CONTEXT_HNAME("name");
        static const std::string       CONTEXT_PAYLOAD("payload");

    static const std::string    METHODID_REQ_MIC("REQ_MIC");
        static const std::string        MIC_OFF("MIC_OFF");
        static const std::string        MIC_ON("MIC_ON");

    static const std::string    METHODID_AI_TTS_READY("AI_TTS_READY");
    static const std::string    METHODID_NOTI_TTS_START("AI_TTS_START");
    static const std::string    METHODID_NOTI_TTS_FINISH("AI_TTS_FINISHED");

   static const std::string    METHODID_AI_AUDIO_STATE("AI_AUDIO_STATE");       
        static const std::string        AUDIO_ITEMID("AudioItemId");
        static const std::string        AUDIO_STATE("AudioState");
        static const std::string        AUDIO_STATE_IDLE("IDLE");
        static const std::string        AUDIO_OFFSET("Offset");
        static const std::string        AUDIO_LENGTH("MediaLengthInMilliseconds");
        static const std::string        AUDIO_ERROR("Error");        

   static const std::string    METHODID_VPA_AUTH_START("VPA_AUTH_START");

   static const std::string    METHODID_VPA_SET_CONF("VPA_SET_CONF");
        static const std::string    SET_CONF_CONFIGURATION("configuration");
        static const std::string    SET_CONF_USERID("userID");
        static const std::string    SET_CONF_DB("databaseFilePath");
        static const std::string    SET_CONF_AUTH_DELEGATE("cblAuthDelegate");

#ifdef OBIGO_SPEECH_SENDER    
    static const std::string    METHODID_AI_SPEECH("AI_SPEECH");
        static const std::string        AI_SPEECH_DIALOGID("DialogID");
        static const std::string        AI_SPEECH_NUM("Num");
        static const std::string        AI_SPEECH_READBYTES("ReadBytes"); 
        static const std::string        AI_SPEECH_SPEECH("Speech");   
#endif // OBIGO_SPEECH_SENDER

}



#endif /* ___AIDAEMON_IPC_H__ */
