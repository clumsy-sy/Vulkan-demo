#pragma once

#include <vulkan/vulkan.hpp>

namespace app {

class Renderer {
public:
  Renderer();
  ~Renderer();

  void Render();

private:
  vk::CommandPool cmdPool;
  vk::CommandBuffer cmdBuffer;

  vk::Semaphore imageAvaliable;
  vk::Semaphore imageDrawFinish;
  vk::Fence cmdAvaliableFence;

  void initCmdPool();
  void allocCmdBuffer();
  void createSems();
  void createFence();
};

} // namespace app