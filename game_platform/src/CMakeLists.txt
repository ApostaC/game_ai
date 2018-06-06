cmake_minimum_required(VERSION 2.6)

project(GamePlatform)
set(CMAKE_CC_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
ADD_COMPILE_OPTIONS(-DOUTPUT)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -lpthread")

include_directories(utils)
include_directories(factories)
include_directories(envs)
add_subdirectory(utils)
add_subdirectory(factories)
add_subdirectory(envs)

#OPTION(FORCE_TESTS "Build tests, regardless of build type." ON)
if (CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" OR FORCE_TESTS)
    ##add_subdirectory(test)
    ADD_EXECUTABLE(test test.cpp)
    TARGET_LINK_LIBRARIES(test LIBENVS LIBUTILS LIBFACTORIES)
    ADD_EXECUTABLE(test2 test2.cpp)
    INSTALL(TARGETS test test2
        DESTINATION for_test)
endif()


