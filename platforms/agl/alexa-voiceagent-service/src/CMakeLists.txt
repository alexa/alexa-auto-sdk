###########################################################################
# Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

# Add target to project dependency list
PROJECT_TARGET_ADD(alexa-voiceagent)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Define project Targets
    add_library(${TARGET_NAME} MODULE
        ${TARGET_NAME}-binding.c
    )

    set(OPENAPI_DEF "${TARGET_NAME}-apidef" CACHE STRING "name and path to the JSON API definition without extension")

    # Binder exposes a unique public entry point
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        PREFIX "afb-"
        LABELS "BINDINGV3"
        LINK_FLAGS ${BINDINGS_LINK_FLAG}
        OUTPUT_NAME ${TARGET_NAME}
    )

    # Library dependencies (include updates automatically)
    TARGET_LINK_LIBRARIES(${TARGET_NAME}
        afb-helpers
        ctl-utilities
        ${link_libraries})

add_subdirectory("plugins")