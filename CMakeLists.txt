cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

# conan is required hard, library name, version and description are configured from conan
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION})

conan_basic_setup(TARGETS)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(SPDLOG_DEBUG_ON SPDLOG_TRACE_ON SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
endif ()

add_executable(${CONAN_PACKAGE_NAME} src/main.cpp)

conan_target_link_libraries(${CONAN_PACKAGE_NAME})

add_executable(${CONAN_PACKAGE_NAME}_fractal src/main_fractal.cpp)

conan_target_link_libraries(${CONAN_PACKAGE_NAME}_fractal)