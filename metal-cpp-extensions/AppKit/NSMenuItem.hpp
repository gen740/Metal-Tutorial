#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <Foundation/Foundation.hpp>
#include <Foundation/NSPrivate.hpp>
#include <string>

#include "AppKitPrivate.hpp"

namespace NS {

_NS_OPTIONS(NS::UInteger, KeyEquivalentModifierMask){
    EventModifierFlagCapsLock = 1 << 16,  // Set if Caps Lock key is pressed.
    EventModifierFlagShift = 1 << 17,     // Set if Shift key is pressed.
    EventModifierFlagControl = 1 << 18,   // Set if Control key is pressed.
    EventModifierFlagOption =
        1 << 19,  // Set if Option or Alternate key is pressed.
    EventModifierFlagCommand = 1 << 20,  // Set if Command key is pressed.
    EventModifierFlagNumericPad =
        1 << 21,  // Set if any key in the numeric keypad is pressed.
    EventModifierFlagHelp = 1 << 22,      // Set if the Help key is pressed.
    EventModifierFlagFunction = 1 << 23,  // Set if any function key is pressed.

    // Used to retrieve only the device-independent modifier flags, allowing
    // applications to mask off the device-dependent modifier flags, including
    // event coalescing information.
    EventModifierFlagDeviceIndependentFlagsMask = 0xffff0000UL};

using MenuItemCallback = void (*)(void* unused, SEL name,
                                  const NS::Object* pSender);

class MenuItem : public NS::Referencing<MenuItem> {
 public:
  static SEL registerActionCallback(const char* name,
                                    MenuItemCallback callback) {
    auto siz = strlen(name);
    SEL sel;
    if ((siz > 0) && (name[siz - 1] != ':')) {
      char* colName = reinterpret_cast<char*>(alloca(siz + 2));
      memcpy(colName, name, siz);
      colName[siz] = ':';
      colName[siz + 1] = '\0';
      sel = sel_registerName(colName);
    } else {
      sel = sel_registerName(name);
    }
    if (callback != nullptr) {
      class_addMethod(static_cast<Class>(_NS_PRIVATE_CLS(NSObject)), sel,
                      reinterpret_cast<IMP>(callback), "v@:@");
    }
    return sel;
  }
  static MenuItem* alloc() {
    return Object::alloc<NS::MenuItem>(_NS_PRIVATE_CLS(NSMenuItem));
  }
  MenuItem* init() {
    return Object::sendMessage<NS::MenuItem*>(this, _NS_PRIVATE_SEL(init));
  }
  void setKeyEquivalentModifierMask(
      NS::KeyEquivalentModifierMask modifierMask) {
    return Object::sendMessage<void>(
        this, _NS_PRIVATE_SEL(setKeyEquivalentModifierMask_), modifierMask);
  }
  [[nodiscard]] NS::KeyEquivalentModifierMask KeyEquivalentModifierMask()
      const {
    return Object::sendMessage<NS::KeyEquivalentModifierMask>(
        this, _NS_PRIVATE_SEL(keyEquivalentModifierMask));
  }
  void setSubmenu(const class Menu* pSubmenu) {
    Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setSubmenu_), pSubmenu);
  }
};

}  // namespace NS
