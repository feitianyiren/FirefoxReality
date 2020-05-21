#pragma once

#include "vrb/Forward.h"
#include "Device.h"
#include <memory>
#include <vector>

#include <EGL/egl.h>
#include "jni.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

namespace crow {

class OpenXRSwapChain;
typedef std::shared_ptr<OpenXRSwapChain> OpenXRSwapChainPtr;

class OpenXRSwapChain {
private:
  vrb::RenderContextPtr context;
  XrSwapchainCreateInfo info;
  device::RenderMode renderMode;
  XrSwapchain swapchain = XR_NULL_HANDLE;
  std::vector<XrSwapchainImageOpenGLESKHR> imageBuffer;
  std::vector<XrSwapchainImageBaseHeader*> images;
  std::vector<vrb::FBOPtr> fbos;
  vrb::FBOPtr acquiredFBO;
public:
  static OpenXRSwapChainPtr create();
  void Init(vrb::RenderContextPtr &aContext, XrSession aSession, const XrSwapchainCreateInfo& aInfo, device::RenderMode aRenderMode);
  void AcquireImage();
  void ReleaseImage();
  void BindFBO();
  void Destroy();
  inline XrSwapchain SwapChain() const { return swapchain;}
  inline int32_t Width() const { return info.width; }
  inline int32_t Height() const { return info.height; }
};

}
