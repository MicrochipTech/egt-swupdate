cmake_minimum_required(VERSION 3.12)

set(executable_name egt-swupdate)

project(egt-swupdate
        VERSION 2.0.0
        LANGUAGES CXX
)

configure_file(version.h.in version.h)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX $ENV{exec_prefix})
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(EGT REQUIRED libegt)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(LIBCONFIG REQUIRED libconfig++)
pkg_check_modules(LIBUBOOTENV REQUIRED libubootenv)
pkg_check_modules(LIBCRYPTO REQUIRED libcrypto)
pkg_check_modules(CURL REQUIRED libcurl)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/cxxopts/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
add_subdirectory(external/cxxopts)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

include_directories(include
        ${PROJECT_BINARY_DIR}
        ${CAIRO_INCLUDE_DIRS}
        ${EGT_INCLUDE_DIRS}
        ${LIBCONFIG_INCLUDE_DIRS}
        ${LIBUBOOTENV_INCLUDE_DIRS}
        ${LIBCRYPTO_DIRS}
        ${CURL_INCLUDE_DIRS}
        external/cxxopts/include
        external/json/single_include
)

add_executable(${executable_name}
        src/main.cpp
        src/mainwin.cpp
        src/http.cpp
)

target_link_libraries(
        ${executable_name}
        ${EGT_LIBRARIES}
        ${LIBCONFIG_LIBRARIES}
        ${LIBUBOOTENV_LIBRARIES}
        ${LIBCRYPTO_LIBRARIES}
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${executable_name} DESTINATION bin)

if(supported)
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET ${executable_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()
