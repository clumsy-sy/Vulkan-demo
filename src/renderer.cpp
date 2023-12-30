#include <memory>

#include <memory>

#include "../header/renderer.h"
#include "../header/Application.h"
#include "../header/vertex.h"
#include "vulkan/vulkan_enums.hpp"
namespace app {

Renderer::Renderer(int maxFlightCount)
    : maxFlightCount(maxFlightCount), curFrame(0) {
  createFences();
  createSemaphores();
  createCmdBuffers();
  createVertexBuffer();
  bufferVertexData();
}

Renderer::~Renderer() {
  hostVertexBuffer.reset();
  deviceVertexBuffer.reset();
  auto &device = Application::GetInstance().device;
  for (auto &sem : imageAvaliableSems) {
    device.destroySemaphore(sem);
  }
  for (auto &sem : renderFinishSems) {
    device.destroySemaphore(sem);
  }
  for (auto &fence : fences) {
    device.destroyFence(fence);
  }
}

void Renderer::Render() {
    std::cout << "Start Render !\n";
  auto &device = Application::GetInstance().device;
  auto &swapchain = Application::GetInstance().swapchain;
  auto &renderProcess =
      Application::GetInstance().renderProcess;
  auto &cmdMgr = Application::GetInstance().commandManager;

  // 等待第一个 fence
  if (device.waitForFences(fences[curFrame], true,
          std::numeric_limits<std::uint64_t>::max()) !=
      vk::Result::eSuccess) {
    throw std::runtime_error("wait for fence failed");
  }
  device.resetFences(fences[curFrame]);

  auto acqResult =
      device.acquireNextImageKHR(swapchain->swapchain,
          std::numeric_limits<uint64_t>::max(),
          imageAvaliableSems[curFrame]);

  if (acqResult.result != vk::Result::eSuccess) {
    throw std::runtime_error(
        "device.acquireNextImageKHR failed!!!");
  }
  // 获得需要写入的图像的下标
  auto imageIndex = acqResult.value;

  cmdBufs[curFrame].reset();

  // begin
  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  cmdBufs[curFrame].begin(beginInfo);
  {
    vk::RenderPassBeginInfo renderPassBegin;
    vk::Rect2D area;
    area.setOffset({0, 0}).setExtent(
        swapchain->info.imageExtent);
    vk::ClearValue clearValue;
    clearValue.setColor(
        vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f));
    renderPassBegin.setRenderPass(renderProcess->renderPass)
        .setRenderArea(area)
        .setFramebuffer(swapchain->framebuffers[imageIndex])
        .setClearValues(clearValue);

    cmdBufs[curFrame].beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
    {
      cmdBufs[curFrame].bindPipeline(
          vk::PipelineBindPoint::eGraphics,
          renderProcess->pipeline);
      // vertex count, prim, first idx,
      vk::DeviceSize offset = 0;
      cmdBufs[curFrame].bindVertexBuffers(
          0, deviceVertexBuffer->buffer, offset);
      cmdBufs[curFrame].draw(deviceVertexBuffer->size, 1, 0, 0);
    }
    cmdBufs[curFrame].endRenderPass();
  }
  cmdBufs[curFrame].end();

  vk::PipelineStageFlags waitDstStageMask =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;

  vk::SubmitInfo submit;
  submit.setWaitDstStageMask(waitDstStageMask)
      .setWaitSemaphores(imageAvaliableSems[curFrame])
      .setSignalSemaphores(renderFinishSems[curFrame])
      .setCommandBuffers(cmdBufs[curFrame]);

  Application::GetInstance().graphicQueue.submit(
      submit, fences[curFrame]);

  vk::PresentInfoKHR present;
  present.setWaitSemaphores(renderFinishSems[curFrame])
      .setImageIndices(imageIndex)
      .setSwapchains(swapchain->swapchain);

  auto result =
      Application::GetInstance().presentQueue.presentKHR(
          present);

  if (result != vk::Result::eSuccess) {
    throw std::runtime_error("image present failed!!!");
  }

  curFrame = (curFrame + 1) % maxFlightCount;

  // CPU GPU 同步
  auto waitResult = device.waitForFences(fences[curFrame],
      true, std::numeric_limits<uint64_t>::max());
  if (waitResult != vk::Result::eSuccess) {
    throw std::runtime_error("wait for fence failed!!!");
  }
  device.resetFences(fences[curFrame]);
}
void Renderer::createFences() {
    fences.resize(maxFlightCount, nullptr);

    for (auto& fence : fences) {
        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        fence = Application::GetInstance().device.createFence(fenceCreateInfo);
    }
}

void Renderer::createSemaphores() {
    auto& device = Application::GetInstance().device;
    vk::SemaphoreCreateInfo info;

    imageAvaliableSems.resize(maxFlightCount);
    renderFinishSems.resize(maxFlightCount);

    for (auto& sem : imageAvaliableSems) {
        sem = device.createSemaphore(info);
    }

    for (auto& sem : renderFinishSems) {
        sem = device.createSemaphore(info);
    }
}

void Renderer::createCmdBuffers() {
    cmdBufs.resize(maxFlightCount);
    
    for (auto& cmd : cmdBufs) {
        cmd = Application::GetInstance().commandManager->CreateOneCommandBuffer();
    }
}

void Renderer::createVertexBuffer() {
    hostVertexBuffer = std::make_unique<BufferPkg>(sizeof(vertices[0]) * vertices.size(),
                                       vk::BufferUsageFlagBits::eTransferSrc,
                                       vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    deviceVertexBuffer = std::make_unique<BufferPkg>(sizeof(vertices[0]) * vertices.size(),
                                         vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,
                                         vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void Renderer::bufferVertexData() {
    memcpy(hostVertexBuffer->map, vertices.data(), sizeof(vertices[0]) * vertices.size());

    auto cmdBuf = Application::GetInstance().commandManager->CreateOneCommandBuffer();
    vk::CommandBufferBeginInfo begin;
    begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf.begin(begin); {
        vk::BufferCopy region;
        region.setSize(hostVertexBuffer->size)
              .setSrcOffset(0)
              .setDstOffset(0);
        cmdBuf.copyBuffer(hostVertexBuffer->buffer, deviceVertexBuffer->buffer, region);
    } cmdBuf.end();
    
    vk::SubmitInfo submit;
    submit.setCommandBuffers(cmdBuf);
    Application::GetInstance().graphicQueue.submit(submit);

    Application::GetInstance().device.waitIdle();

    Application::GetInstance().commandManager->FreeCmd(cmdBuf);
}

} // namespace app
