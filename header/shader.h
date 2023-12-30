#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>


namespace app {

class Shader {
public:
  vk::ShaderModule vertexModule;
  vk::ShaderModule fragmentModule;
  std::vector<vk::DescriptorSetLayout> layouts_;

  Shader(const std::string &vertexSource, const std::string &fragSource);
  ~Shader();

  static void Quit();
  [[nodiscard]] auto GetPushConstantRange() const -> std::vector<vk::PushConstantRange>;

  // pipeline 指定 shader 信息
  auto GetStage() -> std::vector<vk::PipelineShaderStageCreateInfo> { return stage_; }

private:
  std::vector<vk::PipelineShaderStageCreateInfo> stage_;

  void initStage();
  void initDescriptorSetLayouts();

};

} // namespace app