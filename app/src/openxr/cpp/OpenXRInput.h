#pragma once

#include "vrb/Forward.h"
#include <EGL/egl.h>
#include "jni.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <array>

#include "ControllerDelegate.h"
#include "ElbowModel.h"

namespace crow {

class OpenXRInput {
public:
  enum Hand {
    Left = 0,
    Right = 1,
    Count = 2
  };
  struct ControllerState {
    bool enabled = false;
    bool created = false;
    XrSpace space = XR_NULL_HANDLE;
  };
  XrActionSet actionSet = XR_NULL_HANDLE;
  XrAction actionPose = XR_NULL_HANDLE;
  XrAction actionMenuClick = XR_NULL_HANDLE;
  XrAction actionTriggerClick = XR_NULL_HANDLE;
  XrAction actionTriggerTouch = XR_NULL_HANDLE;
  XrAction actionTriggerValue = XR_NULL_HANDLE;
  XrAction actionSqueezeClick = XR_NULL_HANDLE;
  XrAction actionSqueezeValue = XR_NULL_HANDLE;
  XrAction actionTrackpadClick = XR_NULL_HANDLE;
  XrAction actionTrackpadTouch = XR_NULL_HANDLE;
  XrAction actionTrackpadX = XR_NULL_HANDLE;
  XrAction actionTrackpadY = XR_NULL_HANDLE;
  XrAction actionThumbstickClick = XR_NULL_HANDLE;
  XrAction actionThumbstickTouch = XR_NULL_HANDLE;
  XrAction actionThumbstickX = XR_NULL_HANDLE;
  XrAction actionThumbstickY = XR_NULL_HANDLE;
  XrAction actionButtonAClick = XR_NULL_HANDLE;
  XrAction actionButtonBClick = XR_NULL_HANDLE;
  XrAction actionButtonATouch = XR_NULL_HANDLE;
  XrAction actionButtonBTouch = XR_NULL_HANDLE;

  std::array<XrPath, Hand::Count> handSubactionPath;
  std::array<ControllerState, Hand::Count> controllerState;

  void Initialize(XrInstance instance, XrSession session);
  void Update(XrSession session, XrTime predictedDisplayTime, XrSpace baseSpace, device::RenderMode renderMode, ControllerDelegatePtr& delegate);
  void Destroy();
};

} // namespace crow