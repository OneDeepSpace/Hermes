
#pragma once

#include <ctime>
#include <chrono>
#include <iomanip>
#include <cstdint>

namespace network::message::object
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    /* ---------------- Ping message object ----------------
     *
     * Client send this object to server, wrapped into net
     * message; on server sets start_ timepoint and send it
     * back, where sets end_ on read message and later giving
     * the time difference.
     *
     * Structure size - * bytes
     * ----------------------------------------------------- */

    class MPing
    {
    public:
        // millisec, microsec, nanosec
        enum class EDimension : std::uint32_t {
            ms = 0, mc, ns
        };

    private:
        using clock = std::chrono::high_resolution_clock;

        clock::time_point start_;   // sets when server received first
        clock::time_point end_;     // sets when client received back

    public:
        MPing() = default;
        ~MPing() = default;

        void setStart() {
            start_ = clock::now();
        }

        void setEnd() {
            end_ = clock::now();
        }

        [[nodiscard]] std::size_t getTime(EDimension d = EDimension::ms) const {
            const auto dur{end_ - start_};
            switch (d) {
                default:
                case EDimension::ms:
                    return duration_cast<milliseconds>(dur).count();
                case EDimension::mc:
                    return duration_cast<microseconds>(dur).count();
                case EDimension::ns:
                    return duration_cast<nanoseconds>(dur).count();
            }
        }

        friend std::ostream& operator<< (std::ostream& os, MPing const& p) {
            auto st_c { clock::to_time_t(p.start_) };
            auto et_c { clock::to_time_t(p.end_)   };
            os  << "MPing:\n"
                << "  start - " << std::put_time(std::localtime(&st_c), "%F %T") << "\n"
                << "  end   - " << std::put_time(std::localtime(&et_c), "%F %T") << "\n"
                << "  time  - " << p.getTime() << "ms\n";
            return os;
        }

    };  // MPing

}   // network::message::objects