#include <engine/engine.hh>
#include <iostream>

void engine::init() {
    std::cout << "hello, world \n" << std::endl;
}

void engine::cleanup() {}

void engine::frame_cb() {}