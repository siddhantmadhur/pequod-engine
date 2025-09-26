cmake_minimum_required(VERSION 3.8)

project(MercuryEngine)
include_directories("third_party")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(mercury src/main.cc)

include_directories("lib")
#X11, Xi, Xcursor, GL, dl, pthread, m(?)


# Linux libraries
target_link_libraries(mercury "-lX11")
target_link_libraries(mercury "-lXi")
target_link_libraries(mercury "-lXcursor")
target_link_libraries(mercury "-lGL")
target_link_libraries(mercury "-ldl")
target_link_libraries(mercury "-lpthread")

# MacOS libraries
#target_link_libraries(mercury "-framework Cocoa")
#target_link_libraries(mercury "-framework Metal")
#target_link_libraries(mercury "-framework MetalKit")
#target_link_libraries(mercury "-framework QuartzCore")


