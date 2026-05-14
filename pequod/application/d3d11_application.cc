//
// Created by smadhur on 4/5/2026.
//
#include "d3d11_application.h"

#ifdef PEQUOD_GRAPHICS_D3D11

#include <imgui/imgui.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

#include <DirectXMath.h>
#include <GLFW/glfw3native.h>
#include <d3dcompiler.h>
#include <imgui/backends/imgui_impl_dx11.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <string>

#include "debugger/debugger.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "os/filesystem.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

using std::string;

namespace Pequod {
D3D11Application::D3D11Application(const std::string &window_title,
                                   float initial_width, float initial_height)
    : Application(window_title, initial_width, initial_height) {}

void D3D11Application::ImGuiNewFrame() { ImGui_ImplDX11_NewFrame(); }

bool D3D11Application::Initialize() {
  if (!Application::Initialize()) {
    PDebug::error("Could not initialize GLFW application");
    return false;
  }

  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory_)))) {
    PDebug::error("Could not create DXGIFactory...");
    return false;
  }

  // Create dxgi factory
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
  constexpr D3D_FEATURE_LEVEL deviceFeatureLevel =
      D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
  if (FAILED(D3D11CreateDevice(
          nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr,
          creationFlags, &deviceFeatureLevel, 1, D3D11_SDK_VERSION, &device_,
          nullptr, &deviceContext_))) {
    PDebug::error("D3D11: Failed to create device");
    return false;
  }

  // create device and device context
  DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
  swapChainDescriptor.Width = GetWidth();
  swapChainDescriptor.Height = GetHeight();
  swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDescriptor.SampleDesc.Count = 1;
  swapChainDescriptor.SampleDesc.Quality = 0;
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescriptor.BufferCount = 2;
  swapChainDescriptor.SwapEffect =
      DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
  swapChainDescriptor.Flags = {};
  DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
  swapChainFullscreenDescriptor.Windowed = true;

  auto win32Window = glfwGetWin32Window(GetWindow());
  if (win32Window == nullptr) {
    PDebug::error("Could not get win32 from glfw");
    return false;
  }
  if (FAILED(dxgiFactory_->CreateSwapChainForHwnd(
          device_.Get(), win32Window, &swapChainDescriptor,
          &swapChainFullscreenDescriptor, nullptr, &swapchain_))) {
    PDebug::error("DXGI: Could not create swapchain");
    return false;
  }

  if (!CreateSwapchainResources()) {
    PDebug::error("Could not create swapchain resource");
    return false;
  }

  ImGui_ImplDX11_Init(device_.Get(), deviceContext_.Get());

  return true;
}

void D3D11Application::OnNewTick() {
  vertex_buffer_.clear();
  index_buffer_.clear();

  for (const auto &primitive : primitives_) {
    {
      // Set index buffer
      auto offset = vertex_buffer_.size();
      for (UINT index : primitive.indices_) {
        index_buffer_.push_back(index + offset);
      }
    }
    {
      // Set vertex buffer for object
      vertex_buffer_.insert(vertex_buffer_.end(), primitive.vertices_.begin(),
                            primitive.vertices_.end());
    }
  }
}

bool D3D11Application::CreateSwapchainResources() {
  ComPtr<ID3D11Texture2D> backBuffer = nullptr;
  if (FAILED(swapchain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
    PDebug::error("D3D11: Failed to get back buffer");
    return false;
  }

  if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                             &renderTarget_))) {
    PDebug::error("D3D11: Failed to create RTV");
    return false;
  }

  D3D11_TEXTURE2D_DESC depth_desc = {};
  depth_desc.Width = GetWidth();
  depth_desc.Height = GetHeight();
  depth_desc.MipLevels = 1;
  depth_desc.ArraySize = 1;
  depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_desc.SampleDesc.Count = 1;
  depth_desc.SampleDesc.Quality = 0;
  depth_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  if (FAILED(device_->CreateTexture2D(&depth_desc, nullptr,
                                      &depth_stencil_buffer_))) {
    PDebug::error("D3D11: Failed to create depth stencil buffer");
    return false;
  }
  if (FAILED(device_->CreateDepthStencilView(depth_stencil_buffer_.Get(),
                                             nullptr, &depth_stencil_view_))) {
    PDebug::error("D3D11: Failed to create depth stencil view");
    return false;
  }

  return true;
}

