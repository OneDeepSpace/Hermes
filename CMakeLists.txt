cmake_minimum_required(VERSION 3.16)
project(~Hermes~ VERSION 1.0.0)

# options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Werror -Wextra -Winline -Wpedantic") # -Wall
#SET(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

# build type
if (DCMAKE_BUILD_TYPE EQUAL "Debug")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
elseif (CMAKE_BUILD_TYPE EQUAL "Release")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()


# directories
set(PROJDIR ${PROJECT_SOURCE_DIR})
set(BIN_DIR ${PROJDIR}/bin)
set(LIBS_DIR ${PROJDIR}/libs)
set(HERMESNET_DIR ${PROJDIR}/hermesnet)
set(INCLUDE_DIR ${HERMESNET_DIR})
set(CLIENT_DIR ${PROJDIR}/client)
set(SERVER_DIR ${PROJDIR}/server)


# boost env setup
set(Boost_NO_SYSTEM_PATHS True)
set(BOOST_DIR ${LIBS_DIR}/boost)
set(BOOST_ROOT ${BOOST_DIR})
set(BOOST_LIBRARYDIR ${BOOST_DIR}/lib)
set(BOOST_INCLUDEDIR ${BOOST_DIR}/include)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.77.00 REQUIRED COMPONENTS system filesystem iostreams regex chrono date_time atomic)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "boost lib not found! [current path: ${BOOST_DIR}]")
endif()


# P7 log env setup
set(P7_DIR ${LIBS_DIR}/P7)
set(P7_INCLUDE ${P7_DIR}/include)
set(P7_LIB ${P7_DIR}/lib)

find_library(P7Lib NAMES p7 PATHS ${P7_LIB} NO_DEFAULT_PATH)
message(STATUS "Find result: ${P7Lib}")


# status
message(STATUS "\n****************************************************")
message(STATUS "Directories:")
message(STATUS "project:\t${PROJDIR}")
message(STATUS "bin:    \t${BIN_DIR}")
message(STATUS "include \t${INCLUDE_DIR}")
message(STATUS "libs:   \t${LIBS_DIR}")
message(STATUS "boost:  \t${BOOST_DIR}")
message(STATUS "P7:     \t${P7_DIR}")
message(STATUS "\n****************************************************")


# network engine
add_subdirectory(hermesnet)
# test server app
add_subdirectory(server)
# test client app
add_subdirectory(client)
# replication test with raylib
#add_subdirectory(rayrep)
# godot wrapper
#add_subdirectory(godotnet)

message(STATUS "All Done --")

