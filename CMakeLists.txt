cmake_minimum_required(VERSION 3.7)
project(gameserver
  LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Game server for running and streaming data")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)

find_package(nlohmann_json 3.2.0 REQUIRED)