void D3D11Application::DestroySwapchainResources() {
  renderTarget_.Reset();
  depth_stencil_view_.Reset();
  depth_stencil_buffer_.Reset();
}

bool D3D11Application::CreateShadowResources() {
  constexpr UINT kShadowMapSize = 8192;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = kShadowMapSize;
  desc.Height = kShadowMapSize;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  // Typeless so we can have both a depth DSV and a R32 SRV.
  desc.Format = DXGI_FORMAT_R32_TYPELESS;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  if (FAILED(device_->CreateTexture2D(&desc, nullptr, &shadow_map_tex_))) {
    PDebug::error("D3D11: Failed to create shadow map texture");
    return false;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
  dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
  dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  dsv_desc.Texture2D.MipSlice = 0;
  if (FAILED(device_->CreateDepthStencilView(shadow_map_tex_.Get(), &dsv_desc,
                                             &shadow_dsv_))) {
    PDebug::error("D3D11: Failed to create shadow DSV");
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = 1;
  srv_desc.Texture2D.MostDetailedMip = 0;
  if (FAILED(device_->CreateShaderResourceView(shadow_map_tex_.Get(), &srv_desc,
                                               &shadow_srv_))) {
    PDebug::error("D3D11: Failed to create shadow SRV");
    return false;
  }
  return true;
}

template <typename T>
bool D3D11Application::MapBuffer(ComPtr<ID3D11Buffer> gpu_buffer,
                                 const std::vector<T> &vertex_buffer) {
  if (vertex_buffer.size() == 0) return false;
  D3D11_MAPPED_SUBRESOURCE mapped_subresource;
  deviceContext_->Map(gpu_buffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
                      0, &mapped_subresource);
  memcpy(mapped_subresource.pData, &vertex_buffer[0],
         sizeof(T) * vertex_buffer.size());
  deviceContext_->Unmap(gpu_buffer.Get(), 0);
  return true;
}

template <typename T>
bool D3D11Application::MapBuffer(ComPtr<ID3D11Buffer> gpu_buffer,
                                 const T &data) {
  D3D11_MAPPED_SUBRESOURCE mapped_subresource;
  deviceContext_->Map(gpu_buffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
                      0, &mapped_subresource);
  memcpy(mapped_subresource.pData, &data, sizeof(T));
  deviceContext_->Unmap(gpu_buffer.Get(), 0);
  return true;
}

bool D3D11Application::OnLoad() {
#ifndef PEQUOD_SHADER_PATH
  PDebug::error("Could not locate header: PEQUOD_SHADER_PATH");
  return false;
#endif

  fs::path shader_path = PEQUOD_SHADER_PATH;

  fs::path vertex_shader_path = shader_path / "main.vs.hlsl";

  ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
  vertexShader_ = CreateVertexShader(vertex_shader_path, vertexShaderBlob);
  if (vertexShader_ == nullptr) {
    return false;
  }

  constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] = {
      {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
       offsetof(Vertex, position),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
       offsetof(Vertex, color),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0,
       offsetof(Vertex, uv),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
       offsetof(Vertex, normal),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  if (FAILED(device_->CreateInputLayout(
          vertexInputLayoutInfo, _countof(vertexInputLayoutInfo),
          vertexShaderBlob->GetBufferPointer(),
          vertexShaderBlob->GetBufferSize(), &vertexLayout_))) {
    PDebug::error("D3D11: Failed to create default vertex input layout");
    return false;
  }

  fs::path static_vs_path = shader_path / "static.vs.hlsl";
  ComPtr<ID3DBlob> staticVertexShaderBlob = nullptr;
  static_vertex_shader_ =
      CreateVertexShader(static_vs_path, staticVertexShaderBlob);
  if (static_vertex_shader_ == nullptr) {
    return false;
  }

  constexpr D3D11_INPUT_ELEMENT_DESC staticVertexInputLayoutInfo[] = {
      {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
       offsetof(StaticVertex, position),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
       offsetof(StaticVertex, color),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0,
       offsetof(StaticVertex, uv),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
       offsetof(StaticVertex, atlas_uv),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
       offsetof(StaticVertex, normal),
       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  if (FAILED(device_->CreateInputLayout(
          staticVertexInputLayoutInfo, _countof(staticVertexInputLayoutInfo),
          staticVertexShaderBlob->GetBufferPointer(),
          staticVertexShaderBlob->GetBufferSize(), &static_vertex_layout_))) {
    PDebug::error("D3D11: Failed to create static vertex input layout");
    return false;
  }

  D3D11_BUFFER_DESC bufferInfo = {};
  bufferInfo.ByteWidth = sizeof(Vertex) * 64000;  // Max no. of vertices
  bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
  bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
  bufferInfo.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  if (FAILED(device_->CreateBuffer(&bufferInfo, nullptr, &triangleVertices_))) {
    PDebug::error("D3D11: Failed to create triangle vertex buffer");
    return false;
  }
  bufferInfo.ByteWidth = sizeof(StaticVertex) * 640000;
  if (FAILED(device_->CreateBuffer(&bufferInfo, nullptr, &static_vertices_))) {
    PDebug::error("D3D11: Failed to create static vertex buffer");
    return false;
  }

  D3D11_BUFFER_DESC indices_buffer_info = {};
  indices_buffer_info.ByteWidth = sizeof(UINT) * 64000;  // Max no. of indices
  indices_buffer_info.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
  indices_buffer_info.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
  indices_buffer_info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device_->CreateBuffer(&indices_buffer_info, nullptr,
                                   &indices_buffer_))) {
    PDebug::error("D3D11: Failed to create index buffer");
    return false;
  }
  if (FAILED(device_->CreateBuffer(&indices_buffer_info, nullptr,
                                   &static_indices_buffer_))) {
    PDebug::error("D3D11: Failed to create static index buffer");
    return false;
  }

  // Create cbuffers
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = sizeof(CameraCBuffer);
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device_->CreateBuffer(&cbDesc, nullptr, &camera_c_buffer_))) {
    PDebug::error("D3D11: Failed to create camera cbuffer");
    return false;
  }

  cbDesc.ByteWidth = sizeof(VsModelBuffer);
  if (FAILED(device_->CreateBuffer(&cbDesc, nullptr, &vs_model_buffer_))) {
    PDebug::error("D3D11: Failed to create model cbuffer");
    return false;
  }

  cbDesc.ByteWidth = sizeof(LightCBuffer);
  if (FAILED(device_->CreateBuffer(&cbDesc, nullptr, &light_c_buffer_))) {
    PDebug::error("D3D11: Failed to create light cbuffer");
    return false;
  }

  fs::path textured_ps_path = shader_path / "textured.ps.hlsl";
  textured_pixel_shader_ = CreatePixelShader(textured_ps_path);
  if (textured_pixel_shader_ == nullptr) {
    PDebug::error("D3D11: Failed to create textured pixel shader");
    return false;
  }

  D3D11_SAMPLER_DESC sampler_desc = {};
  sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
  if (FAILED(device_->CreateSamplerState(&sampler_desc, &texture_sampler_))) {
    PDebug::error("D3D11: Failed to create texture sampler state");
    return false;
  }

  // Comparison sampler for shadow map PCF lookups.
  D3D11_SAMPLER_DESC shadow_sampler_desc = {};
  shadow_sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  shadow_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  shadow_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  shadow_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  // Border = 1.0 means out-of-shadow-map samples are treated as fully lit.
  shadow_sampler_desc.BorderColor[0] = 1.0f;
  shadow_sampler_desc.BorderColor[1] = 1.0f;
  shadow_sampler_desc.BorderColor[2] = 1.0f;
  shadow_sampler_desc.BorderColor[3] = 1.0f;
  shadow_sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
  shadow_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
  if (FAILED(device_->CreateSamplerState(&shadow_sampler_desc,
                                         &shadow_cmp_sampler_))) {
    PDebug::error("D3D11: Failed to create shadow comparison sampler");
    return false;
  }

  // Shadow VS shaders — depth-only, one per vertex format. They reuse the
  // existing vertexLayout_ / static_vertex_layout_ input layouts because
  // they declare the same VSInput as the lit VSes.
  {
    ComPtr<ID3DBlob> blob;
    shadow_dynamic_vs_ =
        CreateVertexShader(shader_path / "shadow.vs.hlsl", blob);
    if (shadow_dynamic_vs_ == nullptr) {
      PDebug::error("D3D11: Failed to create shadow dynamic VS");
      return false;
    }
    ComPtr<ID3DBlob> blob2;
    shadow_static_vs_ =
        CreateVertexShader(shader_path / "shadow_static.vs.hlsl", blob2);
    if (shadow_static_vs_ == nullptr) {
      PDebug::error("D3D11: Failed to create shadow static VS");
      return false;
    }
  }

  // Shadow rasterizer: front-face culling reduces self-shadow acne on the
  // ground plane (we draw back faces into the shadow map).
  D3D11_RASTERIZER_DESC shadow_rs_desc = {};
  shadow_rs_desc.FillMode = D3D11_FILL_SOLID;
  shadow_rs_desc.CullMode = D3D11_CULL_NONE;
  shadow_rs_desc.FrontCounterClockwise = TRUE;
  shadow_rs_desc.DepthClipEnable = TRUE;
  shadow_rs_desc.DepthBias = 64;
  shadow_rs_desc.SlopeScaledDepthBias = 2.0f;
  if (FAILED(device_->CreateRasterizerState(&shadow_rs_desc,
                                            &shadow_rasterizer_state_))) {
    PDebug::error("D3D11: Failed to create shadow rasterizer state");
    return false;
  }

  if (!CreateShadowResources()) {
    return false;
  }

  D3D11_BLEND_DESC blendDesc = {};
  blendDesc.RenderTarget[0].BlendEnable = TRUE;
  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;
  if (FAILED(device_->CreateBlendState(&blendDesc, &blendState_))) {
    PDebug::error("D3D11: Failed to create blend state");
    return false;
  }

  D3D11_RASTERIZER_DESC rasterizer_desc = {};
  rasterizer_desc.FillMode = D3D11_FILL_SOLID;
  rasterizer_desc.CullMode = D3D11_CULL_BACK;
  rasterizer_desc.FrontCounterClockwise = TRUE;
  rasterizer_desc.DepthClipEnable = TRUE;
  if (FAILED(device_->CreateRasterizerState(&rasterizer_desc,
                                            &rasterizer_state_))) {
    PDebug::error("D3D11: Failed to create rasterizer state");
    return false;
  }

  D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
  depth_stencil_desc.DepthEnable = TRUE;
  depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
  depth_stencil_desc.StencilEnable = FALSE;
  if (FAILED(device_->CreateDepthStencilState(&depth_stencil_desc,
                                              &depth_stencil_state_))) {
    PDebug::error("D3D11: Failed to create depth stencil state");
    return false;
  }

  // Single dynamic atlas texture; uploaded only when the atlas changes.
  TextureAtlas &atlas = game_scene_->GetAtlas();
  D3D11_TEXTURE2D_DESC atlas_desc = {};
  atlas_desc.Width = static_cast<UINT>(atlas.GetWidth());
  atlas_desc.Height = static_cast<UINT>(atlas.GetHeight());
  atlas_desc.MipLevels = 1;
  atlas_desc.ArraySize = 1;
  atlas_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  atlas_desc.SampleDesc.Count = 1;
  atlas_desc.Usage = D3D11_USAGE_DYNAMIC;
  atlas_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  atlas_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device_->CreateTexture2D(&atlas_desc, nullptr, &atlas_texture_))) {
    PDebug::error("D3D11: Failed to create atlas texture");
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC atlas_srv_desc = {};
  atlas_srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  atlas_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  atlas_srv_desc.Texture2D.MipLevels = 1;
  atlas_srv_desc.Texture2D.MostDetailedMip = 0;
  if (FAILED(device_->CreateShaderResourceView(atlas_texture_.Get(),
                                               &atlas_srv_desc, &atlas_srv_))) {
    PDebug::error("D3D11: Failed to create atlas SRV");
    return false;
  }

  return true;
}

