#pragma once

#include <CoreGraphics/CGGeometry.h>

#include <Foundation/NSObject.hpp>

#include "AppKit/NSApplication.hpp"
#include "AppKitPrivate.hpp"
#include "NSView.hpp"

namespace NS {
class Window : public Referencing<Window> {
 public:
  static Window* alloc() {
    return Object::sendMessage<Window*>(_APPKIT_PRIVATE_CLS(NSWindow),
                                        _NS_PRIVATE_SEL(alloc));
  }
  Window* init(CGRect contentRect, WindowStyleMask styleMask,
               BackingStoreType backing, bool defer) {
    return Object::sendMessage<Window*>(
        this, _APPKIT_PRIVATE_SEL(initWithContentRect_styleMask_backing_defer_),
        contentRect, styleMask, backing, defer);
  }

  void setContentView(const View* pContentView) {
    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(setContentView_),
                              pContentView);
  }

  void makeKeyAndOrderFront(const Object* pSender) {
    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(makeKeyAndOrderFront_),
                              pSender);
  }

  void setTitle(const String* pTitle) {
    Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(setTitle_), pTitle);
  }

  void close() { Object::sendMessage<void>(this, _APPKIT_PRIVATE_SEL(close)); }
};

}  // namespace NS
