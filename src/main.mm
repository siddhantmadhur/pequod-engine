#include <iostream>

#define SOKOL_METAL

#define SOKOL_IMPL
#include <sokol/sokol_app.h>

#include <engine/engine.cc>

void engine_init() {}
void engine_cb() {}
void engine_cleanup() {}

sapp_desc sokol_main(int argc, char *argv[]) {
  return (sapp_desc){
      .width = 640,
      .height = 480,
      .init_cb = engine::init,
      .frame_cb = engine::frame_cb,
      .cleanup_cb = engine::cleanup,
      .window_title = "Jester v0.1"
  };
}