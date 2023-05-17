/*
 *
 * Copyright 2022 Apple Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cassert>
#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <AppKit/AppKit.hpp>
// #include <AppKit/NSEvent.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class Renderer {
public:
  explicit Renderer(MTL::Device *pDevice) : _pDevice(pDevice->retain()) {
    _pCommandQueue = _pDevice->newCommandQueue();
  }
  ~Renderer() {
    _pCommandQueue->release();
    _pDevice->release();
  }

  void draw(MTK::View *pView) {
    NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer *pCmd = _pCommandQueue->commandBuffer();
    MTL::RenderPassDescriptor *pRpd = pView->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder *pEnc = pCmd->renderCommandEncoder(pRpd);
    pEnc->endEncoding();
    pCmd->presentDrawable(pView->currentDrawable());
    pCmd->commit();

    pPool->release();
  }

private:
  MTL::Device *_pDevice;
  MTL::CommandQueue *_pCommandQueue;
};

class MyMTKViewDelegate : public MTK::ViewDelegate {
public:
  explicit MyMTKViewDelegate(MTL::Device *pDevice)
      : MTK::ViewDelegate(), _pRenderer(new Renderer(pDevice)) {}
  ~MyMTKViewDelegate() override { delete _pRenderer; }
  void drawInMTKView(MTK::View *pView) override { _pRenderer->draw(pView); }

private:
  Renderer *_pRenderer;
};

class MyAppDelegate : public NS::ApplicationDelegate {
public:
  ~MyAppDelegate() override {
    _pMtkView->release();
    _pWindow->release();
    _pDevice->release();
    delete _pViewDelegate;
  }

  static NS::Menu *createMenuBar() {
    using NS::StringEncoding::UTF8StringEncoding;

    NS::Menu *pMainMenu = NS::Menu::alloc()->init();
    NS::MenuItem *pAppMenuItem = NS::MenuItem::alloc()->init();
    NS::Menu *pAppMenu = NS::Menu::alloc()->init(
        NS::String::string("Appname", UTF8StringEncoding));

    NS::String *appName =
        NS::RunningApplication::currentApplication()->localizedName();
    NS::String *quitItemName = NS::String::string("Quit ", UTF8StringEncoding)
                                   ->stringByAppendingString(appName);
    SEL quitCb = NS::MenuItem::registerActionCallback(
        "appQuit", [](void *, SEL, const NS::Object *pSender) {
          auto *pApp = NS::Application::sharedApplication();
          pApp->terminate(pSender);
        });

    NS::MenuItem *pAppQuitItem = pAppMenu->addItem(
        quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
    pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
    pAppMenuItem->setSubmenu(pAppMenu);

    NS::MenuItem *pWindowMenuItem = NS::MenuItem::alloc()->init();
    NS::Menu *pWindowMenu = NS::Menu::alloc()->init(
        NS::String::string("Window", UTF8StringEncoding));

    SEL closeWindowCb = NS::MenuItem::registerActionCallback(
        "windowClose", [](void *, SEL, const NS::Object *) {
          auto *pApp = NS::Application::sharedApplication();
          pApp->windows()->object<NS::Window>(0)->close();
        });
    NS::MenuItem *pCloseWindowItem = pWindowMenu->addItem(
        NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb,
        NS::String::string("w", UTF8StringEncoding));
    pCloseWindowItem->setKeyEquivalentModifierMask(
        NS::EventModifierFlagCommand);

    pWindowMenuItem->setSubmenu(pWindowMenu);

    pMainMenu->addItem(pAppMenuItem);
    pMainMenu->addItem(pWindowMenuItem);

    pAppMenuItem->release();
    pWindowMenuItem->release();
    pAppMenu->release();
    pWindowMenu->release();

    return pMainMenu->autorelease();
  }

  void
  applicationWillFinishLaunching(NS::Notification *pNotification) override {
    std::cout << "applicationWillFinishLaunching Start" << std::endl;
    NS::Menu *pMenu = createMenuBar();
    auto *pApp = reinterpret_cast<NS::Application *>(pNotification->object());
    pApp->setMainMenu(pMenu);
    pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
    std::cout << "applicationWillFinishLaunching End" << std::endl;
  }

  void applicationDidFinishLaunching(NS::Notification *pNotification) override {
    std::cout << "applicationDidFinishLaunching Start" << std::endl;
    CGRect frame = (CGRect){{100.0, 100.0}, {512.0, 512.0}};

    _pWindow = NS::Window::alloc()->init(
        frame, NS::WindowStyleMaskBorderless, 
        NS::BackingStoreBuffered, false);

    _pDevice = MTL::CreateSystemDefaultDevice();

    _pMtkView = MTK::View::alloc()->init(frame, _pDevice);
    _pMtkView->setColorPixelFormat(
        MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    _pMtkView->setClearColor(MTL::ClearColor::Make(1.0, 1.0, 0.0, 1.0));

    _pViewDelegate = new MyMTKViewDelegate(_pDevice);
    _pMtkView->setDelegate(_pViewDelegate);

    _pWindow->setContentView(_pMtkView);
    _pWindow->setTitle(NS::String::string(
        "00 - Window", NS::StringEncoding::UTF8StringEncoding));

    _pWindow->makeKeyAndOrderFront(nullptr);

    auto *pApp = reinterpret_cast<NS::Application *>(pNotification->object());
    pApp->activateIgnoringOtherApps(true);

    std::cout << "applicationDidFinishLaunching End" << std::endl;
  }

  bool applicationShouldTerminateAfterLastWindowClosed(
      [[maybe_unused]] NS::Application *pSender) override {
    return true;
  }

private:
  NS::Window *_pWindow{};
  
  MTK::View *_pMtkView{};
  MTL::Device *_pDevice{};
  MyMTKViewDelegate *_pViewDelegate = nullptr;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  NS::AutoreleasePool *pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  MyAppDelegate del;

  NS::Application *pSharedApplication = NS::Application::sharedApplication();

  pSharedApplication->setDelegate(&del);
  pSharedApplication->run();

  pAutoreleasePool->release();

  return 0;
}
