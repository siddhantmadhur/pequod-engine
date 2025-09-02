cmake_minimum_required(VERSION 3.8)

project(MercuryEngine)
include_directories("third_party")


add_executable(mercury src/main.mm)

include_directories("lib")

target_link_libraries(mercury "-framework Cocoa")
target_link_libraries(mercury "-framework Metal")
target_link_libraries(mercury "-framework MetalKit")
target_link_libraries(mercury "-framework QuartzCore")