void D3D11Application::Render() {
  if (game_scene_) {
    glm::mat4x4 camera_proj_view = {};
    // Set camera buffer
    if (game_scene_->GetCameraProj(camera_proj_view)) {
      // Create camera buffer
      CameraCBuffer camera_c_buffer{
          .mWorldViewProj = DirectX::XMFLOAT4X4{&camera_proj_view[0][0]},
          .mResolution =
              PQ_FLOAT2{game_scene_->GetWidth(), game_scene_->GetHeight()},
      };
      MapBuffer(camera_c_buffer_, camera_c_buffer);
    }
  }

  D3D11_VIEWPORT viewport = {};
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = static_cast<float>(GetWidth());
  viewport.Height = static_cast<float>(GetHeight());
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  constexpr float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
  constexpr UINT dynamicVertexStride = sizeof(Vertex);
  constexpr UINT staticVertexStride = sizeof(StaticVertex);
  constexpr UINT vertexOffset = 0;

  constexpr UINT indexStride = sizeof(UINT);
  constexpr UINT indexOffset = 0;
  // Scene renders directly to the swapchain backbuffer — bloom pipeline
  // removed. The lit pixel shader gamma-corrects on output.
  deviceContext_->ClearRenderTargetView(renderTarget_.Get(), clearColor);
  deviceContext_->ClearDepthStencilView(depth_stencil_view_.Get(),
                                        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                        1.0f, 0);
  deviceContext_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  deviceContext_->RSSetViewports(1, &viewport);
  deviceContext_->RSSetState(rasterizer_state_.Get());
  deviceContext_->OMSetRenderTargets(1, renderTarget_.GetAddressOf(),
                                     depth_stencil_view_.Get());
  deviceContext_->OMSetDepthStencilState(depth_stencil_state_.Get(), 0);

  ID3D11Buffer *constant_buffers[1] = {camera_c_buffer_.Get()};
  deviceContext_->VSSetConstantBuffers(0, 1, constant_buffers);

  // ===== Compute + upload light cbuffer (includes shadow_view_proj) =====
  glm::mat4 shadow_view_proj_glm = glm::mat4(1.0f);
  if (game_scene_) {
    const SunLight &sun = game_scene_->GetSunLight();
    LightCBuffer light_cb{};
    float dx = sun.direction.x, dy = sun.direction.y, dz = sun.direction.z;
    float len = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 1e-6f) len = 1.0f;
    glm::vec3 sun_dir(dx / len, dy / len, dz / len);
    light_cb.sun_direction = PQ_FLOAT3{sun_dir.x, sun_dir.y, sun_dir.z};
    light_cb.sun_intensity = sun.intensity;
    light_cb.sun_color = sun.color;
    light_cb.saturation = sun.saturation;
    light_cb.ambient_color = sun.ambient;

    // Orthographic light projection covering the playable terrain. Hard-coded
    // for the shipwreck scene — refine later if scenes grow.
    const float kHalfExtent = sun.shadow_extent;
    constexpr float kShadowDistance = 900.0f;
    constexpr float kSceneNear = 1.0f;
    constexpr float kSceneFar = 1800.0f;
    glm::vec3 scene_center(500.0f, 0.0f, 500.0f);
    glm::vec3 light_pos = scene_center - sun_dir * kShadowDistance;
    glm::vec3 up = std::abs(sun_dir.y) > 0.99f ? glm::vec3(0, 0, 1)
                                               : glm::vec3(0, 1, 0);
    glm::mat4 light_view = glm::lookAtLH(light_pos, scene_center, up);
    glm::mat4 light_proj = glm::orthoLH_ZO(
        -kHalfExtent, kHalfExtent, -kHalfExtent, kHalfExtent, kSceneNear,
        kSceneFar);
    shadow_view_proj_glm = light_proj * light_view;
    light_cb.shadow_view_proj =
        DirectX::XMFLOAT4X4{&shadow_view_proj_glm[0][0]};

    MapBuffer(light_c_buffer_, light_cb);
  }
  ID3D11Buffer *light_cb_arr[1] = {light_c_buffer_.Get()};
  deviceContext_->VSSetConstantBuffers(2, 1, light_cb_arr);
  deviceContext_->PSSetConstantBuffers(2, 1, light_cb_arr);

  float blendFactor[4] = {0, 0, 0, 0};
  int copies = 0;

  // ===== Prepare geometry buffers (mapped once, drawn twice) =====
  std::vector<StaticVertex> static_vertices;
  std::vector<UINT> static_indices;
  if (game_scene_) {
    // Atlas upload (texture data; not used by shadow pass but cheap).
    TextureAtlas &atlas = game_scene_->GetAtlas();
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(deviceContext_->Map(atlas_texture_.Get(), 0,
                                      D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
      const int aw = atlas.GetWidth();
      const int ah = atlas.GetHeight();
      const uint8_t *src = atlas.GetData();
      uint8_t *dst = static_cast<uint8_t *>(mapped.pData);
      const size_t src_pitch = static_cast<size_t>(aw) * 4;
      for (int row = 0; row < ah; ++row) {
        memcpy(dst + row * mapped.RowPitch, src + row * src_pitch, src_pitch);
      }
      deviceContext_->Unmap(atlas_texture_.Get(), 0);
      atlas.ClearGpuUploadFlag();
    }

    static_vertices = game_scene_->GetStaticVertices();
    static_indices = game_scene_->GetStaticIndices();
    if (!static_vertices.empty()) {
      MapBuffer(static_vertices_, static_vertices);
      MapBuffer(static_indices_buffer_, static_indices);
    }
    if (!vertex_buffer_.empty()) {
      MapBuffer(triangleVertices_, vertex_buffer_);
      MapBuffer(indices_buffer_, index_buffer_);
    }
  }

  // Helper lambda to issue per-primitive dynamic draws. `set_object_cb`
  // controls whether to upload the model cbuffer (true for both passes).
  auto draw_dynamic_primitives = [&]() {
    if (primitives_.empty() || vertex_buffer_.empty()) return;
    int vertex_offset = 0;
    int index_offset = 0;
    for (const auto &primitive : primitives_) {
      VsModelBuffer vs_model_buffer = {};
      vs_model_buffer.scale = PQ_FLOAT3{&primitive.scale_[0]};
      vs_model_buffer.opacity = primitive.opacity_;
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, primitive.world_position_);
      model = model * primitive.rotation_matrix_;
      vs_model_buffer.world_position = PQ_MATRIX{&model[0][0]};
      vs_model_buffer.atlas_uv =
          PQ_FLOAT4{primitive.atlas_uv_.x, primitive.atlas_uv_.y,
                    primitive.atlas_uv_.z, primitive.atlas_uv_.w};
      MapBuffer(vs_model_buffer_, vs_model_buffer);
      copies += 1;
      ID3D11Buffer *per_object_cbuffer[1] = {vs_model_buffer_.Get()};
      deviceContext_->VSSetConstantBuffers(1, 1, per_object_cbuffer);
      deviceContext_->DrawIndexed(primitive.indices_.size(), index_offset, 0);
      index_offset += primitive.indices_.size();
      vertex_offset += primitive.vertices_.size();
    }
  };

  // ===== SHADOW PASS — render geometry depth from the sun's POV =====
  if (game_scene_) {
    constexpr float kShadowMapSize = 8192.0f;
    D3D11_VIEWPORT shadow_viewport = {};
    shadow_viewport.Width = kShadowMapSize;
    shadow_viewport.Height = kShadowMapSize;
    shadow_viewport.MinDepth = 0.0f;
    shadow_viewport.MaxDepth = 1.0f;

    ID3D11RenderTargetView *null_rtv[1] = {nullptr};
    deviceContext_->OMSetRenderTargets(1, null_rtv, shadow_dsv_.Get());
    deviceContext_->ClearDepthStencilView(shadow_dsv_.Get(), D3D11_CLEAR_DEPTH,
                                          1.0f, 0);
    deviceContext_->RSSetViewports(1, &shadow_viewport);
    deviceContext_->RSSetState(shadow_rasterizer_state_.Get());
    deviceContext_->OMSetDepthStencilState(depth_stencil_state_.Get(), 0);
    deviceContext_->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
    deviceContext_->PSSetShader(nullptr, nullptr, 0);  // depth-only

    if (!static_vertices.empty()) {
      deviceContext_->IASetInputLayout(static_vertex_layout_.Get());
      deviceContext_->VSSetShader(shadow_static_vs_.Get(), nullptr, 0);
      deviceContext_->IASetVertexBuffers(0, 1, static_vertices_.GetAddressOf(),
                                         &staticVertexStride, &vertexOffset);
      deviceContext_->IASetIndexBuffer(static_indices_buffer_.Get(),
                                       DXGI_FORMAT_R32_UINT, 0);
      deviceContext_->DrawIndexed(static_indices.size(), 0, 0);
    }
    if (!primitives_.empty() && !vertex_buffer_.empty()) {
      deviceContext_->IASetInputLayout(vertexLayout_.Get());
      deviceContext_->VSSetShader(shadow_dynamic_vs_.Get(), nullptr, 0);
      deviceContext_->IASetVertexBuffers(0, 1, triangleVertices_.GetAddressOf(),
                                         &dynamicVertexStride, &vertexOffset);
      deviceContext_->IASetIndexBuffer(indices_buffer_.Get(),
                                       DXGI_FORMAT_R32_UINT, 0);
      draw_dynamic_primitives();
    }
  }

  // ===== SCENE PASS — render to backbuffer with shadow lookup =====
  deviceContext_->RSSetViewports(1, &viewport);
  deviceContext_->RSSetState(rasterizer_state_.Get());
  deviceContext_->OMSetRenderTargets(1, renderTarget_.GetAddressOf(),
                                     depth_stencil_view_.Get());
  deviceContext_->OMSetDepthStencilState(depth_stencil_state_.Get(), 0);
  deviceContext_->OMSetBlendState(blendState_.Get(), blendFactor, 0xFFFFFFFF);

  if (game_scene_) {
    // Bind atlas + shadow map for the lit pixel shader.
    ID3D11ShaderResourceView *srvs[2] = {atlas_srv_.Get(), shadow_srv_.Get()};
    deviceContext_->PSSetShaderResources(0, 2, srvs);
    ID3D11SamplerState *samplers[2] = {texture_sampler_.Get(),
                                       shadow_cmp_sampler_.Get()};
    deviceContext_->PSSetSamplers(0, 2, samplers);
    deviceContext_->PSSetShader(textured_pixel_shader_.Get(), nullptr, 0);

    if (!static_vertices.empty()) {
      deviceContext_->IASetInputLayout(static_vertex_layout_.Get());
      deviceContext_->VSSetShader(static_vertex_shader_.Get(), nullptr, 0);
      deviceContext_->IASetVertexBuffers(0, 1, static_vertices_.GetAddressOf(),
                                         &staticVertexStride, &vertexOffset);
      deviceContext_->IASetIndexBuffer(static_indices_buffer_.Get(),
                                       DXGI_FORMAT_R32_UINT, 0);
      deviceContext_->DrawIndexed(static_indices.size(), 0, 0);
    }

    if (!primitives_.empty() && !vertex_buffer_.empty()) {
      deviceContext_->IASetInputLayout(vertexLayout_.Get());
      deviceContext_->VSSetShader(vertexShader_.Get(), nullptr, 0);
      deviceContext_->IASetVertexBuffers(0, 1, triangleVertices_.GetAddressOf(),
                                         &dynamicVertexStride, &vertexOffset);
      deviceContext_->IASetIndexBuffer(indices_buffer_.Get(),
                                       DXGI_FORMAT_R32_UINT, 0);
      draw_dynamic_primitives();
    }

    // Unbind shadow SRV before the next frame's shadow pass writes to it.
    ID3D11ShaderResourceView *null_srvs[2] = {nullptr, nullptr};
    deviceContext_->PSSetShaderResources(0, 2, null_srvs);
  }

  {
    // Debug stats
    ImGui::SetNextWindowBgAlpha(0.3);
    ImGui::SetNextWindowPos(ImVec2{10.0, 10.0});
    ImGui::Begin("Render Stats", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("FPS: %d", average_fps_);
    ImGui::Text("GPU Memory Copies: %d", copies);
    ImGui::Text("Primitives: %lu", primitives_.size());
    ImGui::End();
  }

  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  swapchain_->Present(1, 0);
}

