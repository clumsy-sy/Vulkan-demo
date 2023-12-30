#pragma once

#include <vulkan/vulkan.hpp>

namespace app {
class RenderProcess {
public:
  vk::RenderPass renderPass;
  vk::PipelineLayout layout;
  vk::Pipeline pipeline;

  ~RenderProcess();

  void InitRenderPass();
  void InitLayout();
  void createGraphicsPipeline(uint32_t width, uint32_t height);
};

} // namespace app