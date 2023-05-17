#include <Foundation/NSObject.hpp>

#include "AppKitPrivate.hpp"

namespace NS {
class Event : public Referencing<Event> {
 public:
  static Event* alloc() {
    return Object::sendMessage<Event*>(_APPKIT_PRIVATE_CLS(NSEvent), _NS_PRIVATE_SEL(alloc));
  }
  // Event* init(CGRect contentRect, WindowStyleMask styleMask,
  //              BackingStoreType backing, bool defer);
  //
  // void setContentView(const View* pContentView);
  // void makeKeyAndOrderFront(const Object* pSender);
  // void setTitle(const String* pTitle);
  //
  // void close();
};

}  // namespace NS
