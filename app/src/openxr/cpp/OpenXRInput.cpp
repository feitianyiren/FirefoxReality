#include "OpenXRInput.h"
#include "OpenXRHelpers.h"
#include <vector>

namespace crow {

// Threshold to consider a trigger value as a click
// Used when devices don't map the click value for triggers;
const float kPressThreshold = 0.99f;

void
OpenXRInput::Initialize(XrInstance instance, XrSession session) {
  CHECK(instance != XR_NULL_HANDLE);
  CHECK(session != XR_NULL_HANDLE);

  // Create the main action set.
  {
    XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    strcpy(actionSetInfo.actionSetName, "browser");
    strcpy(actionSetInfo.localizedActionSetName, "Browser");
    actionSetInfo.priority = 0;
    CHECK_XRCMD(xrCreateActionSet(instance, &actionSetInfo, &actionSet));
  }

  // Create subactions for left and right hands.
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left", &handSubactionPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right", &handSubactionPath[Hand::Right]));

  // Create actions. We try to mimic https://www.w3.org/TR/webxr-gamepads-module-1/#xr-standard-gamepad-mapping
  {
    // Create an input action for getting the left and right hand poses.
    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
    actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    strcpy(actionInfo.actionName, "hand_pose");
    strcpy(actionInfo.localizedActionName, "Hand Pose");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionPose));

    // Create input actions for menu click detection, usually used for back action.
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "menu");
    strcpy(actionInfo.localizedActionName, "Menu");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionMenuClick));

    // Create an input action for trigger click, touch and value detection
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "trigger_click");
    strcpy(actionInfo.localizedActionName, "Trigger click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTriggerClick));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "trigger_touch");
    strcpy(actionInfo.localizedActionName, "Trigger touch");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTriggerTouch));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "trigger_value");
    strcpy(actionInfo.localizedActionName, "Trigger value");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTriggerValue));

    // Create an input action for squeeze click and value detection
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "squeeze_click");
    strcpy(actionInfo.localizedActionName, "Squeeze click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionSqueezeClick));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "squeeze_value");
    strcpy(actionInfo.localizedActionName, "Squeeze value");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionSqueezeValue));

    // Create an input action for trackpad click, touch and values detection
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "trackpad_click");
    strcpy(actionInfo.localizedActionName, "Trackpad click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTrackpadClick));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "trackpad_touch");
    strcpy(actionInfo.localizedActionName, "Trackpad touch");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTrackpadTouch));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "trackpad_value_x");
    strcpy(actionInfo.localizedActionName, "Trackpad value X");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTrackpadX));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "trackpad_value_y");
    strcpy(actionInfo.localizedActionName, "Trackpad value Y");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionTrackpadY));

    // Create an input action for thumbstick click, touch and values detection
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "thumbstick_click");
    strcpy(actionInfo.localizedActionName, "Thumbstick click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionThumbstickClick));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "thumbstick_touch");
    strcpy(actionInfo.localizedActionName, "Thumbstick touch");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionThumbstickTouch));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "thumbstick_value_x");
    strcpy(actionInfo.localizedActionName, "Thumbstick value X");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionThumbstickX));

    actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    strcpy(actionInfo.actionName, "thumbstick_value_y");
    strcpy(actionInfo.localizedActionName, "Thumbstick value Y");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionThumbstickY));

    // Create an input action for ButtonA and Button B clicks and touch
    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "button_a_click");
    strcpy(actionInfo.localizedActionName, "Button A click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionButtonAClick));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "button_a_touch");
    strcpy(actionInfo.localizedActionName, "Button A touch");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionButtonATouch));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "button_b_click");
    strcpy(actionInfo.localizedActionName, "Button B click");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionButtonBClick));

    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(actionInfo.actionName, "button_b_touch");
    strcpy(actionInfo.localizedActionName, "Button B touch");
    actionInfo.countSubactionPaths = uint32_t(handSubactionPath.size());
    actionInfo.subactionPaths = handSubactionPath.data();
    CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &actionButtonBTouch));

  }

  // See https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles
  std::array<XrPath, Hand::Count> selectClickPath;
  std::array<XrPath, Hand::Count> triggerValuePath;
  std::array<XrPath, Hand::Count> triggerTouchPath;
  std::array<XrPath, Hand::Count> triggerClickPath;
  std::array<XrPath, Hand::Count> squeezeValuePath;
  std::array<XrPath, Hand::Count> squeezeClickPath;
  std::array<XrPath, Hand::Count> posePath;
  std::array<XrPath, Hand::Count> hapticPath;
  std::array<XrPath, Hand::Count> menuClickPath;
  std::array<XrPath, Hand::Count> backClickPath;
  std::array<XrPath, Hand::Count> trackpadClickPath;
  std::array<XrPath, Hand::Count> trackpadTouchPath;
  std::array<XrPath, Hand::Count> trackpadXPath;
  std::array<XrPath, Hand::Count> trackpadYPath;
  std::array<XrPath, Hand::Count> thumbstickClickPath;
  std::array<XrPath, Hand::Count> thumbstickTouchPath;
  std::array<XrPath, Hand::Count> thumbstickXPath;
  std::array<XrPath, Hand::Count> thumbstickYPath;
  std::array<XrPath, Hand::Count> buttonAClickPath;
  std::array<XrPath, Hand::Count> buttonATouchPath;
  std::array<XrPath, Hand::Count> buttonBClickPath;
  std::array<XrPath, Hand::Count> buttonBTouchPath;
  std::array<XrPath, Hand::Count> buttonXClickPath;
  std::array<XrPath, Hand::Count> buttonXTouchPath;
  std::array<XrPath, Hand::Count> buttonYClickPath;
  std::array<XrPath, Hand::Count> buttonYTouchPath;
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/select/click", &selectClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/select/click", &selectClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trigger/value", &triggerValuePath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trigger/value", &triggerValuePath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trigger/touch", &triggerTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trigger/touch", &triggerTouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trigger/click", &triggerClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trigger/click", &triggerClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/aim/pose", &posePath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/aim/pose", &posePath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/output/haptic", &hapticPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/output/haptic", &hapticPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/menu/click", &menuClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/menu/click", &menuClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/back/click", &backClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/back/click", &backClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trackpad/click", &trackpadClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trackpad/click", &trackpadClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trackpad/touch", &trackpadTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trackpad/touch", &trackpadTouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trackpad/x", &trackpadXPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trackpad/x", &trackpadXPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/trackpad/y", &trackpadYPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/trackpad/y", &trackpadYPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/thumbstick/click", &thumbstickClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/thumbstick/click", &thumbstickClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/thumbstick/touch", &thumbstickTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/thumbstick/touch", &thumbstickTouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/thumbstick/x", &thumbstickXPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/thumbstick/x", &thumbstickXPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/thumbstick/y", &thumbstickYPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/thumbstick/y", &thumbstickYPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/a/click", &buttonAClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/a/click", &buttonAClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/a/touch", &buttonATouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/a/touch", &buttonATouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/b/click", &buttonBClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/b/click", &buttonBClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/b/touch", &buttonBTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/b/touch", &buttonBTouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/x/click", &buttonXClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/x/click", &buttonXClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/x/touch", &buttonXTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/x/touch", &buttonXTouchPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/y/click", &buttonYClickPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/y/click", &buttonYClickPath[Hand::Right]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/left/input/y/touch", &buttonYTouchPath[Hand::Left]));
  CHECK_XRCMD(xrStringToPath(instance, "/user/hand/right/input/y/touch", &buttonYTouchPath[Hand::Right]));


  // Suggest bindings for KHR Simple. Default fallback when we have not implemented a specific controller binding.
  {
    XrPath khrSimpleInteractionProfilePath;
    CHECK_XRCMD(
        xrStringToPath(instance, "/interaction_profiles/khr/simple_controller", &khrSimpleInteractionProfilePath));
    std::vector<XrActionSuggestedBinding> bindings{{// Generic controller mappings
                                                     {actionPose, posePath[Hand::Left]},
                                                     {actionPose, posePath[Hand::Right]},
                                                     {actionMenuClick, menuClickPath[Hand::Left]},
                                                     {actionMenuClick, menuClickPath[Hand::Right]},
                                                     {actionTriggerClick, selectClickPath[Hand::Left]},
                                                     {actionTriggerClick, selectClickPath[Hand::Right]}}};
    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
    suggestedBindings.suggestedBindings = bindings.data();
    suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
    CHECK_XRCMD(xrSuggestInteractionProfileBindings(instance, &suggestedBindings));
  }

  // Suggest bindings for Oculus Go controller
  {
    XrPath khrSimpleInteractionProfilePath;
    CHECK_XRCMD(
        xrStringToPath(instance, "/interaction_profiles/oculus/go_controller", &khrSimpleInteractionProfilePath));
    std::vector<XrActionSuggestedBinding> bindings{{// Controller mappings
                                                       {actionPose, posePath[Hand::Left]},
                                                       {actionPose, posePath[Hand::Right]},
                                                       {actionMenuClick, backClickPath[Hand::Left]},
                                                       {actionMenuClick, backClickPath[Hand::Right]},
                                                       {actionTriggerClick, triggerClickPath[Hand::Left]},
                                                       {actionTriggerClick, triggerClickPath[Hand::Right]},
                                                       {actionTrackpadClick, trackpadClickPath[Hand::Left]},
                                                       {actionTrackpadClick, trackpadClickPath[Hand::Right]},
                                                       {actionTrackpadTouch, trackpadTouchPath[Hand::Left]},
                                                       {actionTrackpadTouch, trackpadTouchPath[Hand::Right]},
                                                       {actionTrackpadX, trackpadXPath[Hand::Left]},
                                                       {actionTrackpadX, trackpadXPath[Hand::Right]},
                                                       {actionTrackpadY, trackpadYPath[Hand::Left]},
                                                       {actionTrackpadY, trackpadYPath[Hand::Right]}}};
    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
    suggestedBindings.suggestedBindings = bindings.data();
    suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
    //CHECK_XRCMD(xrSuggestInteractionProfileBindings(instance, &suggestedBindings));
  }

  // Suggest bindings for Oculus Touch controller.
  {
    XrPath khrSimpleInteractionProfilePath;
    CHECK_XRCMD(
        xrStringToPath(instance, "/interaction_profiles/oculus/touch_controller", &khrSimpleInteractionProfilePath));
    std::vector<XrActionSuggestedBinding> bindings{{// Controller mappings
                                                       {actionPose, posePath[Hand::Left]},
                                                       {actionPose, posePath[Hand::Right]},
                                                       // Actions available only on left controller
                                                       {actionMenuClick, menuClickPath[Hand::Left]},
                                                       {actionButtonAClick, buttonXClickPath[Hand::Left]},
                                                       {actionButtonATouch, buttonXTouchPath[Hand::Left]},
                                                       {actionButtonBClick, buttonYClickPath[Hand::Left]},
                                                       {actionButtonBTouch, buttonYTouchPath[Hand::Left]},
                                                       // Actions available only on right controller
                                                       {actionButtonAClick, buttonAClickPath[Hand::Right]},
                                                       {actionButtonATouch, buttonATouchPath[Hand::Right]},
                                                       {actionButtonBClick, buttonAClickPath[Hand::Right]},
                                                       {actionButtonBTouch, buttonATouchPath[Hand::Right]},
                                                       // Actions available on both controllers
                                                       {actionTriggerValue, triggerValuePath[Hand::Left]},
                                                       {actionTriggerValue, triggerValuePath[Hand::Right]},
                                                       {actionTriggerTouch, triggerTouchPath[Hand::Left]},
                                                       {actionTriggerTouch, triggerTouchPath[Hand::Right]},
                                                       {actionSqueezeValue, squeezeValuePath[Hand::Left]},
                                                       {actionSqueezeValue, squeezeValuePath[Hand::Right]},
                                                       {actionThumbstickClick, thumbstickClickPath[Hand::Left]},
                                                       {actionThumbstickClick, thumbstickClickPath[Hand::Right]},
                                                       {actionThumbstickTouch, thumbstickTouchPath[Hand::Left]},
                                                       {actionThumbstickTouch, thumbstickTouchPath[Hand::Right]},
                                                       {actionThumbstickX, thumbstickXPath[Hand::Left]},
                                                       {actionThumbstickX, thumbstickXPath[Hand::Right]},
                                                       {actionThumbstickY, thumbstickYPath[Hand::Left]},
                                                       {actionThumbstickY, thumbstickYPath[Hand::Right]}}};
    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
    suggestedBindings.suggestedBindings = bindings.data();
    suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
    CHECK_XRCMD(xrSuggestInteractionProfileBindings(instance, &suggestedBindings));
  }

  // Initialize pose actions
  XrActionSpaceCreateInfo actionSpaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
  actionSpaceInfo.action = actionPose;
  actionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
  actionSpaceInfo.subactionPath = handSubactionPath[Hand::Left];
  CHECK_XRCMD(xrCreateActionSpace(session, &actionSpaceInfo, &controllerState[Hand::Left].space));
  actionSpaceInfo.subactionPath = handSubactionPath[Hand::Right];
  CHECK_XRCMD(xrCreateActionSpace(session, &actionSpaceInfo, &controllerState[Hand::Right].space));

  // Attach actions to session
  XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
  attachInfo.countActionSets = 1;
  attachInfo.actionSets = &actionSet;
  CHECK_XRCMD(xrAttachSessionActionSets(session, &attachInfo));
}

