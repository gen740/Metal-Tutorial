#pragma once

#include <Foundation/Foundation.hpp>
#include <Foundation/NSPrivate.hpp>

#include "AppKitPrivate.hpp"
#include "NSMenuItem.hpp"

namespace NS {

class Menu : public Referencing<Menu> {
 public:
  static Menu *alloc() { return Object::alloc<Menu>(_NS_PRIVATE_CLS(NSMenu)); }
  Menu *init() {
    return Object::sendMessage<Menu *>(this, _NS_PRIVATE_SEL(init));
  }
  Menu *init(const String *pTitle) {
    return Object::sendMessage<Menu *>(this, _NS_PRIVATE_SEL(initWithTitle_),
                                       pTitle);
  }

  MenuItem *addItem(const String *pTitle, SEL pSelector,
                    const String *pKeyEquivalent) {
    return Object::sendMessage<MenuItem *>(
        this, _NS_PRIVATE_SEL(addItemWithTitle_action_keyEquivalent_), pTitle,
        pSelector, pKeyEquivalent);
  }
  void addItem(const MenuItem *pItem) {
    Object::sendMessage<void>(this, _NS_PRIVATE_SEL(addItem_), pItem);
  }
};

}  // namespace NS
