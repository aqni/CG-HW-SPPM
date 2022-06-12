#pragma once
#ifndef __PERF_TEST_HPP__
#define __PERF_TEST_HPP__

#include <chrono>
#include <iostream>
#include <string_view>

class GuardClock {
 public:
  GuardClock(std::string_view msg)
      : msg_(msg), start_(std::chrono::system_clock::now()) {}
  ~GuardClock() {
    auto end = std::chrono::system_clock::now();
    auto d =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    std::cerr << msg_ << d.count() << "ms" << std::endl;
  }

 private:
  std::chrono::system_clock::time_point start_;
  std::string_view msg_;
};

#endif  // __PERF_TEST_HPP__
