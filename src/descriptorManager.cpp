#include "../header/descriptorManager.h"
#include "../header/application.h"

namespace app {

std::unique_ptr<DescriptorSetManager> DescriptorSetManager::instance_ = nullptr;

DescriptorSetManager::DescriptorSetManager(uint32_t maxFlight)
    : maxFlight_(maxFlight) {
  vk::DescriptorPoolSize size;
  size.setType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(2 * maxFlight);
  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setMaxSets(maxFlight).setPoolSizes(size);
  auto pool =
      Application::GetInstance().device.createDescriptorPool(createInfo);
  bufferSetPool_.pool_ = pool;
  bufferSetPool_.remainNum_ = maxFlight;
}

DescriptorSetManager::~DescriptorSetManager() {
  auto &device = Application::GetInstance().device;

  device.destroyDescriptorPool(bufferSetPool_.pool_);
  for (auto pool : fulledImageSetPool_) {
    device.destroyDescriptorPool(pool.pool_);
  }
  for (auto pool : avalibleImageSetPool_) {
    device.destroyDescriptorPool(pool.pool_);
  }
}

void DescriptorSetManager::addImageSetPool() {
  constexpr uint32_t MaxSetNum = 10;

  vk::DescriptorPoolSize size;
  size.setType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(MaxSetNum);
  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setMaxSets(MaxSetNum).setPoolSizes(size).setFlags(
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
  auto pool =
      Application::GetInstance().device.createDescriptorPool(createInfo);
  avalibleImageSetPool_.push_back({pool, MaxSetNum});
}

auto
DescriptorSetManager::AllocBufferSets(uint32_t num) -> std::vector<DescriptorSetManager::SetInfo> {
  std::vector<vk::DescriptorSetLayout> layouts(
      maxFlight_,
      Application::GetInstance().shader->layouts_[0]);
  vk::DescriptorSetAllocateInfo allocInfo;
  allocInfo.setDescriptorPool(bufferSetPool_.pool_)
      .setDescriptorSetCount(num)
      .setSetLayouts(layouts);
  auto sets =
      Application::GetInstance().device.allocateDescriptorSets(allocInfo);

  std::vector<SetInfo> result(num);

  for (int i = 0; i < num; i++) {
    result[i].set = sets[i];
    result[i].pool = bufferSetPool_.pool_;
  }

  return result;
}

auto DescriptorSetManager::AllocImageSet() -> DescriptorSetManager::SetInfo {
  std::vector<vk::DescriptorSetLayout> layouts{
      Application::GetInstance().shader->layouts_[1]};
  vk::DescriptorSetAllocateInfo allocInfo;
  auto &poolInfo = getAvaliableImagePoolInfo();
  allocInfo.setDescriptorPool(poolInfo.pool_)
      .setDescriptorSetCount(1)
      .setSetLayouts(layouts);
  auto sets =
      Application::GetInstance().device.allocateDescriptorSets(allocInfo);

  SetInfo result;
  result.pool = poolInfo.pool_;
  result.set = sets[0];

  poolInfo.remainNum_ =
      std::max<int>(static_cast<int>(poolInfo.remainNum_) - sets.size(), 0);
  if (poolInfo.remainNum_ == 0) {
    fulledImageSetPool_.push_back(poolInfo);
    avalibleImageSetPool_.pop_back();
  }

  return result;
}

void DescriptorSetManager::FreeImageSet(const SetInfo &info) {
  auto it = std::find_if(
      fulledImageSetPool_.begin(), fulledImageSetPool_.end(),
      [&](const PoolInfo &poolInfo) { return poolInfo.pool_ == info.pool; });
  if (it != fulledImageSetPool_.end()) {
    it->remainNum_++;
    avalibleImageSetPool_.push_back(*it);
    fulledImageSetPool_.erase(it);
    return;
  }

  it = std::find_if(
      avalibleImageSetPool_.begin(), avalibleImageSetPool_.end(),
      [&](const PoolInfo &poolInfo) { return poolInfo.pool_ == info.pool; });
  if (it != avalibleImageSetPool_.end()) {
    it->remainNum_++;
    return;
  }
}

DescriptorSetManager::PoolInfo &
DescriptorSetManager::getAvaliableImagePoolInfo() {
  if (avalibleImageSetPool_.empty()) {
    addImageSetPool();
    return avalibleImageSetPool_.back();
  }
  return avalibleImageSetPool_.back();
}

} // namespace app
