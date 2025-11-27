#pragma once

#include <chrono>
#include <print>

namespace RTVE {
#ifdef _PROFILING
  class Timer {
  public:
    Timer() {}
  
    std::chrono::duration<double> getTime() {
      return std::chrono::system_clock::now() - mStartTime;
    }

    void start(const std::string& pName) {
      mName = pName;
      mStartTime = std::chrono::system_clock::now();
      std::println("'{0}' timer started", mName);
    }

    void stop() {
      std::println("'{0}' timer stopped: {1}", mName, getTime().count());
    }

  protected:
    std::chrono::time_point<std::chrono::system_clock> mStartTime;
    std::string mName;
  };

  class ScopedTimer: public Timer {
  public:
    ScopedTimer(const std::string& pName) {
      mName = pName;
      start(mName);
    }
  
    ~ScopedTimer() {
      stop();
    }
  };
#endif

#ifndef _PROFILING
  class Timer {
  public:
    Timer() {}

    std::chrono::duration<double> getTime() { return 0.0f; }

    void start(const std::string& pName) {}

    void stop() {}
  };

  class ScopedTimer: public Timer {
  public:
    ScopedTimer(const std::string& pName) {}
  };
#endif
}
