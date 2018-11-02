/*
       Thread Safe Timer Pool Library
       Copyright (C) Dean Camera, 2018.

     dean [at] fourwalledcubicle [dot] com
          www.fourwalledcubicle.cpm
*/

/*
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org/>
*/

#pragma once

#include <condition_variable>
#include <forward_list>
#include <functional>
#include <memory>
#include <string>
#include <thread>


class TimerPool
    : public std::enable_shared_from_this<TimerPool>
{
public:
    class Timer;

    using Clock           = std::chrono::steady_clock;
    using PoolHandle      = std::shared_ptr<TimerPool>;
    using WeakTimerHandle = std::weak_ptr<Timer>;

    static PoolHandle               CreatePool(const std::string& name = "");

    virtual                         ~TimerPool();

    std::string                     name() const    { return m_name; }
    bool                            running() const { return m_running; }

    void                            stop();
    void                            wake();

    WeakTimerHandle                 createTimer();
    void                            deleteTimer(WeakTimerHandle handle);

protected:
    using TimerHandle = std::shared_ptr<Timer>;

    explicit                        TimerPool(const std::string& name);

    void                            run();

private:
    mutable std::mutex              m_mutex;
    std::condition_variable         m_cond;

    const std::string               m_name;
    std::forward_list<TimerHandle>  m_timers;

    bool                            m_running;
    std::thread                     m_thread;
};

class TimerPool::Timer
{
public:
    using Clock      = TimerPool::Clock;
    using Callback   = std::function<void(void)>;
    using PoolHandle = TimerPool::PoolHandle;

    explicit                        Timer(PoolHandle parentPool);
    virtual                         ~Timer() = default;

    void                            setCallback(Callback&& callback);
    void                            setInterval(std::chrono::milliseconds ms);
    void                            setRepeated(bool repeated);

    void                            start();
    void                            stop();
    bool                            running() const { return m_running; }

    Clock::time_point               nextExpiry() const;
    Clock::time_point               fire();

private:
    mutable std::mutex              m_mutex;
    PoolHandle                      m_parent;
    Clock::time_point               m_nextExpiry;

    bool                            m_running;
    Callback                        m_callback;
    std::chrono::milliseconds       m_interval;
    bool                            m_repeated;
};