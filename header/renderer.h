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

  std::unique_ptr<BufferPkg> hostIndexsBuffer;
  std::unique_ptr<BufferPkg> deviceIndexsBuffer;

  void createFences();
  void createSemaphores();
  void createCmdBuffers();
  void createBuffers();
  void bufferData();
  void copyBuffer(vk::Buffer &src, vk::Buffer &dst,
      size_t size, size_t srcOffset, size_t dstOffset);
};

} // namespace app