void D3D11Application::OnResize(int32_t width, int32_t height) {
  Application::OnResize(width, height);
  deviceContext_->Flush();
  DestroySwapchainResources();

  if (FAILED(swapchain_->ResizeBuffers(
          0, width, height, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, 0))) {
    PDebug::error("D3D11: Failed to recreate SwapChain buffers");
    return;
  }

  CreateSwapchainResources();
}

// Shader stuff

bool D3D11Application::CompileShader(const std::wstring &fileName,
                                     const std::string &entryPoint,
                                     const std::string &profile,
                                     ComPtr<ID3DBlob> &shaderBlob) {
  constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

  ComPtr<ID3DBlob> tempShaderBlob = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;

  if (FAILED(D3DCompileFromFile(fileName.data(), nullptr,
                                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                entryPoint.data(), profile.data(), compileFlags,
                                0, &tempShaderBlob, &errorBlob))) {
    PDebug::error("D3D11: Failed to read shader from file");
    if (errorBlob != nullptr) {
      PDebug::error("D3D11: MESSAGE: ");
      PDebug::error("{}",
                    static_cast<const char *>(errorBlob->GetBufferPointer()));
    }
    return false;
  }

  shaderBlob = std::move(tempShaderBlob);
  PDebug::info("Compiled shader!");
  return true;
}

