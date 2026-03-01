#include "scene.hh"
#include "engine/scene.hh"
#include "glm/common.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gameobjects/gameobject.hh>
#include <gameobjects/quad.hh>
#include <imgui/imgui.h>

#define ZOOM 2.0f
#define SCALE (1.0f/ZOOM) 
#define width 1280 * SCALE
#define height 720 * SCALE

void BreakoutScene::OnStart() {

    ImGui::LoadIniSettingsFromDisk("breakout_scene.ini");

    { // CAMERA

        // TODO: add createOrtho and createPerspective in the engine (maybe to scene?) itself to reduce this code as well


        Camera playerCam = Camera(16.0/9.0);
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
        playerCam.setPosition(pos);
        
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-pos.x, pos.y, 0.0f));
        float zoom = ZOOM;
        view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));


        glm::mat4 proj = glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f);

        playerCam.setProj(proj);
        playerCam.setView(view);

        SetPlayerCamera(playerCam);

    }


    { // PLAYER
        this->player = new Shapes::Quad(glm::vec2(8.0f, 2.0f), glm::vec2(width/4.0f, 16), glm::vec4(1.0f));
        AddObject(*player);

    }


}


#define playerPos player->GetPosition()
#define playerSize player->GetSize()

void BreakoutScene::OnUpdate() {
    float direction = 0;
    if (IsKeyPressed(SAPP_KEYCODE_LEFT)) {
        direction -= 1.0f;
    }
    if (IsKeyPressed(SAPP_KEYCODE_RIGHT)) {
        direction += 1.0f;
    }

    if (IsKeyPressed(SAPP_KEYCODE_LEFT_SHIFT) || IsKeyPressed(SAPP_KEYCODE_RIGHT_SHIFT)) {
        direction = direction / 4.0f;
    }

    #define PLAYER_SPEED 0.48f

    if (player) {
        player->Move(glm::vec3(direction * delta_t * PLAYER_SPEED, 0.0f, 0.0f));
        if (direction < 0) { // going left
            glm::vec3 pos = playerPos;
            pos.x = glm::max(playerSize.x + 2, pos.x);
            player->SetPosition(pos);
        }
        if (direction > 0) { // going left
            glm::vec3 pos = playerPos;
            pos.x = glm::min((width / 2.0f) - playerSize.x - 2, pos.x);
            player->SetPosition(pos);
        }
    }


    ImGui::Begin("pos", NULL, 0);
    
    ImGui::Text("player: (%.2f, %.2f)", playerPos.x, width);

    ImGui::End();

}
void BreakoutScene::OnEnd() {
    ImGui::SaveIniSettingsToDisk("breakout_scene.ini");
}
void BreakoutScene::OnEvent(const sapp_event* event) {
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }
}
