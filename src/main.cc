#include <array>
#include <cassert>
#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <simd/simd.h>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class Renderer {
 public:
  explicit Renderer(MTL::Device* pDevice);
  ~Renderer();
  void buildShaders();
  void buildBuffers();
  void buildFrameData();
  void draw(MTK::View* pView);

 private:
  MTL::Device* _pDevice;
  MTL::CommandQueue* _pCommandQueue;
  MTL::Library* _pShaderLibrary{};
  MTL::RenderPipelineState* _pPSO{};
  MTL::Buffer* _pArgBuffer{};
  MTL::Buffer* _pVertexPositionsBuffer{};
  MTL::Buffer* _pVertexColorsBuffer{};

  static constexpr int kMaxFramesInFlight = 1;

  std::array<MTL::Buffer*, kMaxFramesInFlight> _pFrameData{};
  float _angle{};
  int _frame{};
  dispatch_semaphore_t _semaphore;
};

class MyMTKViewDelegate : public MTK::ViewDelegate {
 public:
  explicit MyMTKViewDelegate(MTL::Device* pDevice);
  ~MyMTKViewDelegate() override;
  void drawInMTKView(MTK::View* pView) override;

 private:
  Renderer* _pRenderer;
};

class MyAppDelegate : public NS::ApplicationDelegate {
 public:
  ~MyAppDelegate() override;

  static NS::Menu* createMenuBar();

  void applicationWillFinishLaunching(NS::Notification* pNotification) override;
  void applicationDidFinishLaunching(NS::Notification* pNotification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* pSender) override;

 private:
  NS::Window* _pWindow{};
  MTK::View* _pMtkView{};
  MTL::Device* _pDevice{};
  MyMTKViewDelegate* _pViewDelegate = nullptr;
};

int main() {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  MyAppDelegate del;

  NS::Application* pSharedApplication = NS::Application::sharedApplication();
  pSharedApplication->setDelegate(&del);
  pSharedApplication->run();

  pAutoreleasePool->release();

  return 0;
}

MyAppDelegate::~MyAppDelegate() {
  _pMtkView->release();
  _pWindow->release();
  _pDevice->release();
  delete _pViewDelegate;
}

NS::Menu* MyAppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu* pMainMenu = NS::Menu::alloc()->init();
  NS::MenuItem* pAppMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pAppMenu = NS::Menu::alloc()->init(
      NS::String::string("Appname", UTF8StringEncoding));

  NS::String* appName =
      NS::RunningApplication::currentApplication()->localizedName();
  NS::String* quitItemName = NS::String::string("Quit ", UTF8StringEncoding)
                                 ->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback(
      "appQuit", [](void*, SEL, const NS::Object* pSender) {
        auto* pApp = NS::Application::sharedApplication();
        pApp->terminate(pSender);
      });

  NS::MenuItem* pAppQuitItem = pAppMenu->addItem(
      quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
  pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pAppMenuItem->setSubmenu(pAppMenu);

  NS::MenuItem* pWindowMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pWindowMenu =
      NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

  SEL closeWindowCb = NS::MenuItem::registerActionCallback(
      "windowClose", [](void*, SEL, const NS::Object*) {
        auto* pApp = NS::Application::sharedApplication();
        pApp->windows()->object<NS::Window>(0)->close();
      });
  NS::MenuItem* pCloseWindowItem = pWindowMenu->addItem(
      NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb,
      NS::String::string("w", UTF8StringEncoding));
  pCloseWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

  pWindowMenuItem->setSubmenu(pWindowMenu);

  pMainMenu->addItem(pAppMenuItem);
  pMainMenu->addItem(pWindowMenuItem);

  pAppMenuItem->release();
  pWindowMenuItem->release();
  pAppMenu->release();
  pWindowMenu->release();

  return pMainMenu->autorelease();
}

