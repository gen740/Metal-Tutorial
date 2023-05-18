cmake_minimum_required(VERSION 3.20)

project(Metal CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Apple-cpp)

file(GLOB MAIN_SOUCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cc)
add_executable(Metal ${MAIN_SOUCES})

target_include_directories(
  Metal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/metal-cpp
               ${CMAKE_CURRENT_LIST_DIR}/metal-cpp-extensions)

target_link_libraries(
  Metal
  AGUI
)
