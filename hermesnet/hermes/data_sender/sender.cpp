
#include "sender.h"
#include <hermes/log/log.h>
#include <hermes/common/structures.h>

using namespace network;
using namespace utility::logger;

namespace
{   // right local LOG macro for this entire module
    #define LOG(text) \
            Logger::getInstance().log(EModule::SENDER, (text));
}

DataSender::DataSender()
{
    LOG_REGISTER_MODULE(EModule::SENDER)
}

void DataSender::processOutcomingMessages(Entry& entry, Clients& clients, std::vector<std::uint8_t>& buffer)
{

}
