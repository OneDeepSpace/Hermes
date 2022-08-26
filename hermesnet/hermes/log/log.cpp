
#include "log.h"
#include <iomanip>
#include <sstream>

using namespace utility::logger;

P7ClientWrapper::P7ClientWrapper(const char* pArgs) noexcept
    : ptr_(P7_Create_Client(TM(pArgs)))
{}

P7ClientWrapper::~P7ClientWrapper()
{
    if (nullptr != ptr_) {
        ptr_->Release();
        ptr_ = nullptr;
    }
}

IP7_Client* P7ClientWrapper::get() const
{
    return ptr_;
}

IP7TraceWrapper::IP7TraceWrapper(IP7_Client* client, const char* name) noexcept
    : pTrace_(P7_Create_Trace(client, TM(name)))
{}

IP7TraceWrapper::~IP7TraceWrapper()
{
    if (nullptr != pTrace_) {
        P7_Trace_Release(pTrace_);
        for(auto&[key, ptr]: modules_) {
            ptr = nullptr;
        }
    }
}

bool IP7TraceWrapper::registerModule(EModule name) noexcept
{
    if (!pTrace_ or modules_.find(name) != modules_.end()) return false;
    IP7_Trace::hModule module { nullptr };
    modules_[name] = module;
    pTrace_->Register_Module(TM(getModuleName(name)), &modules_[name]);
    return true;
}

void IP7TraceWrapper::log(EModule name, char const* text) noexcept {
    auto module { modules_[name] };
    pTrace_->P7_TRACE(module, TM("%s"), text);
}

IP7_Trace* IP7TraceWrapper::get() const
{
    return pTrace_;
}

Logger::Logger(std::string* const s, std::string const& name)
    : settings_( s ? std::move(*s) : std::string() )
    , client_(settings_.c_str())
    , trace_(client_.get(), name.c_str())
{
    if ( settings_.empty() ) throw std::runtime_error("P7 not initialized");
    P7_Set_Crash_Handler();
}

Logger& Logger::getInstanceImpl(std::string* const s, std::string const& n)
{
    static Logger instance { s, n };
    return instance;
}

void Logger::init(std::string settings, std::string const& name)
{
    getInstanceImpl(&settings, name);
}

Logger& Logger::getInstance()
{
    return getInstanceImpl();
}

Logger::~Logger()
{
    P7_Flush();
}

bool Logger::registerModule(EModule name) noexcept
{
    bool b { trace_.registerModule(name) };
    std::stringstream ss;
    const char* status { b ? "successful" : "error" };
    ss << "log module " << std::quoted(getModuleName(name)) << " registration - " << status;
    log(name, ss.str().c_str());
    return b;
}

void Logger::log(EModule module, char const* text) noexcept
{
    trace_.log(module, text);
}

