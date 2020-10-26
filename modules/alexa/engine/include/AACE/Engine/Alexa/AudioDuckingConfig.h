/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AACE_ENGINE_ALEXA_AUDIODUCKINGCONFIG_H
#define AACE_ENGINE_ALEXA_AUDIODUCKINGCONFIG_H

namespace aace {
namespace engine {
namespace alexa {

static const std::string& configurationJsonDuckingNotSupported = R"( {
    "virtualChannels":{
        "audioChannels" : [
            {
                "name" : "Earcon",
                "priority" : 250
            }
        ],
        "visualChannels" : [

        ]
    },
    "interruptModel" : {
        "Dialog" : {
        },
        "Communications" : {
            "contentType":
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                },
                "NONMIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                }
            }
        },
        "Alert" : {
            "contentType" :
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                }
            }
        },
        "Content" : {
            "contentType" :
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Alert" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Earcon" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                },
                "NONMIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Alert" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Earcon" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                }
            }
        }
    }
})";

static const std::string& configurationJsonSupportsDucking = R"( {
    "virtualChannels":{
        "audioChannels" : [
            {
                "name" : "Earcon",
                "priority" : 250
            }
        ],
        "visualChannels" : [

        ]
    },
    "interruptModel" : {
        "Dialog" : {
        },
        "Communications" : {
            "contentType":
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK"
                            }
                        }
                    }
                },
                "NONMIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                }
            }
        },
        "Alert" : {
            "contentType" :
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK",
                                "NONMIXABLE" : "MAY_DUCK"
                            }
                        }
                    }
                }
            }
        },
        "Content" : {
            "contentType" :
            {
                "MIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Alert" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK"
                            }
                        },
                        "Earcon" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MAY_DUCK",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                },
                "NONMIXABLE" : {
                    "incomingChannel" : {
                        "Dialog" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Communications" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Alert" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE"
                            }
                        },
                        "Earcon" : {
                            "incomingContentType" : {
                                "MIXABLE" : "MUST_PAUSE",
                                "NONMIXABLE" : "MUST_PAUSE"
                            }
                        }
                    }
                }
            }
        }
    }
})";

class AudioDuckingConfig {
public:
    static std::unique_ptr<std::istream> getConfig(bool supportsDucking = true, const std::string& override = "") {
        if (supportsDucking)
            if (override.empty())
                return std::unique_ptr<std::istringstream>(new std::istringstream(configurationJsonSupportsDucking));
            else
                return std::unique_ptr<std::istringstream>(new std::istringstream(override));
        else
            return std::unique_ptr<std::istringstream>(new std::istringstream(configurationJsonDuckingNotSupported));
    }

    AudioDuckingConfig() = delete;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUDIODUCKINGCONFIG_H
