cmake_minimum_required(VERSION 3.16)
project(client VERSION 1.0.0)

set(SRC_FILES
        ${PROJDIR}/client/test_client.cpp)

set(LIBS
        -ldl
        libhermesnet.a
        ${P7Lib}
        -pthread
        -lrt
        )

# include & libs directories
set(LOCAL_LIB_DIRECTORIES
        ${LIBS_DIR})

set(LOCAL_INC_DIRECTORIES
        ${BOOST_INCLUDEDIR}
        ${INCLUDE_DIR}
        ${P7_INCLUDE})

include_directories(${LOCAL_INC_DIRECTORIES})
link_directories(${LOCAL_LIB_DIRECTORIES})

# build
add_executable(${PROJECT_NAME} ${SRC_FILES})
add_dependencies(${PROJECT_NAME} hermesnet) # make lib before app
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

# copy
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${PROJECT_NAME}>
        ${BIN_DIR}/${PROJECT_NAME}
        COMMENT "Created \"${PROJECT_NAME}\" at ${BIN_DIR}/")