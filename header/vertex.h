#pragma once

#include "vulkan/vulkan.hpp"
#include <glm/glm.hpp>

namespace app {

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static auto GetAttribute()
      -> std::array<vk::VertexInputAttributeDescription,
          2> {
    std::array<vk::VertexInputAttributeDescription, 2> attr;
    // offsetof 自动计算偏移
    attr[0]
        .setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setLocation(0)
        .setOffset(offsetof(Vertex, pos));
    attr[1]
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setLocation(1)
        .setOffset(offsetof(Vertex, color));
    return attr;
  }

  static auto GetBinding()
      -> vk::VertexInputBindingDescription {
    vk::VertexInputBindingDescription binding;

    binding.setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(Vertex));

    return binding;
  }
};

struct MVP {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 project;
};

} // namespace app