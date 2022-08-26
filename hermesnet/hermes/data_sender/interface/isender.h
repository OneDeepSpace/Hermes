
#pragma once

namespace network::service
{
    class ISender
    {
    public:
        virtual void process() = 0;

        // http://www.gotw.ca/publications/mill18.htm
        virtual ~ISender() {};
    };
}
