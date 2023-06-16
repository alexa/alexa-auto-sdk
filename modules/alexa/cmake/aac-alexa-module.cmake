option(EXTERNALMEDIAPLAYER_1_1 "Build ExternalMediaPlayer 1.1" ON)
if(EXTERNALMEDIAPLAYER_1_1)
    add_definitions(-DEXTERNALMEDIAPLAYER_1_1)
endif()

if(AAC_CAPTIONS)
    add_definitions(-DAAC_CAPTIONS)
endif()
