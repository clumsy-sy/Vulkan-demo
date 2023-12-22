#include "../header/renderer.h"
#include "../header/Application.h"
namespace app {

Renderer::Renderer() {
  initCmdPool();
  allocCmdBuffer();
  createSems();
  createFence();
}

Renderer::~Renderer() {
  auto &device = Application::GetInstance().device;
  device.freeCommandBuffers(cmdPool, cmdBuffer);
  device.destroyCommandPool(cmdPool);
  device.destroySemaphore(imageAvaliable);
  device.destroySemaphore(imageDrawFinish);
  device.destroyFence(cmdAvaliableFence);
}

void Renderer::Render() {
  auto &device = Application::GetInstance().device;
  auto &swapchain = Application::GetInstance().swapchain;
  auto &renderProcess = Application::GetInstance().renderProcess;

  auto acqResult = device.acquireNextImageKHR(
      swapchain->swapchain, std::numeric_limits<uint64_t>::max(),
      imageAvaliable);

  if (acqResult.result != vk::Result::eSuccess) {
    throw std::runtime_error("device.acquireNextImageKHR failed!!!");
  }
  auto imageIndex = acqResult.value;

  cmdBuffer.reset();

  // begin
  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  cmdBuffer.begin(beginInfo);
  {
    vk::RenderPassBeginInfo renderPassBegin;
    vk::Rect2D area;
    area.setOffset({0, 0}).setExtent(swapchain->info.imageExtent);
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f));
    renderPassBegin.setRenderPass(renderProcess->renderPass)
        .setRenderArea(area)
        .setFramebuffer(swapchain->framebuffers[imageIndex])
        .setClearValues(clearValue);

    cmdBuffer.beginRenderPass(renderPassBegin, {});
    {
      cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             renderProcess->pipeline);
      // vertex count, prim, first idx,
      cmdBuffer.draw(3, 1, 0, 0);
    }
    cmdBuffer.endRenderPass();
  }
  cmdBuffer.end();

  vk::PipelineStageFlags waitDstStageMask =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;

  vk::SubmitInfo submit;
  submit.setWaitDstStageMask(waitDstStageMask)
      .setWaitSemaphores(imageAvaliable)
      .setSignalSemaphores(imageDrawFinish)
      .setCommandBuffers(cmdBuffer);

  Application::GetInstance().graphicQueue.submit(submit, cmdAvaliableFence);

  vk::PresentInfoKHR present;
  present.setWaitSemaphores(imageDrawFinish)
      .setImageIndices(imageIndex)
      .setSwapchains(swapchain->swapchain);

  auto result = Application::GetInstance().presentQueue.presentKHR(present);

  if (result != vk::Result::eSuccess) {
    throw std::runtime_error("image present failed!!!");
  }
  // CPU GPU 同步
  auto waitResult = device.waitForFences(cmdAvaliableFence, true,
                                         std::numeric_limits<uint64_t>::max());
  if (waitResult != vk::Result::eSuccess) {
    throw std::runtime_error("wait for fence failed!!!");
  }
  device.resetFences(cmdAvaliableFence);
}

void Renderer::initCmdPool() {
  vk::CommandPoolCreateInfo createInfo;
  createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

  cmdPool = Application::GetInstance().device.createCommandPool(createInfo);
}

void Renderer::allocCmdBuffer() {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setCommandPool(cmdPool).setCommandBufferCount(1).setLevel(
      vk::CommandBufferLevel::ePrimary);

  cmdBuffer =
      Application::GetInstance().device.allocateCommandBuffers(allocInfo)[0];
}

void Renderer::createSems() {
  vk::SemaphoreCreateInfo createInfo;

  imageAvaliable =
      Application::GetInstance().device.createSemaphore(createInfo);
  imageDrawFinish =
      Application::GetInstance().device.createSemaphore(createInfo);
}

void Renderer::createFence() {
  vk::FenceCreateInfo createInfo;
  cmdAvaliableFence = Application::GetInstance().device.createFence(createInfo);
}
} // namespace app