void MyAppDelegate::applicationWillFinishLaunching(
    NS::Notification* pNotification) {
  NS::Menu* pMenu = createMenuBar();
  auto* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void MyAppDelegate::applicationDidFinishLaunching(
    NS::Notification* pNotification) {
  CGRect frame = (CGRect){{100.0, 100.0}, {512.0, 512.0}};

  _pWindow = NS::Window::alloc()->init(
      frame, NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
      NS::BackingStoreBuffered, false);

  _pDevice = MTL::CreateSystemDefaultDevice();

  _pMtkView = MTK::View::alloc()->init(frame, _pDevice);
  _pMtkView->setPreferredFramesPerSecond(600);
  _pMtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  _pMtkView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));

  _pViewDelegate = new MyMTKViewDelegate(_pDevice);
  _pMtkView->setDelegate(_pViewDelegate);

  _pWindow->setContentView(_pMtkView);
  _pWindow->setTitle(NS::String::string(
      "03 - Animation", NS::StringEncoding::UTF8StringEncoding));

  _pWindow->makeKeyAndOrderFront(nullptr);

  auto* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->activateIgnoringOtherApps(true);
}

bool MyAppDelegate::applicationShouldTerminateAfterLastWindowClosed(
    [[maybe_unused]] NS::Application* pSender) {
  return true;
}

MyMTKViewDelegate::MyMTKViewDelegate(MTL::Device* pDevice)
    : MTK::ViewDelegate(), _pRenderer(new Renderer(pDevice)) {}

MyMTKViewDelegate::~MyMTKViewDelegate() { delete _pRenderer; }

void MyMTKViewDelegate::drawInMTKView(MTK::View* pView) {
  _pRenderer->draw(pView);
}

Renderer::Renderer(MTL::Device* pDevice) : _pDevice(pDevice->retain()) {
  _pCommandQueue = _pDevice->newCommandQueue();
  buildShaders();
  buildBuffers();
  buildFrameData();

  _semaphore = dispatch_semaphore_create(Renderer::kMaxFramesInFlight);
}

Renderer::~Renderer() {
  _pShaderLibrary->release();
  _pArgBuffer->release();
  _pVertexPositionsBuffer->release();
  _pVertexColorsBuffer->release();
  for (int i = 0; i < Renderer::kMaxFramesInFlight; ++i) {
    _pFrameData[i]->release();
  }
  _pPSO->release();
  _pCommandQueue->release();
  _pDevice->release();
}

void Renderer::buildShaders() {
  using NS::StringEncoding::UTF8StringEncoding;

  const char* shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct v2f
        {
            float4 position [[position]];
            half3 color;
        };

        struct VertexData
        {
            device float3* positions [[id(0)]];
            device float3* colors [[id(1)]];
        };

        struct FrameData
        {
            float angle;
        };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]], constant FrameData* frameData [[buffer(1)]], uint vertexId [[vertex_id]] )
        {
            float a = frameData->angle;
            float3x3 rotationMatrix = float3x3( sin(a), cos(a), 0.0, cos(a), -sin(a), 0.0, 0.0, 0.0, 1.0 );
            v2f o;
            o.position = float4( rotationMatrix * vertexData->positions[ vertexId ], 1.0 );
            o.color = half3(vertexData->colors[ vertexId ]);
            return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]] )
        {
            return half4( in.color, 1.0 );
        }
    )";

  NS::Error* pError = nullptr;
  MTL::Library* pLibrary = _pDevice->newLibrary(
      NS::String::string(shaderSrc, UTF8StringEncoding), nullptr, &pError);
  if (pLibrary == nullptr) {
    std::cerr << pError->localizedDescription()->utf8String() << std::endl;
    assert(false);
  }

  MTL::Function* pVertexFn = pLibrary->newFunction(
      NS::String::string("vertexMain", UTF8StringEncoding));
  MTL::Function* pFragFn = pLibrary->newFunction(
      NS::String::string("fragmentMain", UTF8StringEncoding));

  MTL::RenderPipelineDescriptor* pDesc =
      MTL::RenderPipelineDescriptor::alloc()->init();
  pDesc->setVertexFunction(pVertexFn);
  pDesc->setFragmentFunction(pFragFn);
  pDesc->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  _pPSO = _pDevice->newRenderPipelineState(pDesc, &pError);
  if (_pPSO == nullptr) {
    std::cerr << pError->localizedDescription()->utf8String() << std::endl;
    assert(false);
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  _pShaderLibrary = pLibrary;
}

