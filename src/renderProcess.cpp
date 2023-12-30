#include "../header/renderProcess.h"
#include "../header/application.h"
#include "../header/vertex.h"


namespace app {

void RenderProcess::createGraphicsPipeline(uint32_t width, uint32_t height) {
  vk::GraphicsPipelineCreateInfo createInfo;

  

  // 1. Vertex input
  // vk::PipelineVertexInputStateCreateInfo inputState;
  // createInfo.setPVertexInputState(&inputState);

  vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    auto attribute = Vertex::GetAttribute();
    auto binding = Vertex::GetBinding();

    // vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    // vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size());
    // vertexInputCreateInfo.pVertexBindingDescriptions = &binding;
    // vertexInputCreateInfo.pVertexAttributeDescriptions = attribute.data();
    vertexInputCreateInfo.setVertexBindingDescriptions(binding)
                         .setVertexAttributeDescriptions(attribute);
    createInfo.setPVertexInputState(&vertexInputCreateInfo);

  // 2. Vertex Assembly
  vk::PipelineInputAssemblyStateCreateInfo inputAss;
  inputAss.setPrimitiveRestartEnable(false).setTopology(
      vk::PrimitiveTopology::eTriangleList);

  createInfo.setPInputAssemblyState(&inputAss);
  // 3. shader
  auto stages = Application::GetInstance().shader->GetStage();
  createInfo.setStages(stages);

  // 4. viewport
  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::Viewport viewport(0, 0, static_cast<float>(width),
                        static_cast<float>(height), 0, 1);
  vk::Rect2D rect({0, 0}, {width, height});
  viewportInfo.setViewports(viewport).setScissors(rect);
  createInfo.setPViewportState(&viewportInfo);

  // 5. Rastrization
  vk::PipelineRasterizationStateCreateInfo rastInfo;
  rastInfo.setRasterizerDiscardEnable(false)
      .setCullMode(vk::CullModeFlagBits::eBack)
      .setFrontFace(vk::FrontFace::eClockwise)
      .setPolygonMode(vk::PolygonMode::eFill)
      .setLineWidth(1);
  createInfo.setPRasterizationState(&rastInfo);

  // 6 .multi sample
  vk::PipelineMultisampleStateCreateInfo multiSample;
  multiSample.setSampleShadingEnable(false).setRasterizationSamples(
      vk::SampleCountFlagBits::e1);
  createInfo.setPMultisampleState(&multiSample);

  // 7. depth test

  // 8. color blending

  vk::PipelineColorBlendStateCreateInfo blendInfo;
  vk::PipelineColorBlendAttachmentState attachs;
  attachs.setBlendEnable(false).setColorWriteMask(
      vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB |
      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);

  blendInfo.setLogicOpEnable(false).setAttachments(attachs);

  createInfo.setPColorBlendState(&blendInfo);

  // 9 renderpass and layout
  createInfo.setRenderPass(renderPass).setLayout(layout);

  // 创建
  auto result = Application::GetInstance().device.createGraphicsPipeline(
      nullptr, createInfo);
  if (result.result != vk::Result::eSuccess) {
    throw std::runtime_error("create graphic pipeline failed");
  }
  pipeline = result.value;
}

void RenderProcess::InitLayout() {
  vk::PipelineLayoutCreateInfo createInfo;
  layout = Application::GetInstance().device.createPipelineLayout(createInfo);
}

void RenderProcess::InitRenderPass() {
  vk::RenderPassCreateInfo createInfo;
  vk::AttachmentDescription AttachDescription;

  AttachDescription
      .setFormat(Application::GetInstance().swapchain->info.format.format)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setSamples(vk::SampleCountFlagBits::e1);

  createInfo.setAttachments(AttachDescription);

  vk::AttachmentReference reference;
  reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
      .setAttachment(0);

  vk::SubpassDescription subpass;
  subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
      .setColorAttachments(reference);

  createInfo.setSubpasses(subpass);

  vk::SubpassDependency dependency;
  dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
      .setDstSubpass(0)
      .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
      .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
      .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  createInfo.setDependencies(dependency);

  renderPass = Application::GetInstance().device.createRenderPass(createInfo);
}

RenderProcess::~RenderProcess() {
  auto device = &Application::GetInstance().device;
  device->destroyRenderPass(renderPass);
  device->destroyPipelineLayout(layout);
  device->destroyPipeline(pipeline);
}

} // namespace app
