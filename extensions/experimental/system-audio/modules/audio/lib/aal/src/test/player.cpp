/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#define G_LOG_DOMAIN "AAL"
#include <aal.h>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#include <gtest/gtest.h>

#define LOG(msg, ...) printf("[Player] " msg "\n", ##__VA_ARGS__)

static std::string param_audio_file;
static std::string param_aal_module;
static int param_module_id = -1;
static std::string param_device;
static int param_iterations = 100;

std::pair<bool, std::string> parse_args(int argc, char** argv) {
    std::vector<std::string> args{argv + 1, argv + argc};
    auto c1 = [](const std::string& arg, unsigned char chr = '-') { return arg[0] == chr; };
    auto c2 = [](const std::string& arg, unsigned char chr = '-', const std::string& str = "") {
        return ((arg[1] == chr) && (arg[2] == '\0')) ||
               ((arg[1] == '-') && (arg.compare(2, std::string::npos, str) == 0));
    };
    auto size = args.size();
    for (size_t i = 0; i < size; ++i) {
        const auto& arg = args[i];
        if (c1(arg)) {
            if (c2(arg, ' ', "audio-file")) {
                if (++i == size) {
                    return {false, "missing audio-file"};
                }
                param_audio_file = args[i];
            } else if (c2(arg, ' ', "device")) {
                if (++i == size) {
                    return {false, "missing device"};
                }
                param_device = args[i];
            } else if (c2(arg, ' ', "aal-module")) {
                if (++i == size) {
                    return {false, "missing aal-module"};
                }
                param_aal_module = args[i];
            } else if (c2(arg, ' ', "iterations")) {
                if (++i == size) {
                    return {false, "missing iterations"};
                }
                param_iterations = std::stoi(args[i]);
            } else {
                std::stringstream err;
                err << "unrecognized parameter： " << arg;
                return {false, err.str()};
            }
        } else {
            std::stringstream err;
            err << "unrecognized parameter： " << arg;
            return {false, err.str()};
        }
    }
    return {true, ""};
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    auto result = parse_args(argc, argv);
    if (!result.first) {
        std::cerr << "Error in command line: " << result.second << "\n";
        return EXIT_FAILURE;
    }

    int modules = aal_get_module_count();
    for (int i = 0; i < modules; i++) {
        std::string name = aal_get_module_name(i);
        // If the target is empty string, use the first module we found
        if (name == param_aal_module || param_aal_module.empty()) {
            param_module_id = i;
            break;
        }
    }

    if (param_module_id < 0 || !aal_initialize(param_module_id)) {
        LOG("Invalid module or aal_initialize failed");
        return EXIT_FAILURE;
    }

    auto test_result = RUN_ALL_TESTS();

    aal_deinitialize(param_module_id);

    return test_result;
}

TEST(Interactive, DISABLED_Player) {
    struct TestCase {
        aal_handle_t handle;
        const char* file_name;
        bool streaming;

        static void on_start(void* user_data) {
            LOG("Player started");
        }

        static void on_stop(aal_status_t reason, void* user_data) {
            switch (reason) {
                case AAL_SUCCESS:
                    LOG("Player finished playing");
                    break;
                case AAL_PAUSED:
                    LOG("Player paused playing");
                    break;
                case AAL_ERROR:
                    LOG("Player encountered error");
                    break;
                case AAL_UNKNOWN:
                    LOG("Player stopped playing for unknown reason");
                    break;
            }
        }

        void streaming_loop() {
            const int BUFFER_SIZE = 4096;
            char buffer[BUFFER_SIZE];
            auto fp = fopen(file_name, "r");
            if (!fp) {
                LOG("File (%s) cannot be opened, notify EOS", file_name);
                goto eos;
            }

            do {
                auto r = fread(buffer, sizeof(uint8_t), BUFFER_SIZE, fp);
                if (r > 0) {
                    LOG("Write to player %ld", r);
                    ssize_t w = aal_player_write(handle, buffer, r);
                    if (w != (ssize_t)r) {
                        LOG("aal_player_write failed written=%ld vs size=%ld", w, r);
                        // Will retry...
                    }
                }
                if (r != BUFFER_SIZE) {
                    LOG("File reaches EOF or error");
                    goto eos;
                }
            } while (true);

        eos:
            aal_player_notify_end_of_stream(handle);
            if (fp) fclose(fp);
        }

        std::thread start_streaming_thread() {
            return std::thread(&TestCase::streaming_loop, this);
        }

        void on_data_requested() {
            // LOG("Data requested, start streaming...");
            if (streaming) {
                // LOG("Ignore...");
                return;
            }
            auto thrd = start_streaming_thread();
            thrd.detach();
            streaming = true;
        }
        static void on_data_requested(void* user_data) {
            reinterpret_cast<TestCase*>(user_data)->on_data_requested();
        }
    } t{};

    const aal_listener_t listener = {.on_start = TestCase::on_start,
                                     .on_stop = TestCase::on_stop,
                                     .on_almost_done = nullptr,
                                     .on_data = nullptr,
                                     .on_data_requested = TestCase::on_data_requested};

    aal_attributes_t attr = {.name = "SampleApp",
                             .device = nullptr,
                             .uri = nullptr,
                             .listener = &listener,
                             .user_data = &t,
                             .module_id = param_module_id};

    if (strncmp(param_audio_file.c_str(), "stream://", 9) == 0) {
        /* Create Player in stream mode */
        const char* file = param_audio_file.c_str() + 9;

        aal_audio_parameters_t params;
        params.stream_type = AAL_STREAM_LPCM;
        params.lpcm = {.sample_format = AAL_SAMPLE_FORMAT_DEFAULT, .channels = 0, .sample_rate = 0};
        t.handle = aal_player_create(&attr, &params);
        ASSERT_NE(t.handle, nullptr);
        t.file_name = file;
    } else {
        attr.uri = param_audio_file.c_str();
        t.handle = aal_player_create(&attr, nullptr);
        ASSERT_NE(t.handle, nullptr);
    }

    LOG("Play...");
    aal_player_play(t.handle);

    int c;
    do {
        c = getc(stdin);
        switch (c) {
            case 'p':
                aal_player_pause(t.handle);
                break;
            case 's':
                aal_player_stop(t.handle);
                break;
            case 'q':
                LOG("Shutdown...");
            default:
                // do nothing
                break;
        }
    } while (c != EOF);

    aal_player_destroy(t.handle);
}

