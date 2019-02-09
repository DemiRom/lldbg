#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include "lldb/API/LLDB.h"

namespace {

// A simple thread-safe queue for LLDB events
class EventQueue {
    std::mutex m_mutex;
    std::deque<lldb::SBEvent> m_events;

public:
    size_t size(void) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_events.size();
    }

    void push(const lldb::SBEvent& new_event) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_events.push_back(new_event);
    }

    std::unique_ptr<lldb::SBEvent> pop(void) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_events.empty()) return nullptr;
        auto front_copy = std::unique_ptr<lldb::SBEvent>(new lldb::SBEvent(m_events.front()));
        m_events.pop_front();
        return front_copy;
    }

    void clear(void) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_events.clear();
    }
};

}

namespace lldbg {

// A pollable thread for collecting LLDB events from a queue
class LLDBEventListenerThread {
    lldb::SBListener m_listener;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_continue;
    EventQueue m_events;

    void poll_events();

public:
    void start(lldb::SBDebugger&);
    void stop(lldb::SBDebugger&);
    std::unique_ptr<lldb::SBEvent> pop_event() { return m_events.pop(); }

    LLDBEventListenerThread();

    LLDBEventListenerThread(const LLDBEventListenerThread&) = delete;
    LLDBEventListenerThread& operator=(const LLDBEventListenerThread&) = delete;
    LLDBEventListenerThread& operator=(LLDBEventListenerThread&&) = delete;
};

}
