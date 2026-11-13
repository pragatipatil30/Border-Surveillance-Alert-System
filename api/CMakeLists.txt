cmake_minimum_required(VERSION 3.16)
project(border_command_center_api LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
find_package(Threads REQUIRED)

set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_FUZZER OFF CACHE BOOL "" FORCE)
set(CROW_INSTALL OFF CACHE BOOL "" FORCE)
set(CROW_ENABLE_SSL OFF CACHE BOOL "" FORCE)
set(CROW_ENABLE_COMPRESSION OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.3.1
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Crow)

add_executable(border_api
  server/main.cpp
  server/routes/system_routes.cpp
  server/routes/health_routes.cpp
  server/routes/dashboard_routes.cpp
  server/routes/records_routes.cpp
  server/routes/operational_routes.cpp
  server/services/intruder_repository.cpp
)
target_compile_features(border_api PRIVATE cxx_std_17)
target_include_directories(border_api PRIVATE server/include ../common)
target_link_libraries(border_api PRIVATE Crow::Crow Threads::Threads)

if(MSVC)
  target_compile_options(border_api PRIVATE /W4 /permissive-)
else()
  target_compile_options(border_api PRIVATE -Wall -Wextra -Wpedantic)
endif()
