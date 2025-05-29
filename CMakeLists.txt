cmake_minimum_required(VERSION 3.10)
project(Cite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE SOURCES
  src/*.cpp
  formatters/*.cpp
  parsers/*.cpp
)
add_executable(cite ${SOURCES})

find_package(CURL REQUIRED)
target_link_libraries(cite PRIVATE CURL::libcurl)

# This automatically handles include dirs for nlohmann_json
target_link_libraries(cite PRIVATE nlohmann_json::nlohmann_json)

# Your own includes
target_include_directories(cite PRIVATE ${CMAKE_SOURCE_DIR}/include)
