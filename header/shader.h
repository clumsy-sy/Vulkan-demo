#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>


namespace app {

class Shader {
public:
  vk::ShaderModule vertexModule;
  vk::ShaderModule fragmentModule;

  Shader(const std::string &vertexSource, const std::string &fragSource);
  ~Shader();

  static void Init(const std::string &vertexSource,
                   const std::string &fragSource);
  static void Quit();

  static auto GetInstance() -> Shader & { return *instance_; }
  // 禁止复制构造函数和赋值运算符
  Shader(const Shader &) = delete;
  auto operator=(const Shader &) -> Shader & = delete;

  // pipeline 指定 shader 信息
  auto GetStage() -> std::vector<vk::PipelineShaderStageCreateInfo> { return stage_; }

private:
  static std::unique_ptr<Shader> instance_;
  std::vector<vk::PipelineShaderStageCreateInfo> stage_;

  void initStage();

  // 私有构造函数，防止外部实例化
  Shader() = default;

};

} // namespace app