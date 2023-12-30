#pragma once

#include <vulkan/vulkan.hpp>
#include "buffer.h"

namespace app {

class Renderer {
public:
  Renderer(int maxFlightCount = 2);
  ~Renderer();

  void Render();

private:
  int maxFlightCount;
  int curFrame;
  std::vector<vk::Fence> fences;
  std::vector<vk::Semaphore> imageAvaliableSems;
  std::vector<vk::Semaphore> renderFinishSems;
  std::vector<vk::CommandBuffer> cmdBufs;

  std::unique_ptr<BufferPkg> hostVertexBuffer;
  std::unique_ptr<BufferPkg> deviceVertexBuffer;

  void createFences();
  void createSemaphores();
  void createCmdBuffers();
  void createVertexBuffer();
  void bufferVertexData();

  // vk::CommandPool cmdPool;
  // vk::CommandBuffer cmdBuffer;

  // vk::Semaphore imageAvaliable;
  // vk::Semaphore imageDrawFinish;
  // vk::Fence cmdAvaliableFence;

  // std::unique_ptr<Buffer> hostVertexBuffer_;
  // std::unique_ptr<Buffer> deviceVertexBuffer_;

  // void initCmdPool();
  // void allocCmdBuffer();
  // void createSems();
  // void createFence();
};

} // namespace app