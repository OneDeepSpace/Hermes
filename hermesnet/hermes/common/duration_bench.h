
#pragma once

#include <iostream>
#include <string>
#include <chrono>

namespace utility::bench
{

#define LOG_DURATION(text) utility::bench::Duration((text));

    using namespace std::chrono;

    class Duration
    {
    private:
        time_point<high_resolution_clock> tpStart;
        std::string tag;

    public:
        Duration() noexcept {
            tpStart = high_resolution_clock::now();
        }

        explicit Duration(std::string tag) noexcept {
            this->tag = tag + ": ";
            tpStart = high_resolution_clock::now();
        }

        ~Duration() {
            auto tpEnd = high_resolution_clock::now();
            auto duration = tpEnd - tpStart;
            std::cout << tag << static_cast<std::chrono::nanoseconds>(duration).count() << " ns" << std::endl;
        }
    };
}

