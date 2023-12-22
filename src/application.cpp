#include "../header/application.h"

namespace app {

std::unique_ptr<Application> Application::instance_ = nullptr;

void Application::Init(uint32_t w, uint32_t h) {
  if (instance_ == nullptr) {
    instance_.reset(new Application(w, h));
    instance_->initwindow();
    instance_->initVulkan();
  } else {
    std::cout << "repeat create Application" << '\n';
  }
}

void Application::Quit() { instance_.reset(); }

void Application::run() {
  // showPropInfo();
  mainLoop();
  cleanup();
}

void Application::initwindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
}

void Application::initVulkan() {
  createInstance();
  createSurface();
  pickPhysicalDevice();
  queryQueueFamilyIndices();
  createDevice();
  getGQueue();
  createSwapchain();
  createShaderModules();
  createRenderProcess();
  createRenderer();
}

void Application::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    renderer->Render();
    glfwPollEvents();
  }
  device.waitIdle();
}

void Application::cleanup() {
  renderer.reset();

  renderProcess.reset();

  Shader::Quit();

  swapchain.reset();

  device.destroy();

  vkDestroySurfaceKHR(instance, surface, nullptr);

  instance.destroy();

  glfwDestroyWindow(window);

  glfwTerminate();
}

void Application::createInstance() {

  vk::ApplicationInfo appInfo;
  appInfo.setApiVersion(VK_API_VERSION_1_3).setPEngineName("Render");

  vk::InstanceCreateInfo createInfo;
  createInfo.setPApplicationInfo(&appInfo);

  uint32_t glfwExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, nullptr);

  auto glfwextensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  createInfo.setEnabledExtensionCount(glfwExtensionCount)
      .setPpEnabledExtensionNames(glfwextensions);

  // valia layer
  createInfo.setEnabledLayerCount(1).setPEnabledLayerNames(validationLayers);

  instance = vk::createInstance(createInfo);
}

void Application::pickPhysicalDevice() {
  auto devices = instance.enumeratePhysicalDevices();
  phyDevice = devices[0];
  if (!checkDeviceExtensionSupport()) {
    std::cerr << "Do not support swapchain!!!" << '\n';
    exit(-1);
  }
}

void Application::createDevice() {
  vk::DeviceCreateInfo createInfo;

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  float proprities = 1.0;
  if (queueFamilyIndices.presentQueue.value() ==
      queueFamilyIndices.graphicQueue.value()) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setPQueuePriorities(&proprities)
        .setQueueCount(1)
        .setQueueFamilyIndex(queueFamilyIndices.graphicQueue.value());
    queueCreateInfos.push_back(queueCreateInfo);
  } else {
    vk::DeviceQueueCreateInfo queueCreateInfo1;
    queueCreateInfo1.setPQueuePriorities(&proprities)
        .setQueueCount(1)
        .setQueueFamilyIndex(queueFamilyIndices.graphicQueue.value());
    queueCreateInfos.push_back(queueCreateInfo1);
    vk::DeviceQueueCreateInfo queueCreateInfo2;
    queueCreateInfo2.setPQueuePriorities(&proprities)
        .setQueueCount(1)
        .setQueueFamilyIndex(queueFamilyIndices.presentQueue.value());
    queueCreateInfos.push_back(queueCreateInfo2);
  }
  createInfo.setPEnabledExtensionNames(deviceExtensions)
      .setQueueCreateInfos(queueCreateInfos);

  createInfo
      .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
      .setPpEnabledExtensionNames(deviceExtensions.data());
  device = phyDevice.createDevice(createInfo);
}
void Application::createSurface() {
  VkSurfaceKHR surfaceOld;
  if (glfwCreateWindowSurface(instance, window, nullptr, &surfaceOld) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = static_cast<vk::SurfaceKHR>(surfaceOld);
}

void Application::createSwapchain() {
  swapchain.reset(new Swapchain(width, height));
}

void Application::createShaderModules() {
  std::string vertexSource, fragSource;
  try {
    vertexSource =
        readSpvFile("C:/Users/Sy200/Desktop/Project/Application/spv/vert.spv");
    fragSource =
        readSpvFile("C:/Users/Sy200/Desktop/Project/Application/spv/frag.spv");
    if (vertexSource.size() == 0 || fragSource.size() == 0) {
      throw std::runtime_error("Shader file read failed!!!\n");
    } else {
      std::cout << " Read shader files Sucess!!!" << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "SPV fail : " << e.what() << '\n';
    exit(-1);
  }
  Shader::Init(vertexSource, fragSource);
}

void Application::createRenderProcess() {
  renderProcess = std::make_unique<RenderProcess>();
  renderProcess->InitRenderPass();
  renderProcess->InitLayout();
  // !!!
  swapchain->createFrameBuffers(width, height);
  renderProcess->InitPipeline(width, height);
}

void Application::createRenderer() { renderer = std::make_unique<Renderer>(); }

void Application::getGQueue() {
  graphicQueue = device.getQueue(queueFamilyIndices.graphicQueue.value(), 0);
  presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
}

// show some GPU support
void Application::showPropInfo() {
  std::cout << "Layers ---------------------\n";
  auto layers = vk::enumerateInstanceLayerProperties();
  for (auto &layer : layers) {
    std::cout << layer.layerName << '\n';
  }
  std::cout << "Extensions ---------------------\n";
  auto extensions = vk::enumerateInstanceExtensionProperties();
  for (auto &extension : extensions) {
    std::cout << extension.extensionName << '\n';
  }
}

void Application::queryQueueFamilyIndices() {
  auto properties = phyDevice.getQueueFamilyProperties();
  for (int i = 0; i < properties.size(); ++i) {
    auto propertie = properties[i];
    if (propertie.queueFlags | vk::QueueFlagBits::eGraphics) {
      queueFamilyIndices.graphicQueue = i;
    }
    if (phyDevice.getSurfaceSupportKHR(i, surface)) {
      queueFamilyIndices.presentQueue = i;
    }
    if (queueFamilyIndices)
      break;
  }
}

auto Application::checkDeviceExtensionSupport() -> bool {
  auto availableExtensions = phyDevice.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

} // namespace app