D3D11Application::ComPtr<ID3D11VertexShader>
D3D11Application::CreateVertexShader(const std::wstring &fileName,
                                     ComPtr<ID3DBlob> &vertexShaderBlob) const {
  if (!CompileShader(fileName, "Main", "vs_5_0", vertexShaderBlob)) {
    return nullptr;
  }

  ComPtr<ID3D11VertexShader> vertexShader;
  if (FAILED(device_->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                         vertexShaderBlob->GetBufferSize(),
                                         nullptr, &vertexShader))) {
    PDebug::error("D3D11: Failed to compile vertex shader");
    return nullptr;
  }

  return vertexShader;
}

D3D11Application::ComPtr<ID3D11PixelShader> D3D11Application::CreatePixelShader(
    const std::wstring &fileName) const {
  ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
  if (!CompileShader(fileName, "Main", "ps_5_0", pixelShaderBlob)) {
    return nullptr;
  }

  ComPtr<ID3D11PixelShader> pixelShader;
  if (FAILED(device_->CreatePixelShader(pixelShaderBlob->GetBufferPointer(),
                                        pixelShaderBlob->GetBufferSize(),
                                        nullptr, &pixelShader))) {
    PDebug::error("D3D11: Failed to compile pixel shader");
    return nullptr;
  }

  return pixelShader;
}
}  // namespace Pequod

#endif  // PEQUOD_GRAPHICS_D3D11
