cmake_minimum_required(VERSION 3.1)
project(KISSPipe)

add_subdirectory(LibAPRS)
include_directories(LibAPRS/include)
include_directories(LibAPRS/thirdparty/spdlog/include)
file(GLOB SOURCES "src/*.cpp")

add_executable(kisspipe ${SOURCES})
target_link_libraries(kisspipe aprs)