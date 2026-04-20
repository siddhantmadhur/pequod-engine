//
// Created by smadhur on 4/20/2026.
//

#include "scene.h"

namespace Pequod {

GameScene::GameScene() {
  this->object_manager_ = std::make_unique<PObjectManager>();
}

} // Pequod