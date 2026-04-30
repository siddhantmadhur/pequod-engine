//
// Created by smadhur on 4/30/2026.
//

#define STB_RECT_PACK_IMPLEMENTATION
#include "texture_atlas.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include "globals.h"
#include "debugger/debugger.h"

namespace Pequod {

TextureAtlas::TextureAtlas() {
  stbrp_init_target(&ctx_, width_, height_, nodes_, 512);
}

void TextureAtlas::AddTexture(std::shared_ptr<Texture2D> texture) {
  if (!texture) {
    return;
  }

  if (images_.contains(texture->GetPath())) {
    return;
  }
  rects.push_back(stbrp_rect{
      .id = img_id_,
      .w = texture->GetWidth(),
      .h = texture->GetHeight(),
  });
  img_id_ += 1;
  stbrp_pack_rects(&ctx_, rects.data(), static_cast<int>(rects.size()));
  textures_.push_back(texture);
  images_[texture->GetPath()] = texture->GetData();
}

void TextureAtlas::UpdateAtlas() {
  if (img_id_ == last_img_id_updated_) {
    return;
  }
  last_img_id_updated_ = img_id_;
  uint8_t* raw_data = (uint8_t*)malloc(width_ * height_ * 4);
  for (stbrp_rect& rect : rects) {
    auto img = textures_[rect.id];
    for (int row = 0; row < rect.h; row++) {
      const uint8_t* src_row = img->GetData() + (size_t)(row * rect.w * 4);
      uint8_t* dest_row =
          raw_data + (size_t)((rect.y + row) * width_ + rect.x) * 4;
      memcpy(dest_row, src_row, (size_t)rect.w * 4);
    }

    auto img_width = img->GetWidth();
    auto img_height = img->GetHeight();
    glm::vec4 atlas_uv;
    atlas_uv.x = rect.x / width_;
    atlas_uv.y = rect.y / height_;
    atlas_uv.z = atlas_uv.x + img_width / width_;
    atlas_uv.z = atlas_uv.y + img_height / height_;
    img->SetAtlasUV(atlas_uv);
  }

  PDebug::info("Writing atlas...");
  stbi_write_png("atlas.png", width_, height_, 4, raw_data, 4 * width_);

  delete[] raw_data;
}

TextureAtlas::~TextureAtlas() { delete raw_data_; }

}  // namespace Pequod