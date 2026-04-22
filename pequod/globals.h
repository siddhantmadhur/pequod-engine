//
// Created by smadhur on 4/10/2026.
//

#ifndef PEQUODENGINE_GLOBALS_H
#define PEQUODENGINE_GLOBALS_H

#include <DirectXMath.h>
#include <intsafe.h>

#include <cstdint>
#include <vector>

using entity_id = uint16_t;
#define height_s (sapp_heightf() * (1.0f / ZOOM))
#define width_s (sapp_widthf() * (1.0f / ZOOM))
using PQ_FLOAT3 = DirectX::XMFLOAT3;

using Position = PQ_FLOAT3;
using Color = PQ_FLOAT3;

struct Vertex {
  Position position;
  Color color;
};

/**
 * @brief Describes how a singular object can be drawn
 *
 * Primitives are the minimum basic requirement for a renderer to draw
 * something on the screen. It will thus contain things like vertices,
 * indices, textures, uv's etc.
 */
struct Primitive {
  std::vector<Vertex> vertices;
  std::vector<UINT> indices;
};

// Handles information on a per-model basis
struct VsModelBuffer {
  PQ_FLOAT3 scale;
  float opacity;
};
#endif  // PEQUODENGINE_GLOBALS_H
