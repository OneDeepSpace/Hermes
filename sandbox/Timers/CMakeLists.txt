cmake_minimum_required(VERSION 3.16)
project(Timers VERSION 0.1.0)

# C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wcomment")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Directories
set(PROJDIR ${PROJECT_SOURCE_DIR})
set(BIN_DIR ${PROJDIR}/bin)

# Thirdparty
set(THIRDPARTY ${PROJDIR}/../../../thirdparty)
# Boost
set(BOOST_DIR ${THIRDPARTY}/boost_1_77_0)
set(BOOST_LIBS ${BOOST_DIR}/stage/lib)
set(BOOST_INCLUDE ${BOOST_DIR})

# Source files
file(GLOB SOURCE_GLOB ${PROJDIR}/*.cpp)
set(SOURCE ${SOURCE_GLOB}) #${PROJDIR}/object_pool.h)
# Used libs
set(LIBS -lboost_system -lboost_thread -pthread -ldl)

# Status
message(STATUS "\n*******************************************")
message(STATUS "Project\tdir:\t${PROJDIR}")
message(STATUS "Binnary\tdir:\t${BIN_DIR}")
message(STATUS "Boost\tlibs\tdir:\t${BOOST_LIBS}")
message(STATUS "Boost\tinclude\tdir:\t${BOOST_INCLUDE}")
message(STATUS "*******************************************\n")

# include all stuff
include_directories(${BOOST_INCLUDE})
link_directories(${BOOST_LIBS})

# nettool
add_executable(Timers ${SOURCE})
set_target_properties(Timers PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(Timers PUBLIC ${LIBS})

#копируем исполняемый в bin
add_custom_command(
        TARGET Timers POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/Timers
        ${BIN_DIR}/Timers)

# **********************************************

message(STATUS "Done. o/")