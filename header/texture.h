#pragma once

#include "buffer.h"
#include "descriptorManager.h"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace app {

class TextureManager;

class Texture final {
public:
  friend class TextureManager;
  ~Texture();

  vk::Image image;
  vk::DeviceMemory memory;
  vk::ImageView view;
  DescriptorSetManager::SetInfo set;

private:
  Texture(std::string_view filename);

  Texture(void *data, uint32_t w, uint32_t h);

  void createImage(uint32_t w, uint32_t h);
  void createImageView();
  void allocMemory();
  auto queryImageMemoryIndex() -> uint32_t;
  void transitionImageLayoutFromUndefine2Dst();
  void transitionImageLayoutFromDst2Optimal();
  void transformData2Image(BufferPkg &, uint32_t w, uint32_t h);
  void updateDescriptorSet();

  void init(void *data, uint32_t w, uint32_t h);
};

class TextureManager final {
public:
  static TextureManager &Instance() {
    if (!instance_) {
      instance_ = std::make_unique<TextureManager>();
    }
    return *instance_;
  }

  auto Load(const std::string &filename) -> Texture *;

  // data must be a RGBA8888 format data
  auto Create(void *data, uint32_t w, uint32_t h) -> Texture *;
  void Destroy(Texture *);
  void Clear();

private:
  static std::unique_ptr<TextureManager> instance_;

  std::vector<std::unique_ptr<Texture>> datas_;
};

} // namespace app