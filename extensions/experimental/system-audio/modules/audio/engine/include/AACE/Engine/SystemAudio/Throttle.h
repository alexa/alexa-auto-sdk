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

#ifndef AACE_ENGINE_SYSTEMAUDIO_AUDIO_THROTTLE_H
#define AACE_ENGINE_SYSTEMAUDIO_AUDIO_THROTTLE_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace aace {
namespace engine {
namespace systemAudio {

/**
 * Throttle breaks down a large chunk of data into fragments of specified size and delivers
 * them periodically with specified interval. The delivery of fragments are done by calling
 * specified output function in a separate thread.
 *
 * If the incoming data is already provided in small fragments, the throttling will be bypassed
 * and the data will be delivered in the same timing with minimum overhead.
 */
template <typename T>
class Throttle {
public:
    using OutputFunc = std::function<void(const T* data, size_t length)>;

    explicit Throttle(size_t frag_size, std::chrono::milliseconds frag_interval, OutputFunc output) :
            m_frag_size{frag_size}, m_frag_interval{frag_interval}, m_output{std::move(output)} {
    }

    ~Throttle() {
        if (m_deliveryThread.joinable()) {
            quit();
            m_deliveryThread.join();
        }
    }

    void write(const T* data, size_t length) {
        flush();

        // If the duration of audio data is less than twice the minimum size, sent it immediately.
        if (length < m_frag_size * 2) {
            m_output(data, length);
            return;
        }

        // create a delivery thread to deliver audio fragments periodically
        if (!m_deliveryThread.joinable()) {
            m_deliveryThread = std::thread([this] {
                std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
                for (;;) {
                    lock.lock();
                    while (m_fragments.empty()) {
                        m_cv.wait(lock, [this] { return !m_fragments.empty() || m_quit; });
                        if (m_quit) {
                            return;
                        }
                    }

                    auto& fragment = m_fragments.front();
                    std::this_thread::sleep_until(fragment.time);
                    m_output(fragment.audio.data(), fragment.audio.size());
                    m_fragments.pop_front();

                    lock.unlock();
                }
            });
        }

        const T* frag_begin = data;
        const T* data_end = frag_begin + length;
        auto frag_time = std::chrono::high_resolution_clock::now();

        // send the first fragment immediately
        m_output(frag_begin, m_frag_size);
        frag_begin += m_frag_size;
        frag_time += m_frag_interval;

        // queue the rest
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            while ((size_t)(data_end - frag_begin) >= m_frag_size * 2) {
                // queue a fragment with calculated time
                m_fragments.emplace_back(frag_time, std::vector<T>(frag_begin, frag_begin + m_frag_size));
                frag_begin += m_frag_size;
                frag_time += m_frag_interval;
            }

            // queue the last fragment
            m_fragments.emplace_back(frag_time, std::vector<T>(frag_begin, data_end));
            m_cv.notify_all();
        }
    }

private:
    void flush() {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (!m_fragments.empty()) {
            // send any remaining in the queue immediately
            for (const auto& fragment : m_fragments) {
                m_output(fragment.audio.data(), fragment.audio.size());
            }
            m_fragments.clear();
        }
    }

    void quit() {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_quit = true;
        m_cv.notify_all();
    }

private:
    using Clock = std::chrono::high_resolution_clock;

    const size_t m_frag_size;
    const std::chrono::milliseconds m_frag_interval;
    OutputFunc m_output;

    struct Fragment {
        Fragment(Clock::time_point time, std::vector<T>&& audio) : time{time}, audio{audio} {
        }
        Clock::time_point time;
        std::vector<T> audio;
    };
    std::deque<Fragment> m_fragments;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_deliveryThread;
    bool m_quit = false;
};

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_SYSTEMAUDIO_AUDIO_THROTTLE_H