TEST(StressTest, RepeatedStops) {
    struct TestCase {
        bool started{};
        bool stopped{};
        std::mutex mutex;
        std::condition_variable cv;

        void reset() {
            std::lock_guard<std::mutex> lock(mutex);
            started = false;
            stopped = false;
        }

        void on_start() {
            {
                std::lock_guard<std::mutex> lock(mutex);
                started = true;
            }
            cv.notify_all();
        }

        bool wait_started(std::chrono::seconds timeout) {
            std::unique_lock<std::mutex> lock(mutex);
            return cv.wait_for(lock, timeout, [this] { return started; });
        }

        void on_stop(aal_status_t reason) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                stopped = true;
            }
            cv.notify_all();
        }

        bool wait_stopped(std::chrono::seconds timeout) {
            std::unique_lock<std::mutex> lock(mutex);
            return cv.wait_for(lock, timeout, [this] { return stopped; });
        }

        static void on_start(void* user_data) {
            reinterpret_cast<TestCase*>(user_data)->on_start();
        }

        static void on_stop(aal_status_t reason, void* user_data) {
            reinterpret_cast<TestCase*>(user_data)->on_stop(reason);
        }
    } t;

    aal_listener_t listener = {.on_start = TestCase::on_start,
                               .on_stop = TestCase::on_stop,
                               .on_almost_done = nullptr,
                               .on_data = nullptr,
                               .on_data_requested = nullptr};

    const aal_attributes_t attr = {.name = "RepeatedStops",
                                   .device = param_device.empty() ? nullptr : param_device.c_str(),
                                   .uri = param_audio_file.c_str(),
                                   .listener = &listener,
                                   .user_data = &t,
                                   .module_id = param_module_id};
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1000, 2000);

    for (int i = 0; i < param_iterations; ++i) {
        std::cout << "Iteration #" << i << "\n";

        std::cout << "Play " << param_audio_file << "\n";
        aal_handle_t player = aal_player_create(&attr, nullptr);
        ASSERT_NE(player, nullptr);

        aal_player_play(player);
        ASSERT_TRUE(t.wait_started(std::chrono::seconds(1)));

        int play_duration = dist(rng);
        std::cout << "Let it play for " << play_duration << " ms..."
                  << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(play_duration));

        aal_player_stop(player);
        ASSERT_TRUE(t.wait_stopped(std::chrono::seconds(5)));

        aal_player_destroy(player);

        t.reset();
    }
}

TEST(FunctionalTest, CreateWithFile) {
    aal_attributes_t attr = {.name = "AudioParameters",
                             .device = param_device.empty() ? nullptr : param_device.c_str(),
                             .uri = param_audio_file.c_str(),
                             .listener = nullptr,
                             .user_data = nullptr,
                             .module_id = param_module_id};

    ASSERT_FALSE(param_audio_file.empty());

    {  // Can play file without specifying any audio parameters
        aal_handle_t player = aal_player_create(&attr, nullptr);
        ASSERT_NE(player, nullptr);
        aal_player_play(player);
        std::this_thread::sleep_for(std::chrono::milliseconds(std::chrono::seconds(5)));
        aal_player_stop(player);
        aal_player_destroy(player);
    }
    {  // Cannot play file with AAL_STREAM_LPCM parameter
        aal_audio_parameters_t audio_params;
        audio_params.stream_type = AAL_STREAM_LPCM;

        aal_handle_t player = aal_player_create(&attr, &audio_params);
        ASSERT_EQ(player, nullptr);
    }
}

TEST(FunctionalTest, CreateWithStream) {
    aal_attributes_t attr = {.name = "AudioParameters",
                             .device = param_device.empty() ? nullptr : param_device.c_str(),
                             .uri = "",
                             .listener = nullptr,
                             .user_data = nullptr,
                             .module_id = param_module_id};
    {  // Can play without parameters
        aal_handle_t player = aal_player_create(&attr, nullptr);
        ASSERT_NE(player, nullptr);
        aal_player_destroy(player);
    }
    {  // Can play with LPCM parameters
        aal_audio_parameters_t audio_params;
        audio_params.stream_type = AAL_STREAM_LPCM;
        audio_params.lpcm = {.sample_format = AAL_SAMPLE_FORMAT_DEFAULT, .channels = 0, .sample_rate = 0};

        for (auto channel_count : {1, 2}) {
            audio_params.lpcm.channels = channel_count;
            for (auto sample_rate : {16000, 32000, 44100, 48000}) {
                audio_params.lpcm.sample_rate = sample_rate;

                aal_handle_t player = aal_player_create(&attr, &audio_params);
                ASSERT_NE(player, nullptr);
                aal_player_destroy(player);
            }
        }
    }
    {  // Cannot play with non-LPCM stream
        aal_audio_parameters_t audio_params;
        audio_params.stream_type = AAL_STREAM_UNKNOWN;

        aal_handle_t player = aal_player_create(&attr, &audio_params);
        ASSERT_EQ(player, nullptr);
    }
}