//
// Created by smadhur on 4/30/2026.
//

#ifndef PEQUOD_ENGINE_TEXTURE_ATLAS_H
#define PEQUOD_ENGINE_TEXTURE_ATLAS_H
#include <map>
#include <memory>

#include "imstb_rectpack.h"

#include "properties/texture2d.h"
#include <vector>

namespace Pequod {
class TextureAtlas {
 public:
  TextureAtlas();
  ~TextureAtlas();
  void AddTexture(std::shared_ptr<Texture2D>);
  void UpdateAtlas();

 protected:
 private:
  int width_ = 1300;   // max size of the atlas
  int height_ = 1300;  // max size of the atlas
  stbrp_context ctx_{};
  stbrp_node nodes_[512] = {};
  std::vector<stbrp_rect> rects;
  std::vector<std::shared_ptr<Texture2D>> textures_;
  int img_id_ = 0;
  uint8_t* raw_data_ = nullptr;
  std::map<std::string, const stbi_uc*> images_;
  int last_img_id_updated_ = 0;
};
}  // namespace Pequod

#endif  // PEQUOD_ENGINE_TEXTURE_ATLAS_H
