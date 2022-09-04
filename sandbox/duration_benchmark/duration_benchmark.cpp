
#include <iostream>
#include <string>
#include <chrono>

class Duration
{
private:
    using namespace std::chrono;

    time_point<high_resolution_clock> tpStart;
    std::string tag;

public:
    Duration() noexcept {
        tpStart = high_resolution_clock::now();
    }

    explicit Duration(std::string tag) noexcept {
        this->tag = tag;
        tag.append(": ");
        tpStart = high_resolution_clock::now();
    }

    ~Duration() {
        auto tpEnd = high_resolution_clock::now();
        auto duration = tpEnd - tpStart;
        std::cout << tag << duration.count();
    }
};