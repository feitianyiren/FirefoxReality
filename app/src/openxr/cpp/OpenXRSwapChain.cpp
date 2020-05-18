#include "OpenXRSwapChain.h"
#include "OpenXRHelpers.h"
#include "vrb/FBO.h"
#include "vrb/GLError.h"
#include "vrb/Logger.h"

namespace crow {

OpenXRSwapChainPtr
OpenXRSwapChain::create() {
  return std::make_shared<OpenXRSwapChain>();
}

void
OpenXRSwapChain::Init(vrb::RenderContextPtr &aContext, XrSession aSession, const XrSwapchainCreateInfo& aInfo, device::RenderMode aRenderMode) {
  Destroy();
  info = aInfo;
  context = aContext;
  renderMode = aRenderMode;

  CHECK_XRCMD(xrCreateSwapchain(aSession, &info, &swapchain));
  CHECK(swapchain != XR_NULL_HANDLE);

  uint32_t imageCount;
  CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr));
  CHECK(imageCount > 0);
  imageBuffer.resize(imageCount);
  fbos.resize(imageCount);
  for (XrSwapchainImageOpenGLESKHR& image : imageBuffer) {
    image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
    images.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
  }
  CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, images[0]));
}

void
OpenXRSwapChain::AcquireImage() {
  CHECK_MSG(!acquiredFBO, "Expected no acquired FBOs. ReleaseImage not called?");

  XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
  uint32_t swapchainImageIndex = 0;
  CHECK_XRCMD(xrAcquireSwapchainImage(swapchain, &acquireInfo, &swapchainImageIndex));
  CHECK(swapchainImageIndex < imageBuffer.size());
  CHECK(swapchainImageIndex < fbos.size());

  XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
  waitInfo.timeout = XR_INFINITE_DURATION;
  CHECK_XRCMD(xrWaitSwapchainImage(swapchain, &waitInfo));

  if (!fbos[swapchainImageIndex]) {
    vrb::FBOPtr fbo = vrb::FBO::Create(context);
    fbos[swapchainImageIndex] = fbo;
    uint32_t texture = imageBuffer[swapchainImageIndex].image;
    VRB_GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    VRB_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    VRB_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    VRB_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    VRB_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    vrb::FBO::Attributes attributes;
    if (renderMode == device::RenderMode::Immersive) {
      attributes.samples = 0;
    } else {
      attributes.samples = 4;
    }

    VRB_GL_CHECK(fbo->SetTextureHandle(texture, info.width, info.height, attributes));
    if (!fbo->IsValid()) {
      VRB_ERROR("OpenXR XrSwapchainImageOpenGLESKHR texture FBO is not valid");
    } else{
      VRB_DEBUG("OpenXR succesfully created FBO for swapChainImageIndex: %d", swapchainImageIndex);
    }
  }

  acquiredFBO = fbos[swapchainImageIndex];
}

void
OpenXRSwapChain::ReleaseImage() {
  CHECK_MSG(acquiredFBO, "Expected a valid acquired FBO. AcquireImage not called?");

  XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
  CHECK_XRCMD(xrReleaseSwapchainImage(swapchain, &releaseInfo));
  acquiredFBO = nullptr;
}

void
OpenXRSwapChain::BindFBO() {
  CHECK_MSG(acquiredFBO, "Expected a valid acquired FBO. AcquireImage not called?");
  acquiredFBO->Bind();
}

void
OpenXRSwapChain::Destroy() {
  if (acquiredFBO) {
    ReleaseImage();
  }
  fbos.clear();
  imageBuffer.clear();
  images.clear();
  if (swapchain != XR_NULL_HANDLE) {
    xrDestroySwapchain(swapchain);
    swapchain = XR_NULL_HANDLE;
  }
}

}
