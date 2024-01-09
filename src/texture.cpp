#include "../header/texture.h"
#include "../header/application.h"
#include "vulkan/vulkan_handles.hpp"
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

namespace app {

Texture::Texture(
    std::string_view filename, vk::Sampler sampler) {
  int w, h, channel;
  stbi_uc *pixels = stbi_load(
      filename.data(), &w, &h, &channel, STBI_rgb_alpha);
  size_t size = w * h * 4;

  if (!pixels) {
    std::cerr << "image load failed\n";
    throw std::runtime_error("image load failed");
    exit(EXIT_FAILURE);
  }
  init(pixels, w, h, sampler);

  stbi_image_free(pixels);
}

Texture::Texture(void *data, unsigned int w, unsigned int h,
    vk::Sampler sampler) {
  init(data, w, h, sampler);
}

void Texture::init(void *data, uint32_t w, uint32_t h,
    vk::Sampler sampler) {
  const uint32_t size = w * h * 4;
  std::unique_ptr<BufferPkg> buffer(new BufferPkg(size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostCoherent |
          vk::MemoryPropertyFlagBits::eHostVisible));
  memcpy(buffer->map, data, size);

  createImage(w, h);
  allocMemory();
  Application::GetInstance().device.bindImageMemory(
      image, memory, 0);

  transitionImageLayoutFromUndefine2Dst();
  transformData2Image(*buffer, w, h);
  transitionImageLayoutFromDst2Optimal();

  createImageView();
  // set = DescriptorSetManager::Instance().AllocImageSet();
  // updateDescriptorSet(sampler);
}

Texture::~Texture() {
  auto &device = Application::GetInstance().device;
  DescriptorSetManager::Instance().FreeImageSet(set);
  device.destroyImageView(view);
  device.freeMemory(memory);
  device.destroyImage(image);
}

void Texture::createImage(uint32_t w, uint32_t h) {
  vk::ImageCreateInfo createInfo;
  createInfo.setImageType(vk::ImageType::e2D)
      .setArrayLayers(1)
      .setMipLevels(1)
      .setExtent({w, h, 1})
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setUsage(vk::ImageUsageFlagBits::eTransferDst |
                vk::ImageUsageFlagBits::eSampled)
      .setSamples(vk::SampleCountFlagBits::e1);
  image = Application::GetInstance().device.createImage(
      createInfo);
}

void Texture::allocMemory() {
  auto &device = Application::GetInstance().device;
  vk::MemoryAllocateInfo allocInfo;

  auto requirements =
      device.getImageMemoryRequirements(image);
  allocInfo.setAllocationSize(requirements.size);

  auto index =
      QueryBufferMemTypeIndex(requirements.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal);
  allocInfo.setMemoryTypeIndex(index);

  memory = device.allocateMemory(allocInfo);
}

void Texture::transformData2Image(
    BufferPkg &buffer, uint32_t w, uint32_t h) {
  Application::GetInstance().commandManager->ExecuteCmd(
      Application::GetInstance().graphicQueue,
      [&](vk::CommandBuffer cmdBuf) {
        vk::BufferImageCopy region;
        vk::ImageSubresourceLayers subsource;
        subsource
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseArrayLayer(0)
            .setMipLevel(0)
            .setLayerCount(1);
        region.setBufferImageHeight(0)
            .setBufferOffset(0)
            .setImageOffset(0)
            .setImageExtent({w, h, 1})
            .setBufferRowLength(0)
            .setImageSubresource(subsource);
        cmdBuf.copyBufferToImage(buffer.buffer, image,
            vk::ImageLayout::eTransferDstOptimal, region);
      });
}

void Texture::transitionImageLayoutFromUndefine2Dst() {
  Application::GetInstance().commandManager->ExecuteCmd(
      Application::GetInstance().graphicQueue,
      [&](vk::CommandBuffer cmdBuf) {
        vk::ImageMemoryBarrier barrier;
        vk::ImageSubresourceRange range;
        range.setLayerCount(1)
            .setBaseArrayLayer(0)
            .setLevelCount(1)
            .setBaseMipLevel(0)
            .setAspectMask(vk::ImageAspectFlagBits::eColor);
        barrier.setImage(image)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(
                vk::ImageLayout::eTransferDstOptimal)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstAccessMask(
                (vk::AccessFlagBits::eTransferWrite))
            .setSubresourceRange(range);
        cmdBuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer, {}, {},
            nullptr, barrier);
      });
}

void Texture::transitionImageLayoutFromDst2Optimal() {
  Application::GetInstance().commandManager->ExecuteCmd(
      Application::GetInstance().graphicQueue,
      [&](vk::CommandBuffer cmdBuf) {
        vk::ImageMemoryBarrier barrier;
        vk::ImageSubresourceRange range;
        range.setLayerCount(1)
            .setBaseArrayLayer(0)
            .setLevelCount(1)
            .setBaseMipLevel(0)
            .setAspectMask(vk::ImageAspectFlagBits::eColor);
        barrier.setImage(image)
            .setOldLayout(
                vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(
                vk::ImageLayout::eShaderReadOnlyOptimal)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSrcAccessMask(
                (vk::AccessFlagBits::eTransferWrite))
            .setDstAccessMask(
                (vk::AccessFlagBits::eShaderRead))
            .setSubresourceRange(range);
        cmdBuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader, {},
            {}, nullptr, barrier);
      });
}

void Texture::createImageView() {
  vk::ImageViewCreateInfo createInfo;
  vk::ComponentMapping mapping;
  vk::ImageSubresourceRange range;
  range.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseArrayLayer(0)
      .setLayerCount(1)
      .setLevelCount(1)
      .setBaseMipLevel(0);
  createInfo.setImage(image)
      .setViewType(vk::ImageViewType::e2D)
      .setComponents(mapping)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setSubresourceRange(range);
  view = Application::GetInstance().device.createImageView(
      createInfo);
}

void Texture::updateDescriptorSet(vk::Sampler sampler) {
  vk::WriteDescriptorSet writer;
  vk::DescriptorImageInfo imageInfo;
  imageInfo
      .setImageLayout(
          vk::ImageLayout::eShaderReadOnlyOptimal)
      .setImageView(view)
      .setSampler(sampler);
  writer.setImageInfo(imageInfo)
      .setDstBinding(1)
      .setDstArrayElement(0)
      .setDstSet(set.set)
      .setDescriptorCount(1)
      .setDescriptorType(
          vk::DescriptorType::eCombinedImageSampler);
  Application::GetInstance().device.updateDescriptorSets(
      writer, {});
}

// std::unique_ptr<TextureManager> TextureManager::instance_
// = nullptr;

// Texture *TextureManager::Load(const std::string
// &filename) {
//   datas_.push_back(std::unique_ptr<Texture>(new
//   Texture(filename))); return datas_.back().get();
// }

// Texture *TextureManager::Create(void *data, uint32_t w,
// uint32_t h) {
//   datas_.push_back(std::unique_ptr<Texture>(new
//   Texture(data, w, h))); return datas_.back().get();
// }

// void TextureManager::Clear() {
//   datas_.clear();
// }

// void TextureManager::Destroy(Texture *texture) {
//   auto it = std::find_if(datas_.begin(), datas_.end(),
//   [&](const std::unique_ptr<Texture> &t) {
//     return t.get() == texture;
//   });
//   if (it != datas_.end()) {
//     Application::GetInstance().device.waitIdle();
//     datas_.erase(it);
//     return;
//   }
// }

} // namespace app