#pragma once

#include <Foundation/Foundation.hpp>

#include "AppKitPrivate.hpp"

namespace NS {
_NS_OPTIONS(NS::UInteger, WindowStyleMask){
    WindowStyleMaskBorderless = 0,
    WindowStyleMaskTitled = (1 << 0),
    WindowStyleMaskClosable = (1 << 1),
    WindowStyleMaskMiniaturizable = (1 << 2),
    WindowStyleMaskResizable = (1 << 3),
    WindowStyleMaskTexturedBackground = (1 << 8),
    WindowStyleMaskUnifiedTitleAndToolbar = (1 << 12),
    WindowStyleMaskFullScreen = (1 << 14),
    WindowStyleMaskFullSizeContentView = (1 << 15),
    WindowStyleMaskUtilityWindow = (1 << 4),
    WindowStyleMaskDocModalWindow = (1 << 6),
    WindowStyleMaskNonactivatingPanel = (1 << 7),
    WindowStyleMaskHUDWindow = (1 << 13)};

_NS_ENUM(NS::UInteger, BackingStoreType){BackingStoreRetained = 0,
                                         BackingStoreNonretained = 1,
                                         BackingStoreBuffered = 2};

_NS_ENUM(NS::UInteger, ActivationPolicy){ActivationPolicyRegular,
                                         ActivationPolicyAccessory,
                                         ActivationPolicyProhibited};

class ApplicationDelegate {
 public:
  virtual ~ApplicationDelegate() = default;
  virtual void applicationWillFinishLaunching(
      [[maybe_unused]] Notification* pNotification) {}
  virtual void applicationDidFinishLaunching(
      [[maybe_unused]] Notification* pNotification) {}
  virtual bool applicationShouldTerminateAfterLastWindowClosed(
      [[maybe_unused]] class Application* pSender) {
    return false;
  }
};

class Application : public NS::Referencing<Application> {
 public:
  static Application* sharedApplication() {
    return Object::sendMessage<Application*>(
        _APPKIT_PRIVATE_CLS(NSApplication),
        _APPKIT_PRIVATE_SEL(sharedApplication));
  }

  void setDelegate(const ApplicationDelegate* pDelegate) {
    NS::Value* pWrapper = NS::Value::value(pDelegate);

    typedef void (*DispatchFunction)(NS::Value*, SEL, void*);

    DispatchFunction willFinishLaunching = [](Value* pSelf, SEL,
                                              void* pNotification) {
      auto* pDel =
          reinterpret_cast<NS::ApplicationDelegate*>(pSelf->pointerValue());
      pDel->applicationWillFinishLaunching(
          static_cast<NS::Notification*>(pNotification));
    };

    DispatchFunction didFinishLaunching = [](Value* pSelf, SEL,
                                             void* pNotification) {
      auto* pDel =
          reinterpret_cast<NS::ApplicationDelegate*>(pSelf->pointerValue());
      pDel->applicationDidFinishLaunching(
          static_cast<NS::Notification*>(pNotification));
    };

    DispatchFunction shouldTerminateAfterLastWindowClosed =
        [](Value* pSelf, SEL, void* pApplication) {
          auto* pDel =
              reinterpret_cast<NS::ApplicationDelegate*>(pSelf->pointerValue());
          pDel->applicationShouldTerminateAfterLastWindowClosed(
              static_cast<NS::Application*>(pApplication));
        };

    class_addMethod(static_cast<Class>(_NS_PRIVATE_CLS(NSValue)),
                    _APPKIT_PRIVATE_SEL(applicationWillFinishLaunching_),
                    reinterpret_cast<IMP>(willFinishLaunching), "v@:@");
    class_addMethod(static_cast<Class>(_NS_PRIVATE_CLS(NSValue)),
                    _APPKIT_PRIVATE_SEL(applicationDidFinishLaunching_),
                    reinterpret_cast<IMP>(didFinishLaunching), "v@:@");
    class_addMethod(
        static_cast<Class>(_NS_PRIVATE_CLS(NSValue)),
        _APPKIT_PRIVATE_SEL(applicationShouldTerminateAfterLastWindowClosed_),
        reinterpret_cast<IMP>(shouldTerminateAfterLastWindowClosed), "B@:@");

    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(setDelegate_),
                              pWrapper);
  }

  bool setActivationPolicy(ActivationPolicy activationPolicy) {
    return NS::Object::sendMessage<bool>(
        this, _APPKIT_PRIVATE_SEL(setActivationPolicy_), activationPolicy);
  }

  void activateIgnoringOtherApps(bool ignoreOtherApps) {
    Object::sendMessage<void>(this,
                              _APPKIT_PRIVATE_SEL(activateIgnoringOtherApps_),
                              (ignoreOtherApps ? YES : NO));
  }

  void setMainMenu(const class Menu* pMenu) {
    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(setMainMenu_), pMenu);
  }

  [[nodiscard]] NS::Array* windows() const {
    return Object::sendMessage<NS::Array*>(this, _APPKIT_PRIVATE_SEL(windows));
  }

  void run() { Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(run)); }

  void terminate(const Object* pSender) {
    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(terminate_), pSender);
  }
};

}  // namespace NS
