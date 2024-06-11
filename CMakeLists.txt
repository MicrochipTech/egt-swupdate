cmake_minimum_required(VERSION 3.12)

set(executable_name egt-swupdate)

project(egt-swupdate
        VERSION 0.1.0
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

include_directories(include 
        ${PROJECT_BINARY_DIR}
        ${CAIRO_INCLUDE_DIRS}
        ${EGT_INCLUDE_DIRS}
        external/cxxopts/include
)

add_executable(${executable_name}
        src/main.cpp
        src/mainwin.cpp
)

target_link_libraries(
        ${executable_name}
        ${EGT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${executable_name} DESTINATION bin)
