#include <iostream>

#define SOKOL_IMPL
#define SOKOL_GLCORE
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_app.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_log.h>

#include <imgui/imgui.h>
#include <sokol/util/sokol_imgui.h>


//#include <engine/engine.cc>

static struct {
    sg_pass_action pass_action;
} state;

void engine_init() {
    sg_setup((sg_desc){
        .logger = {
            .func = slog_func,
        },
        .environment = sglue_environment(),
    });

    simgui_desc_t simgui_desc = { };
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);

    state.pass_action = (sg_pass_action){
        .colors = { { .load_action = SG_LOADACTION_CLEAR, .clear_value={0.2f, 0.3f, 0.3f, 1.0f}}}
    };
}
void engine_frame() {

    const int width = sapp_width();
    const int height = sapp_height();
    simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() });

    ImGui::ShowDemoWindow();

    sg_begin_pass((sg_pass){
        .action = state.pass_action,
        .swapchain = sglue_swapchain(),
    });
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void engine_event_cb(const sapp_event* event) {
    simgui_handle_event(event);
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        std::cout << event->type << std::endl;
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }
}
void engine_cleanup() {
    simgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[]) {
    return (sapp_desc){
        .init_cb = engine_init,
        .frame_cb = engine_frame,
        .cleanup_cb = engine_cleanup,
        .event_cb = engine_event_cb,
        .width = 1280,
        .height = 720,
        .window_title = "Mercury Engine",
        .logger = {
            .func = slog_func,
        },
    };
}
