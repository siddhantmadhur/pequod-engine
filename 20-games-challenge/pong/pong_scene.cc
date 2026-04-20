//
// Created by smadhur on 4/20/2026.
//

#include "pong_scene.h"

#include "debugger/debugger.h"

void PongScene::OnStart() {};

void PongScene::OnFrame(double delta_t) {};

void PongScene::OnTick(double delta_t) {
  PDebug::log("Tick: {}", delta_t);
};

void PongScene::OnDestroy() {};