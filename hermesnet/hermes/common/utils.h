
#pragma once

#include <iostream>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

namespace utility::misc
{

    // generate unique UUID
    boost::uuids::uuid generateUUID() noexcept {
        static boost::uuids::random_generator generator;
        return generator();
    }

}
