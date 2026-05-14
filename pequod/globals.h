//
// Created by smadhur on 4/10/2026.
//

#ifndef PEQUOD_ENGINE_VERSION
#define PEQUOD_ENGINE_VERSION "dev"
#endif

#ifndef PEQUODENGINE_GLOBALS_H
#define PEQUODENGINE_GLOBALS_H

#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>
#include <vector>

#ifdef PEQUOD_GRAPHICS_D3D11
#include <DirectXMath.h>
#include <intsafe.h>
#endif

using entity_id = uint16_t;
using kEntityId = uint16_t;
#define height_s (sapp_heightf() * (1.0f / ZOOM))
#define width_s (sapp_widthf() * (1.0f / ZOOM))

#ifdef PEQUOD_GRAPHICS_D3D11
using PQ_FLOAT2 = DirectX::XMFLOAT2;
using PQ_FLOAT3 = DirectX::XMFLOAT3;
using PQ_FLOAT4 = DirectX::XMFLOAT4;
using PQ_MATRIX = DirectX::XMFLOAT4X4;
#else
using PQ_FLOAT2 = glm::vec2;
using PQ_FLOAT3 = glm::vec3;
using PQ_FLOAT4 = glm::vec4;
using PQ_MATRIX = glm::mat4;
using UINT = uint32_t;
#endif

struct Vertex {
  PQ_FLOAT3 position;
  PQ_FLOAT3 color;
  PQ_FLOAT2 uv;
  PQ_FLOAT3 normal = {0.0f, 1.0f, 0.0f};
};

struct StaticVertex {
  PQ_FLOAT3 position;
  PQ_FLOAT4 color;
  PQ_FLOAT2 uv;
  PQ_FLOAT4 atlas_uv;
  PQ_FLOAT3 normal = {0.0f, 1.0f, 0.0f};
};

/**
 * @brief Directional sun light + ambient.
 *
 * One per scene; populated once at scene setup and uploaded each frame.
 */
struct SunLight {
  PQ_FLOAT3 direction = {-0.55f, -0.7f, -0.35f};  // from sun toward world
  float intensity = 1.0f;
  PQ_FLOAT3 color = {1.0f, 0.95f, 0.85f};
  float saturation = 1.;  // 0 = grayscale, 1 = original, >1 boosts color
  PQ_FLOAT3 ambient = {0.15f, 0.17f, 0.22f};

  // Shadow tuning — not uploaded to GPU; consumed by the shadow pass for
  // computing the orthographic light frustum.
  float shadow_extent = 400.0f;  // half-side of the shadow frustum
};

/**
 * @brief Describes how a singular object can be drawn
 *
 * Primitives are the minimum basic requirement for a renderer to draw
 * something on the screen. It will thus contain things like vertices,
 * indices, textures, uv's etc.
 */
struct Primitive {
  std::vector<Vertex> vertices_;
  std::vector<UINT> indices_;
  glm::vec3 scale_;
  glm::vec3 world_position_;
  glm::mat4 rotation_matrix_ = glm::mat4(1.0f);
  float opacity_ = 1.0f;
  glm::vec4 atlas_uv_ = glm::vec4(0.0f);
};

/**
 * @brief Contains per-object information required to render
 *
 * These can be changing often and provide per-object required to render
 * correctly.
 */
struct VsModelBuffer {
  PQ_FLOAT3 scale;
  float opacity;
  PQ_MATRIX world_position;
  PQ_FLOAT4 atlas_uv;
};

#define PEQUOD_SAFE_FREE(pointer) \
  {                               \
    delete pointer;               \
    pointer = nullptr;            \
  }
#endif  // PEQUODENGINE_GLOBALS_H
