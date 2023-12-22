#include <memory>

#include "../header/application.h"
#include "../header/shader.h"


namespace app {
std::unique_ptr<Shader> Shader::instance_ = nullptr;

void Shader::Init(const std::string &vertexSource,
                  const std::string &fragSource) {
  if (instance_ == nullptr) {
    instance_ = std::make_unique<Shader>(vertexSource, fragSource);
  } else {
    std::cout << "repeat create shader" << '\n';
  }
}

void Shader::Quit() { instance_.reset(); }

Shader::Shader(const std::string &vertexSource, const std::string &fragSource) {
  // std::cout << "vertexSource : " << vertexSource << std::endl;
  // std::cout << "fragSource : " << fragSource << std::endl;
  // std::cout << "devcie : " << Application::GetInstance().device << std::endl;
  vk::ShaderModuleCreateInfo createInfo1;
  createInfo1.setCodeSize(vertexSource.size())
      .setPCode(reinterpret_cast<const uint32_t *>(vertexSource.data()));

  vertexModule =
      Application::GetInstance().device.createShaderModule(createInfo1);

  vk::ShaderModuleCreateInfo createInfo2;
  createInfo2.setCodeSize(fragSource.size())
      .setPCode(reinterpret_cast<const uint32_t *>(fragSource.data()));
  fragmentModule =
      Application::GetInstance().device.createShaderModule(createInfo2);

  initStage();
}

Shader::~Shader() {
  auto &device = Application::GetInstance().device;
  device.destroyShaderModule(vertexModule);
  device.destroyShaderModule(fragmentModule);
}

void Shader::initStage() {
  // shader 数量
  stage_.resize(2);
  stage_[0]
      .setStage(vk::ShaderStageFlagBits::eVertex)
      .setModule(vertexModule)
      .setPName("main");
  stage_[1]
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setModule(fragmentModule)
      .setPName("main");
}

} // namespace app