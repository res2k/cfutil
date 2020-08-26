cmake_minimum_required (VERSION 3.8)

project ("cfutil")

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  message(STATUS "Using Conan build info")
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  set(BOOST_ROOT "${CONAN_BOOST_ROOT}")
else()
  message(STATUS "Not using Conan build info!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.73 REQUIRED COMPONENTS program_options)

add_compile_definitions(UNICODE _UNICODE NOMINMAX)
add_executable(cfutil)
target_sources(cfutil PRIVATE
               "cfutil.cpp"
               "Command.hpp"
               "CommandInfo.hpp"
               "CommandInfo.cpp"
               "ntdll.cpp"
               "ntdll.hpp"
               "util.cpp"
               "util.hpp")
target_link_libraries(cfutil PRIVATE Boost::disable_autolinking Boost::program_options)
target_link_libraries(cfutil PRIVATE cldapi.lib)
