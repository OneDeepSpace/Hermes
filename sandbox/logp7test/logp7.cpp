
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>
#include <exception>
#include <unordered_map>

#include <P7_Version.h>
#include <P7_Client.h>
#include <P7_Trace.h>
#include <P7_Extensions.h>

// todo: verbose/severity mode

// register module names here
enum class EModule : std::uint8_t {
    MAIN = 0,
    // ...
};

// helper function
inline const char* getModuleName(EModule e) noexcept {
    switch (e) {
        case EModule::MAIN: return "main";
        default: return "undefined";
    }
}

class P7ClientWrapper
{
private:
    IP7_Client* ptr_ { nullptr };

public:
    P7ClientWrapper() = default;

    explicit P7ClientWrapper(const char* pArgs) noexcept
        : ptr_(P7_Create_Client(TM(pArgs)))
    {}

    virtual ~P7ClientWrapper() {
        if (nullptr != ptr_) {
            ptr_->Release();
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] IP7_Client* get() const {
        return ptr_;
    }

};

class IP7TraceWrapper
{
public:
    using hash_map = std::unordered_map<EModule, IP7_Trace::hModule>;

    IP7TraceWrapper() = default;

    explicit IP7TraceWrapper(IP7_Client* client, const char* name) noexcept
        : pTrace_(P7_Create_Trace(client, TM(name)))
    {}

    ~IP7TraceWrapper() {
        if (nullptr != pTrace_) {
            P7_Trace_Release(pTrace_);
            for(auto&[key, ptr]: modules_) {
                ptr = nullptr;
            }
        }
    }

public:

    bool registerModule(EModule name) noexcept {
        if (!pTrace_) return false;
        IP7_Trace::hModule module { nullptr };
        modules_[name] = module;
        pTrace_->Register_Module(TM(getModuleName(name)), &modules_[name]);
        return true;
    }

    void log(EModule name, char const* text) {
        auto module { modules_[name] };
        pTrace_->P7_TRACE(module, TM("%s"), text);
    }

    [[nodiscard]] IP7_Trace* get() const {
        return pTrace_;
    }

private:
    IP7_Trace*  pTrace_  { nullptr };
    hash_map    modules_;

};

class Logger
{
private:
    std::string settings_ {};
    P7ClientWrapper client_;
    IP7TraceWrapper trace_;

    explicit Logger(std::string* const s, std::string const& name)
        : settings_( s ? std::move(*s) : std::string() )
        , client_(settings_.c_str())
        , trace_(client_.get(), name.c_str())
    {
        if ( settings_.empty() ) throw std::runtime_error("P7 not initialized");
        P7_Set_Crash_Handler();
    }

    static Logger& getInstanceImpl(std::string* const s = nullptr, std::string const& n = "app") {
        static Logger instance { s, n };
        return instance;
    }

public:
    static void init(std::string settings, std::string const& name) {
        getInstanceImpl(&settings, name);
    }

    static Logger& getInstance() {
        return getInstanceImpl();
    }

    virtual ~Logger() {
        P7_Flush();
    }

    Logger(Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

public:

    bool registerModule(EModule name) {
        return trace_.registerModule(name);
    }

    void log(EModule module, char const* text) {
        trace_.log(module, text);
    }

};  // Logger

namespace
{
// define for each compilation unit in anonymous
#define LOG(text) \
    Logger::getInstance().log(EModule::MAIN, (text));

}

#define LOG_REGISTER_MODULE(module) \
    Logger::getInstance().registerModule((module));

int main()
{
    {
        const char* appName {"sandbox"};
        const char* p7Config { "/P7.Sink=Console /P7.On=1 /P7.Pool=1024 /P7.Format=\"{%cn}{%mn}[%tm][%lv][%fn] %ms\"" };
        Logger::init({p7Config}, { appName });
        LOG_REGISTER_MODULE(EModule::MAIN)
        LOG("log initialized")
    }

    const std::uint32_t count { 42 };
    for (std::uint32_t i {0}; i < count; ++i) {
        LOG("Hello P7 !!!")
    }

    return 0;
}