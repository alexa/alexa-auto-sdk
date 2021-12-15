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

#ifndef SAMPLEAPP_TTY_H
#define SAMPLEAPP_TTY_H

// C++ Standard Library
#include <termios.h>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TTY
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TTY {
private:
    int savefd{-1};
    struct termios savets {};
    enum { RESET, CBREAK } state{RESET};

public:
    TTY() = default;
    auto isatty(int fd) -> bool {
        struct termios ts {};              // TERMinal I/O Structure
        return (tcgetattr(fd, &ts) >= 0);  // true if no error (is a tty)
    }
    auto cbreak(int fd) -> int {  // put the terminal into cbreak mode
        if (state == CBREAK) {
            return 0;
        }
        struct termios ts {};
        if (tcgetattr(fd, &ts) < 0) {
            return -1;
        }
        savets = ts;
        // echo off, canonical mode off
        ts.c_lflag &= ~(ECHO | ICANON);
        // case b: 1 byte at a time, no timer
        ts.c_cc[VMIN] = 1;
        ts.c_cc[VTIME] = 0;
        if (tcsetattr(fd, TCSAFLUSH, &ts) < 0) {
            return -1;
        }
        if (tcgetattr(fd, &ts) < 0) {
            int err = errno;
            tcsetattr(fd, TCSAFLUSH, &savets);
            errno = err;
            return -1;
        }
        if (((ts.c_lflag & (ECHO | ICANON)) != 0) || (ts.c_cc[VMIN] != 1) || (ts.c_cc[VTIME] != 0)) {
            tcsetattr(fd, TCSAFLUSH, &savets);
            errno = EINVAL;
            return -1;
        }
        savefd = fd;
        state = CBREAK;
        return 0;
    }
    auto reset(int fd) -> int {  // reset the terminal
        if (state == RESET) {
            return 0;
        }
        if (tcsetattr(fd, TCSAFLUSH, &savets) < 0) {
            return -1;
        }
        savefd = -1;
        state = RESET;
        return 0;
    }
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_TTY_H
