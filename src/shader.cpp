#include "../header/application.h"
#include "../header/shader.h"
#include "../header/math.h"
#include "glm/fwd.hpp"

namespace app {

Shader::Shader(const std::string &vertexSource,
    const std::string &fragSource) {

  vk::ShaderModuleCreateInfo createInfo1;
  createInfo1.setCodeSize(vertexSource.size())
      .setPCode(reinterpret_cast<const uint32_t *>(
          vertexSource.data()));

  vertexModule =
      Application::GetInstance().device.createShaderModule(
          createInfo1);

  vk::ShaderModuleCreateInfo createInfo2;
  createInfo2.setCodeSize(fragSource.size())
      .setPCode(reinterpret_cast<const uint32_t *>(
          fragSource.data()));
  fragmentModule =
      Application::GetInstance().device.createShaderModule(
          createInfo2);

  initDescriptorSetLayouts();
}

Shader::~Shader() {
  auto &device = Application::GetInstance().device;
  for (auto &layout : layouts) {
    device.destroyDescriptorSetLayout(layout);
  }
  device.destroyShaderModule(vertexModule);
  device.destroyShaderModule(fragmentModule);
}

void Shader::initDescriptorSetLayouts() {
  vk::DescriptorSetLayoutCreateInfo createInfo;
  std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
  bindings[0]
      .setBinding(0)
      .setDescriptorCount(1)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);
  createInfo.setBindings(bindings);

  layouts.push_back(
      Application::GetInstance()
          .device.createDescriptorSetLayout(createInfo));

  bindings.resize(1);
  bindings[0]
      .setBinding(0)
      .setDescriptorCount(1)
      .setDescriptorType(
          vk::DescriptorType::eCombinedImageSampler)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);
  createInfo.setBindings(bindings);

  layouts.push_back(
      Application::GetInstance()
          .device.createDescriptorSetLayout(createInfo));
}

auto Shader::GetPushConstantRange() const
    -> std::vector<vk::PushConstantRange> {
  std::vector<vk::PushConstantRange> ranges(2);
  ranges[0]
      .setOffset(0)
      .setSize(sizeof(glm::mat4))
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);
  ranges[1]
      .setOffset(sizeof(glm::mat4))
      .setSize(sizeof(glm::vec3))
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);
  return ranges;
}

} // namespace app