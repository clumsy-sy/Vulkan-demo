#pragma once

#include "vulkan/vulkan.hpp"
#include <memory>
#include <vector>

namespace app {

class DescriptorSetManager final {
public:
  struct SetInfo {
    vk::DescriptorSet set;
    vk::DescriptorPool pool;
  };

  static void Init(uint32_t maxFlight) {
    instance_ = std::make_unique<DescriptorSetManager>(maxFlight);
  }

  static void Quit() { instance_.reset(); }

  static DescriptorSetManager &Instance() { return *instance_; }

  DescriptorSetManager(uint32_t maxFlight);
  ~DescriptorSetManager();

  auto AllocBufferSets(uint32_t num) -> std::vector<SetInfo>;
  auto AllocImageSet() -> SetInfo;

  void FreeImageSet(const SetInfo &);

private:
  struct PoolInfo {
    vk::DescriptorPool pool_;
    uint32_t remainNum_;
  };

  PoolInfo bufferSetPool_;

  std::vector<PoolInfo> fulledImageSetPool_;
  std::vector<PoolInfo> avalibleImageSetPool_;

  void addImageSetPool();
  auto getAvaliableImagePoolInfo() -> PoolInfo &;

  uint32_t maxFlight_;

  static std::unique_ptr<DescriptorSetManager> instance_;
};

} // namespace app