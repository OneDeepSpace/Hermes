cmake_minimum_required(VERSION 3.16)
project(hermesnet VERSION 0.5.0)


set(SOURCES
		${HERMESNET_DIR}/hermes/log/log.cpp
		${HERMESNET_DIR}/hermes/message/message.hpp
		${HERMESNET_DIR}/hermes/data_receiver/receiver.cpp
		${HERMESNET_DIR}/hermes/data_sender/sender.cpp
		${HERMESNET_DIR}/hermes/loop/mainloop.cpp)
        
set(LIBS
		${P7Lib}
		-ldl
		-pthread
		-lrt
		libboost_system.a
		libboost_iostream.a
		libboost_date_time.a
		libboost_atomic.a
		libboost_chrono.a
		libboost_regex.a
		libboost_filesystem.a
		libboost_thread.a)

# include & libs directories
set(LOCAL_LIB_DIRECTORIES
		${BOOST_LIBRARYDIR}
		${P7_LIB})

set(LOCAL_INC_DIRECTORIES
		${BOOST_INCLUDEDIR}
		${INCLUDE_DIR}
		${P7_INCLUDE})

include_directories(${LOCAL_INC_DIRECTORIES})
link_directories(${LOCAL_LIB_DIRECTORIES})

# build
add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)	# -fpic
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

# copy
add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:${PROJECT_NAME}>
#		${BIN_DIR}/libhermesnet.a
		${LIBS_DIR}/libhermesnet.a
#		COMMENT "Created \"libhermesnet.a\" at ${BIN_DIR}/")
		COMMENT "Created \"libhermesnet.a\" at ${LIBS_DIR}")