void OpenXRInput::Update(XrSession session, XrTime predictedDisplayTime, XrSpace baseSpace, device::RenderMode renderMode, ControllerDelegatePtr& delegate) {
  CHECK(session != XR_NULL_HANDLE);

  // Sync actions
  const XrActiveActionSet activeActionSet{actionSet, XR_NULL_PATH};
  XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
  syncInfo.countActiveActionSets = 1;
  syncInfo.activeActionSets = &activeActionSet;
  CHECK_XRCMD(xrSyncActions(session, &syncInfo));

  // Query actions and pose state for each hand
  for (auto hand : {Hand::Right}) {
    const int index = hand;
    ControllerState& controller = controllerState[hand];

    // Query pose state
    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
    getInfo.subactionPath = handSubactionPath[hand];
    getInfo.action = actionPose;
    XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
    CHECK_XRCMD(xrGetActionStatePose(session, &getInfo, &poseState));

    if (!poseState.isActive) {
      if (controller.created) {
        delegate->SetEnabled(hand, false);
      }
      // Controller inactive, skip.
      continue;
    }

    if (!controller.created) {
      if (hand == 0) {
        vrb::Matrix beamTransform = vrb::Matrix::Translation(vrb::Vector(-0.011f, -0.007f, 0.0f));
        delegate->CreateController(index, index,"Oculus Touch (Left)", beamTransform);
        delegate->SetLeftHanded(index, true);

      } else {
        vrb::Matrix beamTransform = vrb::Matrix::Translation(vrb::Vector(0.011f, -0.007f, 0.0f));
        delegate->CreateController(hand, hand, "Oculus Touch (Right)", beamTransform);
      }

      delegate->SetControllerType(index, device::OculusQuest); // TODO: remove this
      // Set default counts for xr-standard-gamepad-mapping
      // See: https://www.w3.org/TR/webxr-gamepads-module-1/#xr-standard-gamepad-mapping
      delegate->SetButtonCount(index, 7);
      delegate->SetHapticCount(index, 0);
      controller.created = true;
    }

    // Query controller tracking and map the pose.
    XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
    XrResult res = xrLocateSpace(controller.space, baseSpace, predictedDisplayTime, &spaceLocation);
    CHECK_XRRESULT(res, "Input xrLocateSpace");
    if (XR_UNQUALIFIED_SUCCESS(res)) {
      controller.enabled = true;
      delegate->SetEnabled(index, true);
      delegate->SetVisible(hand, true);
      // set up controller capability caps
      device::CapabilityFlags caps = device::Orientation;
      if (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
        caps |= (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) ? device::Position : device::PositionEmulated;
      }
      delegate->SetCapabilityFlags(index, caps);
      // set up pose
      vrb::Matrix transform = XrPoseToMatrix(spaceLocation.pose);
      if (renderMode == device::RenderMode::StandAlone) {
        transform.TranslateInPlace(vrb::Vector(0.0f, 1.7f, 0.0f));
      }
      delegate->SetTransform(index, transform);
    } else {
      controller.enabled = false;
      delegate->SetEnabled(hand, false);
      delegate->SetVisible(hand, false);
      // Tracking lost or inactive, skip.
      continue;
    }

    // Query buttons and axes
    XrActionStateBoolean menuClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionMenuClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &menuClick));

    XrActionStateBoolean triggerClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionTriggerClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &triggerClick));

    XrActionStateBoolean triggerTouch{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionTriggerTouch;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &triggerTouch));

    XrActionStateFloat triggerValue{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionTriggerValue;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &triggerValue));

    XrActionStateBoolean squeezeClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionSqueezeClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &squeezeClick));

    XrActionStateFloat squeezeValue{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionSqueezeValue;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &squeezeValue));

    XrActionStateBoolean trackpadClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionTrackpadClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &trackpadClick));

    XrActionStateBoolean trackpadTouch{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionTrackpadTouch;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &trackpadTouch));

    XrActionStateFloat trackpadX{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionTrackpadX;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &trackpadX));

    XrActionStateFloat trackpadY{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionTrackpadY;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &trackpadY));

    XrActionStateBoolean thumbStickClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionThumbstickClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &thumbStickClick));

    XrActionStateBoolean thumbstickTouch{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionThumbstickTouch;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &thumbstickTouch));

    XrActionStateFloat thumbstickX{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionThumbstickX;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &thumbstickX));

    XrActionStateFloat thumbstickY{XR_TYPE_ACTION_STATE_FLOAT};
    getInfo.action = actionThumbstickY;
    CHECK_XRCMD(xrGetActionStateFloat(session, &getInfo, &thumbstickY));

    XrActionStateBoolean buttonAClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionButtonAClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &buttonAClick));

    XrActionStateBoolean buttonATouch{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionButtonATouch;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &buttonATouch));

    XrActionStateBoolean buttonBClick{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionButtonBClick;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &buttonBClick));

    XrActionStateBoolean buttonBTouch{XR_TYPE_ACTION_STATE_BOOLEAN};
    getInfo.action = actionButtonBTouch;
    CHECK_XRCMD(xrGetActionStateBoolean(session, &getInfo, &buttonBTouch));


    // Map to controller delegate
    std::array<float, 4> axes;

    if (menuClick.isActive) {
      const bool pressed = menuClick.currentState != 0;
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_APP, -1, pressed, pressed);
    }

    if (triggerValue.isActive || triggerClick.isActive || triggerTouch.isActive) {
      bool pressed = triggerClick.isActive && triggerClick.currentState != 0;
      if (!triggerClick.isActive) {
        pressed |= triggerValue.isActive && triggerValue.currentState > kPressThreshold;
      }
      bool touched = pressed;
      touched |= triggerValue.isActive && triggerValue.currentState > 0.0f;
      touched |= triggerTouch.isActive && triggerTouch.currentState != 0;
      float value = pressed ? 1.0f : 0.0f;
      if (triggerValue.isActive) {
        value = triggerValue.currentState;
      }
      VRB_ERROR("makelele trigger pressed: %d", pressed);
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_TRIGGER, device::kImmersiveButtonTrigger, pressed, touched, value);
      if (pressed && renderMode == device::RenderMode::Immersive) {
        delegate->SetSelectActionStart(index);
      } else {
        delegate->SetSelectActionStop(index);
      }
    }

    if (squeezeClick.isActive || squeezeValue.isActive) {
      bool pressed = squeezeClick.isActive && squeezeClick.currentState != 0;
      if (!squeezeClick.isActive) {
        pressed |= squeezeValue.isActive && squeezeValue.currentState > kPressThreshold;
      }
      float value = pressed ? 1.0f : 0.0f;
      if (squeezeValue.isActive) {
        value = squeezeValue.currentState;
      }
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_SQUEEZE, device::kImmersiveButtonSqueeze, pressed, pressed, value);
      if (pressed && renderMode == device::RenderMode::Immersive) {
        delegate->SetSqueezeActionStart(index);
      } else {
        delegate->SetSqueezeActionStop(index);
      }
    }

    if (trackpadClick.isActive || trackpadTouch.isActive || trackpadX.isActive || trackpadY.isActive) {
      bool pressed = trackpadClick.isActive && trackpadClick.currentState != 0;
      bool touched = pressed || (trackpadTouch.isActive && trackpadTouch.currentState != 0);
      const float x = trackpadX.isActive ? trackpadX.currentState : 0.0f;
      const float y = trackpadY.isActive ? trackpadY.currentState : 0.0f;
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_TOUCHPAD, device::kImmersiveButtonTouchpad, pressed, touched);
      axes[device::kImmersiveAxisTouchpadX] = x;
      axes[device::kImmersiveAxisTouchpadY] = y;
      delegate->SetScrolledDelta(index, x, y);
    }

    if (thumbStickClick.isActive || thumbstickTouch.isActive || thumbstickX.isActive || thumbstickY.isActive) {
      bool pressed = thumbStickClick.isActive && thumbStickClick.currentState != 0;
      bool touched = pressed || (thumbstickTouch.isActive && thumbstickTouch.currentState != 0);
      const float x = thumbstickX.isActive ? thumbstickX.currentState : 0.0f;
      const float y = thumbstickY.isActive ? thumbstickY.currentState : 0.0f;
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_TOUCHPAD, device::kImmersiveButtonThumbstick, pressed, touched);
      axes[device::kImmersiveAxisThumbstickX] = x;
      axes[device::kImmersiveAxisThumbstickY] = y;
      delegate->SetScrolledDelta(index, x, y);
    }

    if (buttonAClick.isActive) {
      const bool pressed = buttonAClick.currentState != 0;
      const bool touched = pressed || (buttonATouch.isActive && buttonATouch.currentState != 0);
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_A, device::kImmersiveButtonA, pressed, touched);
    }

    if (buttonBClick.isActive) {
      const bool pressed = buttonBClick.currentState != 0;
      const bool touched = pressed || (buttonBTouch.isActive && buttonBTouch.currentState != 0);
      delegate->SetButtonState(index, ControllerDelegate::BUTTON_B, device::kImmersiveButtonB, pressed, touched);
    }

    delegate->SetAxes(index, axes.data(), axes.size());
  }
}


void OpenXRInput::Destroy() {
  if (actionSet != XR_NULL_HANDLE) {
    CHECK_XRCMD(xrDestroyActionSet(actionSet));
    actionSet = XR_NULL_HANDLE;
    // No need to destroy input actions, they are destroyed automatically when destroying the parent action set
  }
}

} // namespace crow