void Renderer::buildBuffers() {
  using simd::float3;
  const size_t NumVertices = 3;

  std::array<float3, NumVertices> positions = {float3{-0.8F, 0.8F, 0.0F},
                                               float3{0.0F, -0.8F, 0.0F},
                                               float3{+0.8F, 0.8F, 0.0F}};

  std::array<float3, NumVertices> colors = {float3{1.0, 0.3F, 0.2F},
                                            float3{0.8F, 1.0, 0.0F},
                                            float3{0.8F, 0.0F, 1.0}};

  const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
  const size_t colorDataSize = NumVertices * sizeof(simd::float3);

  MTL::Buffer* pVertexPositionsBuffer =
      _pDevice->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
  MTL::Buffer* pVertexColorsBuffer =
      _pDevice->newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

  _pVertexPositionsBuffer = pVertexPositionsBuffer;
  _pVertexColorsBuffer = pVertexColorsBuffer;

  memcpy(_pVertexPositionsBuffer->contents(), positions.data(),
         positionsDataSize);
  memcpy(_pVertexColorsBuffer->contents(), colors.data(), colorDataSize);

  _pVertexPositionsBuffer->didModifyRange(
      NS::Range::Make(0, _pVertexPositionsBuffer->length()));
  _pVertexColorsBuffer->didModifyRange(
      NS::Range::Make(0, _pVertexColorsBuffer->length()));

  using NS::StringEncoding::UTF8StringEncoding;
  assert(_pShaderLibrary);

  MTL::Function* pVertexFn = _pShaderLibrary->newFunction(
      NS::String::string("vertexMain", UTF8StringEncoding));
  MTL::ArgumentEncoder* pArgEncoder = pVertexFn->newArgumentEncoder(0);

  MTL::Buffer* pArgBuffer = _pDevice->newBuffer(
      pArgEncoder->encodedLength(), MTL::ResourceStorageModeManaged);
  _pArgBuffer = pArgBuffer;

  pArgEncoder->setArgumentBuffer(_pArgBuffer, 0);

  pArgEncoder->setBuffer(_pVertexPositionsBuffer, 0, 0);
  pArgEncoder->setBuffer(_pVertexColorsBuffer, 0, 1);

  _pArgBuffer->didModifyRange(NS::Range::Make(0, _pArgBuffer->length()));

  pVertexFn->release();
  pArgEncoder->release();
}

struct FrameData {
  float angle;
};

void Renderer::buildFrameData() {
  for (int i = 0; i < Renderer::kMaxFramesInFlight; ++i) {
    _pFrameData[i] =
        _pDevice->newBuffer(sizeof(FrameData), MTL::ResourceStorageModeManaged);
  }
}

void Renderer::draw(MTK::View* pView) {
  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  _frame = (_frame + 1) % Renderer::kMaxFramesInFlight;
  MTL::Buffer* pFrameDataBuffer = _pFrameData[_frame];

  MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
  Renderer* pRenderer = this;
  pCmd->addCompletedHandler([=]([[maybe_unused]] MTL::CommandBuffer* pCmd) {
    dispatch_semaphore_signal(pRenderer->_semaphore);
  });

  reinterpret_cast<FrameData*>(pFrameDataBuffer->contents())->angle =
      (_angle += 0.01F);
  pFrameDataBuffer->didModifyRange(NS::Range::Make(0, sizeof(FrameData)));

  MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(_pPSO);
  pEnc->setVertexBuffer(_pArgBuffer, 0, 0);
  pEnc->useResource(_pVertexPositionsBuffer, MTL::ResourceUsageRead);
  pEnc->useResource(_pVertexColorsBuffer, MTL::ResourceUsageRead);

  pEnc->setVertexBuffer(pFrameDataBuffer, 0, 1);
  pEnc->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                       static_cast<NS::UInteger>(0),
                       static_cast<NS::UInteger>(3));

  pEnc->endEncoding();
  pCmd->presentDrawable(pView->currentDrawable());
  pCmd->commit();

  pPool->release();
}
